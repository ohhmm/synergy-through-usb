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

#include "arch/Arch.h"
#include "arch/XArch.h"
#include "CUSBDataLink.h"

//
// CUSBDataLink
//

CUSBDataLink::CUSBDataLink(IEventQueue* events)
: base_t(events)
, m_device(NULL)
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
CUSBDataLink::connect(const NetworkAddress& addr)
{
	m_device = ARCH->usbOpenDevice(m_config.vid, m_config.pid, m_config.ifid);
}

//
// ISocket overrides
//

void
CUSBDataLink::bind(const NetworkAddress& addr)
{
	// not required
}

void
CUSBDataLink::close()
{
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

// TODO : use timeout or maybe use async methods?

UInt32
CUSBDataLink::read(void* buffer, UInt32 n)
{
	return ARCH->usbBulkTransfer(m_device, false, m_config.bulkin, (unsigned char*)buffer, n, 0);
}

void
CUSBDataLink::write(const void* buffer, UInt32 n)
{
	ARCH->usbBulkTransfer(m_device, true, m_config.bulkin, (unsigned char*)buffer, n, 0);
}

void
CUSBDataLink::flush()
{
	// TODO : USB
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

bool
CUSBDataLink::isReady() const
{
	// TODO : USB
	return false;
}

UInt32
CUSBDataLink::getSize() const
{
	// TODO : USB
	return 0;
}

//
// private
//

void
CUSBDataLink::init()
{
	// not required?
}
