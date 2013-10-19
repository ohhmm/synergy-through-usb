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

#ifndef CUSBDataLinkListener_H
#define CUSBDataLinkListener_H

#include "arch/IArchUsbDataLink.h"
#include "IListenSocket.h"
#include "IDataTransfer.h"
#include "mt/Lock.h"
#include "mt/Thread.h"
#include "CUSBAddress.h"
#include <set>
#include <deque>
#include <map>


//! USB data link cable listen
/*!
 Data link connection listener class.
*/
class CUSBDataLinkListener : public IListenSocket {
public:
	CUSBDataLinkListener(IEventQueue* events);
	virtual ~CUSBDataLinkListener();

	// ISocket overrides
	virtual void		bind(const BaseAddress &);
	virtual void		close();
	virtual void*		getEventTarget() const;

	// IListenSocket overrides
	virtual IDataTransfer*	accept();

private:
	void				handleData(const Event&, void*);
	void				handleDeleting(const Event&, void*);

	typedef std::set<IDataTransfer*> CUSBLinkSet;
	typedef std::deque<IDataTransfer*> CUSBLinkDeque;
	typedef std::map<IDataTransfer*, CUSBAddress> CAddressMap;

	CUSBLinkSet			m_bindedLinks;
	CUSBLinkDeque		m_waitingLinks;
	CAddressMap			m_addressMap;

	Mutex*				m_mutex;
	IEventQueue*		m_events;
};

#endif
