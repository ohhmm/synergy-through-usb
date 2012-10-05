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

#include "CUSBDataLinkListener.h"
#include "CUSBAddress.h"

//
// CUSBDataLinkListener
//

CUSBDataLinkListener::CUSBDataLinkListener()
{

}

CUSBDataLinkListener::~CUSBDataLinkListener()
{

}

void
CUSBDataLinkListener::bind(const CBaseAddress& addr)
{
	assert(addr.getAddressType() == CBaseAddress::USB);
	const CUSBAddress& usbAddress = reinterpret_cast<const CUSBAddress&>(addr);

	// TODO : USB
	m_config.idVendor = usbAddress.GetPID();
	m_config.idProduct = usbAddress.GetVID();
	m_config.ifid = 0;
	m_config.bulkin = usbAddress.GetIDBulkIN();
	m_config.bulkout = usbAddress.GetIDBulkOut();

}

void
CUSBDataLinkListener::close()
{
	// TODO : USB
}

void*
CUSBDataLinkListener::getEventTarget() const
{
	// TODO : USB
	return NULL;
}

IDataTransfer*
CUSBDataLinkListener::accept()
{
	// TODO : USB
	return NULL;
}
