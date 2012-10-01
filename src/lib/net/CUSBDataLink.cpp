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

#include "CUSBDataLink.h"

//
// CUSBDataLink
//

CUSBDataLink::CUSBDataLink()
{
	// TODO : USB

	init();
}

CUSBDataLink::~CUSBDataLink()
{
	// TODO : USB
}

void
CUSBDataLink::bind(const CNetworkAddress& addr)
{
	// TODO : USB
}

void
CUSBDataLink::close()
{
	// TODO : USB
}

void*
CUSBDataLink::getEventTarget() const
{
	// TODO : USB
	return nullptr;
}

UInt32
CUSBDataLink::read(void* buffer, UInt32 n)
{
	// TODO : USB
	return 0;
}

void
CUSBDataLink::write(const void* buffer, UInt32 n)
{
	// TODO : USB
}

void
CUSBDataLink::flush()
{
	// TODO : USB
}

void
CUSBDataLink::shutdownInput()
{
	// TODO : USB
}

void
CUSBDataLink::shutdownOutput()
{
	// TODO : USB
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

void
CUSBDataLink::connect(const CNetworkAddress& addr)
{
	// TODO : USB
}

void
CUSBDataLink::init()
{
	// TODO : USB
}
