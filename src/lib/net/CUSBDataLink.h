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
#include "io/StreamBuffer.h"
#include "mt/CondVar.h"
#include "mt/Mutex.h"
#include "arch/IArchUsbDataLink.h"
#include <libusb.h>

//
// handshake messages
//

extern const char*		kUsbConnect;
extern const char*		kUsbAccept;
extern const char*		kUsbReject;

class IUSBDataLinkListenerEvents {
public:
	virtual void onDataLinkDestroyed(IDataTransfer* dataLink) = 0;
};

class CUSBDataLink : public IDataTransfer {
    typedef IDataTransfer base_t;
public:
    CUSBDataLink(IEventQueue* events);
	~CUSBDataLink();

	// IDataTransfer overrides
	virtual void		connect(const BaseAddress &);

	// ISocket overrides
	virtual void		bind(const BaseAddress&);
	virtual void		bind(const BaseAddress&, IUSBDataLinkListenerEvents* listenerEvents);
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
	void				initConnection(const BaseAddress&);
	void				scheduleWrite();

	static void LIBUSB_CALL	readCallback(libusb_transfer *transfer);
	static void	LIBUSB_CALL writeCallback(libusb_transfer *transfer);

	void				sendEvent(Event::Type);

	void				onInputShutdown();
	void				onOutputShutdown();
	void				onDisconnected();

private:
	IEventQueue*		m_events;

	IUSBDataLinkListenerEvents * m_listenerEvents;

	USBDeviceHandle		m_device;
	USBDataLinkConfig	m_config;

	libusb_transfer*	m_transferRead;
	libusb_transfer*	m_transferWrite;

	Mutex				m_mutex;
	char				m_readBuffer[1024*1024];
	StreamBuffer		m_inputBuffer;
	StreamBuffer		m_outputBuffer;
	CondVar<bool>		m_flushed;
	bool				m_connected;
	bool				m_readable;
	bool				m_writable;

	//CMutex				m_acceptedMutex;
	CondVar<bool>		m_acceptedFlag;

	//CMutex				m_activeTransfersMutex;
	CondVar<int>		m_activeTransfers;

	int					m_transferLeft;
};

#endif
