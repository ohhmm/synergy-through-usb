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

#ifndef CUSBDATALINK_H
#define CUSBDATALINK_H

#include "IDataTransfer.h"
#include "IArchUsbDataLink.h"

class CUSBDataLink : public IDataTransfer {
public:
	CUSBDataLink();
	~CUSBDataLink();

	// IDataTransfer overrides
	virtual void		connect(const CBaseAddress&);

	// ISocket overrides
	virtual void		bind(const CBaseAddress&);
	virtual void		close();
	virtual void*		getEventTarget() const;

	// IStream overrides
	virtual UInt32		read(void* buffer, UInt32 n);
	virtual void		write(const void* buffer, UInt32 n);
	virtual void		flush();
	virtual void		shutdownInput();
	virtual void		shutdownOutput();
	virtual bool		isReady() const;
	virtual UInt32		getSize() const;

private:
	void				init();
	void				fetchReceivedData() const;

private:
	USBDeviceHandle m_device;
	USBDataLinkConfig m_config;

	enum {USB_BUF_LEN = 4096};
	unsigned char m_input1[USB_BUF_LEN];
	unsigned char m_input2[USB_BUF_LEN];
	mutable unsigned char* m_input;
	mutable unsigned int m_inputPos; // where to write next chunk of data from system buffer
};

#endif
