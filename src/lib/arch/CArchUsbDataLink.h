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

#ifndef CARCHUSBDATALINK_H
#define CARCHUSBDATALINK_H

#include "IArchUsbDataLink.h"

#define ARCH_USB CArchUsbDataLink

//! Class for architecture dependent USB
/*!
 This interface defines the USB transport operations for
 platforms supported by libusb.
 */
class CArchUsbDataLink : public IArchUsbDataLink {
public:
	CArchUsbDataLink();
	virtual ~CArchUsbDataLink();
	
	void init();
	void usbInit();
	void usbShut();
	USBContextHandle usbGetContext();

	USBDeviceHandle usbOpenDevice(int vid, int pid, int ifid);
	void usbCloseDevice(USBDeviceHandle dev, int ifid);
	int usbBulkTransfer(USBDeviceHandle dev, bool write, unsigned char port, unsigned char* buf, unsigned int len, unsigned int timeout);

private:
	USBContextHandle m_usbContext;
};

#endif
