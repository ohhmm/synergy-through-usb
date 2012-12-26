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

#include "CLog.h"
#include "CArch.h"
#include "XArch.h"
#include "XSocket.h"
#include "CLock.h"
#include "CEvent.h"
#include "CStopwatch.h"
#include "CUSBDataLink.h"
#include "CUSBAddress.h"

const char*		kUsbConnect	= "USB_CONNECT";
const char*		kUsbAccept	= "USB_ACCEPT";
const char*		kUsbReject	= "USB_REJECT";

const unsigned int	TRANSFER_TIMEOUT = 2*1000;

enum message_id {
	MSGID_HANDSHAKE		= 0,
	MSGID_NORMAL		= 1,
	MSGID_DISCONNECT	= 2,
	MSGID_LAST			= MSGID_DISCONNECT
};

struct message_hdr {
	message_id		id;
	unsigned int	data_size;
};

CEvent::Type CUSBDataLink::s_deletingEvent = CEvent::kUnknown;

//
// CUSBDataLink
//

CUSBDataLink::CUSBDataLink() :
	m_listener(NULL),
	m_device(NULL),
	m_transferRead(NULL),
	m_transferWrite(NULL),
	m_mutex(),
	m_flushed(&m_mutex, true),
	m_connected(false),
	m_readable(false),
	m_writable(false),
	m_acceptedFlag(&m_mutex, false),
	m_activeTransfers(&m_mutex, 0),
    m_writeBuffer(NULL),
    m_writeBufferSize(0),
    m_writeBufferSent(0),
	m_leftToWrite(0),
	m_leftToRead(0)
{
	memset(&m_config, 0, sizeof(m_config));
}

CUSBDataLink::~CUSBDataLink()
{
	m_writable = false;
	try {
		close();
	} 
	catch (...) {
		// ignore
	}

	// if listener is registered then send notification about our destruction
	if( m_listener != NULL )
	{
		EVENTQUEUE->addEvent(CEvent(getDeletingEvent(), m_listener, this, CEvent::kDontFreeData));
		m_listener = NULL;
	}
}


//
// IDataTransfer overrides
//

void
CUSBDataLink::connect(const CBaseAddress& addr)
{
	close();

	initConnection(addr);
	
	LOG((CLOG_DEBUG "USB datalink: send connection request"));

	std::string buf(kUsbConnect);
	
	write(buf.c_str(), buf.size());

	{
		CLock lock(&m_mutex);

		CStopwatch stopwatch(false);
		bool bCancelled = false;

		LOG((CLOG_DEBUG "USB datalink: wait for connection to be accepted..."));
		while (m_acceptedFlag == false) {
			m_acceptedFlag.wait(2.0);

			if (!bCancelled && !m_acceptedFlag && (stopwatch.getTime() > 2.0))
			{
				LOG((CLOG_DEBUG "USB datalink: connection accepting timeout"));
				libusb_cancel_transfer(m_transferRead);
				bCancelled = true;
				break;
			}
		}

		if (!m_connected) {
			throw XSocketConnect("connection failed");
		}

		LOG((CLOG_DEBUG "USB datalink: connection accepted."));
		sendEvent(getConnectedEvent());
	}
}

//
// ISocket overrides
//

void
CUSBDataLink::bind(const CBaseAddress& addr)
{
	initConnection(addr);
	
	{
		CLock lock(&m_mutex);
		// set accepted flag, because this is a listen socket.
		m_acceptedFlag = true;
	}
}

void
CUSBDataLink::bind(const CBaseAddress& addr, void* listener)
{
	assert(m_listener == NULL);

	m_listener = listener;
	bind(addr);
}

