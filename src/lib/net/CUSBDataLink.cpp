/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012 Bolton Software Ltd.
 * Copyright (C) 2002 Chris Schoeneman
 * 
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file COPYING that should have accompanied this file.
 * 
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CArch.h"
#include "XArch.h"
#include "XSocket.h"
#include "CLock.h"
#include "CUSBDataLink.h"
#include "CUSBAddress.h"

//
// CUSBDataLink
//

CUSBDataLink::CUSBDataLink() :
	m_device(NULL),
	m_transferRead(NULL),
	m_transferWrite(NULL),
	m_mutex(),
	m_flushed(&m_mutex, true),
	m_connected(false),
	m_readable(false),
	m_writable(false),
	m_waitAccept(false),
	m_waitAcceptMutex(),
	m_accepted(&m_waitAcceptMutex, false)
{
	memset(&m_config, 0, sizeof(m_config));
}

CUSBDataLink::~CUSBDataLink()
{
	close();
}

//
// IDataTransfer overrides
//

void
CUSBDataLink::connect(const CBaseAddress& addr)
{
	initConnection(addr);

	m_waitAccept = true;

	std::string buf("USB_CONNECT");
	write(buf.c_str(), buf.size());	

	// wait for connection to be accepted
	CLock lock(&m_waitAcceptMutex);
	
	while (m_accepted == false) {
		m_accepted.wait();
	}

	m_connected = true;
}

//
// ISocket overrides
//

void
CUSBDataLink::bind(const CBaseAddress& addr)
{
	initConnection(addr);
}

void
CUSBDataLink::close()
{
	CLock lock(&m_mutex);

	// clear buffers and enter disconnected state
	if (m_connected) {
		sendEvent(getDisconnectedEvent());
	}
	onDisconnected();

	if (m_transferRead)
	{
		libusb_free_transfer(m_transferRead);
		m_transferRead = NULL;
	}

	if (m_transferWrite){
		libusb_free_transfer(m_transferWrite);
		m_transferWrite = NULL;
	}

	// close the usb device
	if (m_device)
	{
		ARCH->usbCloseDevice(m_device, m_config.ifid);
		m_device = NULL;
	}
}

void*
CUSBDataLink::getEventTarget() const
{
	return const_cast<void*>(reinterpret_cast<const void*>(this));
}

//
// IStream overrides
//

bool
CUSBDataLink::isReady() const
{
	CLock lock(&m_mutex);
	return (m_inputBuffer.getSize() > 0);
}

UInt32
CUSBDataLink::getSize() const
{
	CLock lock(&m_mutex);
	return m_inputBuffer.getSize();
}

UInt32
CUSBDataLink::read(void* buffer, UInt32 n)
{
	CLock lock(&m_mutex);

	// copy data directly from our input buffer
	UInt32 size = m_inputBuffer.getSize();
	if (n > size) {
		n = size;
	}
	if (buffer != NULL && n != 0) {
		memcpy(buffer, m_inputBuffer.peek(n), n);
	}
	m_inputBuffer.pop(n);

	// if no more data and we cannot read or write then send disconnected
	if (n > 0 && m_inputBuffer.getSize() == 0 && !m_readable && !m_writable) {
		sendEvent(getDisconnectedEvent());
		m_connected = false;
	}

	return n;
}

void
CUSBDataLink::write(const void* buffer, UInt32 n)
{
	bool wasEmpty;
	
	CLock lock(&m_mutex);

	// must not have shutdown output
	if (!m_writable) {
		sendEvent(getOutputErrorEvent());
		return;
	}

	// ignore empty writes
	if (n == 0) {
		return;
	}

	// copy data to the output buffer
	wasEmpty = (m_outputBuffer.getSize() == 0);
	m_outputBuffer.write(buffer, n);

	// there's data to write
	m_flushed = false;

	if (wasEmpty)
	{
		// No active write transfers at this moment. Call writeCallback to start new write transfer.
		m_transferWrite->status = LIBUSB_TRANSFER_COMPLETED;
		m_transferWrite->actual_length = 0;
		m_transferWrite->user_data = this;
		writeCallback(m_transferWrite);
	}
}

void
CUSBDataLink::flush()
{
	CLock lock(&m_mutex);
	
	while (m_flushed == false) {
		m_flushed.wait();
	}
}

void
CUSBDataLink::shutdownInput()
{
	CLock lock(&m_mutex);

	// shutdown socket for reading
	try {
		//ARCH->closeSocketForRead(m_socket);
	}
	catch (XArchNetwork&) {
		// ignore
	}

	// shutdown buffer for reading
	if (m_readable) {
		sendEvent(getInputShutdownEvent());
		onInputShutdown();
	}
}

void
CUSBDataLink::shutdownOutput()
{
	CLock lock(&m_mutex);

	// shutdown socket for writing
	try {
//		ARCH->closeSocketForWrite(m_socket);
	}
	catch (XArchNetwork&) {
		// ignore
	}

	// shutdown buffer for writing
	if (m_writable) {
		sendEvent(getOutputShutdownEvent());
		onOutputShutdown();
	}
}

