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
#include "CMutex.h"
#include "CArch.h"
#include "IArchUsbDataLink.h"

//
// CUSBDataLinkListener
//

CUSBDataLinkListener::CUSBDataLinkListener()
{
	m_mutex = new CMutex;
}

CUSBDataLinkListener::~CUSBDataLinkListener()
{
	close();
	delete m_mutex;
}

void
CUSBDataLinkListener::bind(const CBaseAddress& addr)
{
	/*
	assert(addr.getAddressType() == CBaseAddress::USB);
	const CUSBAddress& usbAddress = reinterpret_cast<const CUSBAddress&>(addr);

	// TODO : USB
	m_config.idVendor = usbAddress.GetPID();
	m_config.idProduct = usbAddress.GetVID();
	m_config.ifid = 0;
	m_config.bulkin = usbAddress.GetIDBulkIN();
	m_config.bulkout = usbAddress.GetIDBulkOut();
	*/

	USBDeviceEnumerator *list;
	USBDeviceEnumerator iter;

	size_t i = 0;

	const unsigned int idVendor = 0x0402;
	const unsigned int idProduct = 0x5632;

	ARCH->usbGetDeviceList(&list);
	try
	{
		while ((iter = list[i++]) != NULL) 
		{
			USBDeviceInfo info;
			
			ARCH->usbGetDeviceInfo(iter, info);

			if (info.idVendor == idVendor &&
				info.idProduct == idProduct)
			{
				m_usbLinks.push_back(ARCH->usbOpenDevice(iter, 0));
			}
		}
	}
	catch(...)
	{
		ARCH->usbFreeDeviceList(list);
		throw;
	}

	ARCH->usbFreeDeviceList(list);
}

void
CUSBDataLinkListener::close()
{
	for (CUSBLinks::const_iterator i=m_usbLinks.cbegin(); i!=m_usbLinks.cend(); ++i)
	{
		ARCH->usbCloseDevice(*i, 0);
	}
	m_usbLinks.clear();
}

void*
CUSBDataLinkListener::getEventTarget() const
{
	return const_cast<void*>(reinterpret_cast<const void*>(this));
}

IDataTransfer*
CUSBDataLinkListener::accept()
{
	// TODO : USB
	return NULL;
}