void
CUSBDataLink::close()
{
	CLock lock(&m_mutex);

	// clear buffers and enter disconnected state
	if (m_connected) {

		if (m_writable)
		{
			// send disconnect message
			message_hdr hdr;

			hdr.id = MSGID_DISCONNECT;
			hdr.data_size = 0;

			doWrite(&hdr, (void*)0);

			// wait for message to be delivered
			while (m_flushed == false) {
				m_flushed.wait();
			}
		}

		onDisconnect();
	}

	// Cancel active transfers.
	// If there was no transfer yet, than dev_handle == 0 and we should not call libusb_cancel_transfer because it causes AV in this case.
	if (m_transferRead && m_transferRead->dev_handle) {
		onInputShutdown();
		libusb_cancel_transfer(m_transferRead);
	}

	if (m_transferWrite && m_transferWrite->dev_handle) {
		onOutputShutdown();
		libusb_cancel_transfer(m_transferWrite);
	}
	
	// wait for transfers to finish
	while (m_activeTransfers > 0) {
		m_activeTransfers.wait();
	}
	
	if (m_transferRead)	{
		libusb_free_transfer(m_transferRead);
		m_transferRead = NULL;
	}

	if (m_transferWrite) {
		libusb_free_transfer(m_transferWrite);
		m_transferWrite = NULL;
	}

	// close the usb device
	if (m_device) {
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

CEvent::Type
CUSBDataLink::getDeletingEvent()
{
	return EVENTQUEUE->registerTypeOnce(s_deletingEvent,
							"CUSBDataLink::connecting");
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
		onDisconnect();
	}

	return n;
}

void
CUSBDataLink::write(const void* buffer, UInt32 n)
{
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

	message_hdr hdr;
	hdr.id = m_connected? MSGID_NORMAL : MSGID_HANDSHAKE;
	hdr.data_size = n;

	// if this is a server - set m_connected after first write, because we send kUsbAccept
	if (m_listener)
		m_connected = true;

	doWrite(&hdr, buffer);
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
	CLock lock(&m_mutex);

	assert(addr.getAddressType() == CBaseAddress::USB);
	CUSBAddress& usbAddress = const_cast<CUSBAddress&>(
		reinterpret_cast<const CUSBAddress&>(addr));

	m_config.idVendor = usbAddress.GetVID();
	m_config.idProduct = usbAddress.GetPID();
	m_config.busNumber = usbAddress.GetIDBus();
	m_config.deviceAddress = usbAddress.GetIDDeviceOnBus();
	m_config.ifid = 0;
	m_config.bulkin = usbAddress.GetIDBulkIN();
	m_config.bulkout = usbAddress.GetIDBulkOut();

	try 
	{
		m_device = ARCH->usbOpenDevice(m_config, m_config.ifid);	
		if (!m_device) {
			usbAddress.resolve();

			m_config.idVendor = usbAddress.GetVID();
			m_config.idProduct = usbAddress.GetPID();
			m_config.busNumber = usbAddress.GetIDBus();
			m_config.deviceAddress = usbAddress.GetIDDeviceOnBus();
			m_config.ifid = 0;
			m_config.bulkin = usbAddress.GetIDBulkIN();
			m_config.bulkout = usbAddress.GetIDBulkOut();

			m_device = ARCH->usbOpenDevice(m_config, m_config.ifid);	
			if (!m_device) 
				throw XSocketConnect("Open device failed");
		}

		m_transferRead = libusb_alloc_transfer(0);
		if (!m_transferRead) {
			throw XSocketConnect("alloc read transfer failed");
		}
		m_transferRead->dev_handle = 0;
		
		m_transferWrite = libusb_alloc_transfer(0);
		if (!m_transferWrite) {
			throw XSocketConnect("alloc write transfer failed");
		}		
		m_transferWrite->dev_handle = 0;
	} 
	catch (...) 
	{
		if (m_transferRead)	{
			libusb_free_transfer(m_transferRead);
			m_transferRead = NULL;
		}

		if (m_transferWrite) {
			libusb_free_transfer(m_transferWrite);
			m_transferWrite = NULL;
		}

		// close the usb device
		if (m_device) {
			ARCH->usbCloseDevice(m_device, m_config.ifid);
			m_device = NULL;
		}

		throw;
	}

	m_readable = true;
	m_writable = true;

	// Start pooling input endpoint.
	m_activeTransfers = m_activeTransfers + 1;
	m_transferRead->status = LIBUSB_TRANSFER_COMPLETED;
	m_transferRead->actual_length = 0;
	m_transferRead->user_data = this;
	readCallback(m_transferRead);
}

void CUSBDataLink::scheduleWrite()
{
	// No active write transfers at this moment. Call writeCallback to start new write transfer.
	assert(m_activeTransfers < 2);
	m_activeTransfers = m_activeTransfers + 1;

	m_transferWrite->status = LIBUSB_TRANSFER_COMPLETED;
	m_transferWrite->actual_length = 0;
	m_transferWrite->user_data = this;
	writeCallback(m_transferWrite);
}

void CUSBDataLink::readCallback(libusb_transfer *transfer)
{
	CUSBDataLink* this_ = reinterpret_cast<CUSBDataLink*>(transfer->user_data);

	CLock lock(&this_->m_mutex);

	this_->m_activeTransfers = this_->m_activeTransfers - 1;
	if (!this_->m_readable)
	{
		this_->m_activeTransfers.signal();
		return;
	}

	switch (transfer->status)
	{
	case LIBUSB_TRANSFER_COMPLETED:
		break;
	
	case LIBUSB_TRANSFER_CANCELLED:
		if (!this_->m_acceptedFlag)
		{
			LOG((CLOG_DEBUG "USB datalink: accept waiting cancelled"));
			this_->m_acceptedFlag = true;
			this_->m_acceptedFlag.broadcast();
			break;
		}
		// go to default here

	default:
		this_->onDisconnect();
		return;
	}


	size_t n = transfer->actual_length;
	if (n > 0) 
	{	
		bool checkHeader = false;

		message_hdr hdr;
		char* data_ptr = this_->m_readBuffer;

		while (n > 0) 
		{
			if (!this_->m_leftToRead)
			{
				assert(n >= sizeof(hdr));
			
				memcpy(&hdr, data_ptr, sizeof(hdr));
				n -= sizeof(hdr);
				data_ptr += sizeof(hdr);
				this_->m_leftToRead = hdr.data_size;

				if (static_cast<unsigned>(hdr.id) > MSGID_LAST)
				{
					LOG((CLOG_ERR "USB datalink: invalid data packet. Unknown MSGID"));
					this_->onDisconnect();
					return;
				}

				checkHeader = hdr.id != MSGID_NORMAL;

				// if we check header after collecting data, than all the buffer must be sent within this transfer
				assert(!checkHeader || hdr.data_size <= n);
                if (checkHeader && hdr.data_size > n)
                {
					LOG((CLOG_ERR "USB datalink: invalid data packet. Data size is too big."));
					this_->onDisconnect();
					return;
                }
			}

			size_t chunkSize = this_->m_leftToRead;
			if (chunkSize > n)
			{
				chunkSize = n;
			}

			this_->m_inputBuffer.write(data_ptr, chunkSize);

			assert(this_->m_leftToRead >= chunkSize);
			this_->m_leftToRead -= chunkSize;
			n -= chunkSize;
			data_ptr += chunkSize;

			if (!this_->m_leftToRead && checkHeader) 
			{
				switch (hdr.id)
				{
				case MSGID_HANDSHAKE:
					if (this_->m_connected)
					{
						LOG((CLOG_ERR "USB datalink: unexpected handshake packet"));
						this_->onDisconnect();
						return;
					}
					else if (this_->m_acceptedFlag)
					{
						// server got handshake request. remove any previously read data.
						LOG((CLOG_DEBUG "USB datalink: server got handshake packet"));
						this_->m_inputBuffer.pop(this_->m_inputBuffer.getSize() - hdr.data_size);
					}
					else
					{
						std::string buf("");
						buf.resize(this_->m_inputBuffer.getSize(), 0);
						this_->read((void*)buf.c_str(), buf.size());

						if (buf == kUsbAccept)
						{
							LOG((CLOG_DEBUG "USB datalink: connection accept detected"));
							this_->m_connected = true;	
						}
						else
						{
							LOG((CLOG_ERR "USB datalink: unexpected data during handshake, connection is not accepted"));	
							// Just skip setting m_connected in this case. "Connect" funcion detects this and drops exception
						}

						this_->m_acceptedFlag = true;
						this_->m_acceptedFlag.broadcast();

						if (!this_->m_connected)
							return;
					}
					break;
			
				case MSGID_NORMAL: 
					break;
				case MSGID_DISCONNECT:
					assert(n == 0);
					LOG((CLOG_DEBUG "USB datalink: disconnect"));
					this_->onDisconnect();
					break;
				default:
					LOG((CLOG_ERR "USB datalink: invalid data packet"));
					this_->onDisconnect();
					return;
				}
			}

		}

		if ((this_->m_inputBuffer.getSize() > 0) && !this_->m_leftToRead) {
			this_->sendEvent(this_->getInputReadyEvent());
		}

	}

	libusb_fill_bulk_transfer(
		this_->m_transferRead, 
		this_->m_device, 
		this_->m_config.bulkin, 
		(unsigned char*)this_->m_readBuffer, 
		sizeof(this_->m_readBuffer), 
		CUSBDataLink::readCallback, 
		this_, 
		-1);
	
	if (libusb_submit_transfer(this_->m_transferRead) != 0)
	{
		this_->onDisconnect();
		return;
	}

	this_->m_activeTransfers = this_->m_activeTransfers + 1;
}

void CUSBDataLink::writeCallback(libusb_transfer *transfer)
{
	CUSBDataLink* this_ = reinterpret_cast<CUSBDataLink*>(transfer->user_data);

	CLock lock(&this_->m_mutex);

	this_->m_activeTransfers = this_->m_activeTransfers - 1;
	if (!this_->m_writable)
	{
		this_->m_activeTransfers.signal();
		return;
	}

	if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
	{
		if (!this_->m_acceptedFlag)
		{
			this_->m_acceptedFlag = true;
			this_->m_acceptedFlag.broadcast();
		}

		this_->onDisconnect();
		return;
	}

	// discard written data
	UInt32 n = transfer->actual_length;
	if (n > 0) 
	{
		assert(this_->m_leftToWrite >= n);
        assert(this_->m_writeBufferSent + n <= this_->m_writeBufferSize);

		this_->m_leftToWrite -= n;
        this_->m_writeBufferSent += n;
	}

    // check if we have finished to transfer the message
    if (this_->m_leftToWrite == 0)
    {
        // if all the write buffer was sent then get next buffer 
        if (this_->m_writeBufferSent >= this_->m_writeBufferSize)
        {
		    this_->m_outputBuffer.pop(this_->m_writeBufferSent);

            this_->m_writeBufferSent = 0;            
            this_->m_writeBufferSize = this_->m_outputBuffer.getSize();
            this_->m_writeBuffer = (char*)this_->m_outputBuffer.peek(this_->m_writeBufferSize);
        }

        if (this_->m_writeBufferSize > 0)
            this_->m_leftToWrite = sizeof(message_hdr) + ((message_hdr*)(this_->m_writeBuffer + this_->m_writeBufferSent))->data_size;
    }

    if (this_->m_leftToWrite == 0) 
	{
        // no data to transfer. notify that write buffer flushed
        assert(this_->m_outputBuffer.getSize() == 0);

		if (!this_->m_flushed)
		{
			this_->sendEvent(this_->getOutputFlushedEvent());
			this_->m_flushed = true;
			this_->m_flushed.broadcast();
		}
	}
	else
	{
        // schedule the transfer of the next message
        n = this_->m_leftToWrite;
		
        // the size of the data sent must not exceed read buffer size.
		if (n > sizeof(this_->m_readBuffer))
			n = sizeof(this_->m_readBuffer);

		libusb_fill_bulk_transfer(
			this_->m_transferWrite, 
			this_->m_device, 
			this_->m_config.bulkout, 
			(unsigned char*)(this_->m_writeBuffer + this_->m_writeBufferSent), 
			n, 
			CUSBDataLink::writeCallback, 
			this_, 
			TRANSFER_TIMEOUT);
		
		if (libusb_submit_transfer(this_->m_transferWrite) != 0)
		{
			LOG((CLOG_DEBUG "USB datalink: failed to write packet"));
			this_->onDisconnect();
			return;
		}

		this_->m_activeTransfers = this_->m_activeTransfers + 1;
	}
}

void
CUSBDataLink::sendEvent(CEvent::Type type)
{
	EVENTQUEUE->addEvent(CEvent(type, getEventTarget()));
}

void CUSBDataLink::doWrite(const message_hdr* hdr, const void* buffer)
{
	bool wasEmpty = (m_outputBuffer.getSize() == 0);

	m_outputBuffer.write(hdr, sizeof(message_hdr));

	if (hdr->data_size > 0)
		m_outputBuffer.write(buffer, hdr->data_size);

	//assert(m_outputBuffer.getSize() <= sizeof(m_writeBuffer));

	// there's data to write
	m_flushed = false;

	if (wasEmpty)
		scheduleWrite();
}

void CUSBDataLink::onDisconnect()
{
	if (m_readable)
	{
		sendEvent(getInputShutdownEvent());
		onInputShutdown();
	}

	if (m_writable)
	{
		sendEvent(getOutputShutdownEvent());
		onOutputShutdown();
	}
	
	sendEvent(getDisconnectedEvent());
	m_connected = false;
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