//
// private
//

void
CUSBDataLink::initConnection(const CBaseAddress& addr)
{
	assert(addr.getAddressType() == CBaseAddress::USB);
	const CUSBAddress& usbAddress = reinterpret_cast<const CUSBAddress&>(addr);

	close();

	m_config.idVendor = usbAddress.GetVID();
	m_config.idProduct = usbAddress.GetPID();
	m_config.busNumber = usbAddress.GetIDBus();
	m_config.devAddress = usbAddress.GetIDDeviceOnBus();
	m_config.ifid = 0;
	m_config.bulkin = usbAddress.GetIDBulkIN();
	m_config.bulkout = usbAddress.GetIDBulkOut();
	
	m_device = ARCH->usbOpenDevice(m_config, m_config.ifid);	

	m_transferRead = libusb_alloc_transfer(0);
	m_transferWrite = libusb_alloc_transfer(0);

	m_readable = true;
	m_writable = true;

	// Start pooling input endpoint.
	m_transferRead->status = LIBUSB_TRANSFER_COMPLETED;
	m_transferRead->actual_length = 0;
	m_transferRead->user_data = this;
	readCallback(m_transferRead);
}

void CUSBDataLink::readCallback(libusb_transfer *transfer)
{
	CUSBDataLink* this_ = reinterpret_cast<CUSBDataLink*>(transfer->user_data);

	CLock lock(&this_->m_mutex);

	if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
	{
		// remote write end of stream hungup.  our input side
		// has therefore shutdown but don't flush our buffer
		// since there's still data to be read.
		this_->sendEvent(this_->getInputShutdownEvent());
		if (!this_->m_writable && this_->m_inputBuffer.getSize() == 0) {
			this_->sendEvent(getDisconnectedEvent());
			this_->m_connected = false;
		}
		this_->m_readable = false;

		return;
	}

	size_t n = transfer->actual_length;
	if (n > 0) {
		bool wasEmpty = (this_->m_inputBuffer.getSize() == 0);

		this_->m_inputBuffer.write(this_->m_readBuffer, (UInt32)n);
		// slurp up as much as possible
		//do {
		//	m_inputBuffer.write(buffer, (UInt32)n);
		//	n = ARCH->readSocket(m_socket, buffer, sizeof(buffer));
		//} while (n > 0);

		if (this_->m_waitAccept && !this_->m_accepted)
		{
			std::string buf("");
			buf.resize(this_->m_inputBuffer.getSize(), 0);
			this_->read((void*)buf.c_str(), buf.size());

			assert(buf == "USB_ACCEPT");

			this_->m_accepted = true;
			this_->m_accepted.broadcast();

		}
		else
		{
			// send input ready if input buffer was empty
			if (wasEmpty) {
				this_->sendEvent(this_->getInputReadyEvent());
			}
		}
	}

	libusb_fill_bulk_transfer(this_->m_transferRead, this_->m_device, this_->m_config.bulkin, (unsigned char*)this_->m_readBuffer, sizeof(this_->m_readBuffer), CUSBDataLink::readCallback, this_, -1);
	libusb_submit_transfer(this_->m_transferRead);
}

void CUSBDataLink::writeCallback(libusb_transfer *transfer)
{
	CUSBDataLink* this_ = reinterpret_cast<CUSBDataLink*>(transfer->user_data);

	CLock lock(&this_->m_mutex);

	if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
	{
		this_->sendEvent(getDisconnectedEvent());
		this_->onDisconnected();
		return;
	}

	// discard written data
	UInt32 n = transfer->actual_length;
	if (n > 0) 
	{
		this_->m_outputBuffer.pop(n);
	}

	n = this_->m_outputBuffer.getSize();
	if (n == 0) 
	{
		this_->sendEvent(this_->getOutputFlushedEvent());
		this_->m_flushed = true;
		this_->m_flushed.broadcast();
	}
	else
	{
		// write data
		const void* buffer = this_->m_outputBuffer.peek(n);
		libusb_fill_bulk_transfer(this_->m_transferWrite, this_->m_device, this_->m_config.bulkout, (unsigned char*)buffer, n, CUSBDataLink::writeCallback, this_, -1);
		libusb_submit_transfer(this_->m_transferWrite);
	}
}

void
CUSBDataLink::sendEvent(CEvent::Type type)
{
	EVENTQUEUE->addEvent(CEvent(type, getEventTarget(), NULL));
}

void
CUSBDataLink::onConnected()
{
	m_connected = true;
	m_readable  = true;
	m_writable  = true;
}

void
CUSBDataLink::onInputShutdown()
{
	m_inputBuffer.pop(m_inputBuffer.getSize());
	m_readable = false;
}

void
CUSBDataLink::onOutputShutdown()
{
	m_outputBuffer.pop(m_outputBuffer.getSize());
	m_writable = false;

	// we're now flushed
	m_flushed = true;
	m_flushed.broadcast();
}

void
CUSBDataLink::onDisconnected()
{
	// disconnected
	onInputShutdown();
	onOutputShutdown();
	m_connected = false;
}