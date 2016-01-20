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
#include "IDataSocket.h"
#include "mt/Lock.h"
#include "mt/Thread.h"
#include <set>
#include <deque>


//! USB data link cable listen
/*!
 Data link connection listener class.
*/
class CUSBDataLinkListener : public IListenSocket {
public:
	CUSBDataLinkListener(IEventQueue* events);
	~CUSBDataLinkListener();

	// ISocket overrides
	virtual void		bind(const BaseAddress &);
	virtual void		close();
	virtual void*		getEventTarget() const;

	// IListenSocket overrides
	virtual IDataSocket*	accept();

private:
	void				handleData(const Event&, void*);

	typedef std::set<IDataSocket*> CUSBLinkSet;
	typedef std::deque<IDataSocket*> CUSBLinkDeque;
    
	CUSBLinkSet			m_bindedLinks;
	CUSBLinkDeque		m_waitingLinks;
	CUSBLinkSet			m_activeLinks;

	Mutex*				m_mutex;
	IEventQueue*		m_events;
};

#endif
