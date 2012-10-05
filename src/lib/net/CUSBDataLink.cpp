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
#include "CUSBDataLink.h"
#include "CUSBAddress.h"

//
// CUSBDataLink
//

CUSBDataLink::CUSBDataLink() :
	m_device(NULL),
	m_input(m_input1),
	m_inputPos(0)
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
	assert(addr.getAddressType() == CBaseAddress::USB);
	const CUSBAddress& usbAddress(reinterpret_cast<const CUSBAddress&>(addr));

	// TODO: USB : fill m_config from addr
	m_config.vid = 0x0402;
	m_config.pid = 0x5632;
	m_config.ifid = 0;
	m_config.bulkin = 0x81;
	m_config.bulkout = 0x02;

	m_device = ARCH->usbOpenDevice(m_config.vid, m_config.pid, m_config.ifid);
}

//
// ISocket overrides
//

void
CUSBDataLink::bind(const CBaseAddress& addr)
{
	// not required
}

void
CUSBDataLink::close()
{
	m_inputPos = 0;

	if (m_device)
	{
		ARCH->usbCloseDevice(m_device, m_config.ifid);
		m_device = NULL;
	}
}

void*
CUSBDataLink::getEventTarget() const
{
	return const_cast<void*>(reinterpret_cast<const void*>(this)); // WTF?
}

//
// IStream overrides
//

bool
CUSBDataLink::isReady() const
{
	fetchReceivedData();
	return (m_inputPos > 0);
}

UInt32
CUSBDataLink::getSize() const
{
	fetchReceivedData();
	return m_inputPos;
}

void
CUSBDataLink::fetchReceivedData() const
{
	if (m_inputPos < USB_BUF_LEN)
	{
		// input buffer empty space
		unsigned int n = USB_BUF_LEN - m_inputPos;

		// using small timeout to fetch already received data
		unsigned int transferred = ARCH->usbTryBulkTransfer(m_device, false, m_config.bulkin, m_input + m_inputPos, n, 1);
		m_inputPos += transferred;
	}
}

UInt32
CUSBDataLink::read(void* buffer, UInt32 n)
{
	unsigned char* dst = (unsigned char*)buffer;
	unsigned int bufferLen = n;

	if (m_inputPos > 0) // some data already available in local buffer
	{
		unsigned char* src = m_input; // save pointer to input buffer because it may be changed
		unsigned int readLen = m_inputPos;

		if (readLen > bufferLen) // buffer can't fit all available data
		{
			readLen = bufferLen; // clamp to max size

			// get pointer to new buffer where to store remaining data
			if (m_input == m_input1)
			{
				m_input = m_input2;
			}
			else
			{
				m_input = m_input1;
			}

			// copy remaining data
			memcpy(m_input, src + readLen, m_inputPos - readLen);
		}

		memcpy(dst, src, readLen);
		dst += readLen;
		bufferLen -= readLen;
		m_inputPos -= readLen;
	}

	// blocking
	return ARCH->usbBulkTransfer(m_device, false, m_config.bulkin, dst, bufferLen, 0);
}

void
CUSBDataLink::write(const void* buffer, UInt32 n)
{
	// blocking
	ARCH->usbBulkTransfer(m_device, true, m_config.bulkout, (unsigned char*)buffer, n, 0);
}

void
CUSBDataLink::flush()
{
	// not required?
}

void
CUSBDataLink::shutdownInput()
{
	// not required?
}

void
CUSBDataLink::shutdownOutput()
{
	// not required?
}

//
// private
//

void
CUSBDataLink::init()
{
	// not required?
}
