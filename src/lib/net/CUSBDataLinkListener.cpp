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

#include "base/Log.h"
#include "CUSBAddress.h"
#include "CUSBDataLink.h"
#include "CUSBDataLinkListener.h"
#include "base/TMethodEventJob.h"
#include <algorithm>
#include "arch/Arch.h"
#include "mt/Mutex.h"

//
// CUSBDataLinkListener
//

CUSBDataLinkListener::CUSBDataLinkListener(IEventQueue* events)
: m_events(events)
{
	m_mutex = new Mutex;

	m_events->adoptHandler(
			CUSBDataLink::getDeletingEvent(m_events),
			getEventTarget(),
			new TMethodEventJob<CUSBDataLinkListener>(this, &CUSBDataLinkListener::handleDeleting, NULL));
}

CUSBDataLinkListener::~CUSBDataLinkListener()
{
	m_events->removeHandler(
			CUSBDataLink::getDeletingEvent(m_events),
			getEventTarget());

	close();
	delete m_mutex;
}

void
CUSBDataLinkListener::bind(const BaseAddress & addr)
{
	Lock lock(m_mutex);

    CUSBDataLink* dataLink = new CUSBDataLink(m_events);
	try
	{
		m_events->adoptHandler(
			m_events->forIStream().inputReady(),
			dataLink->getEventTarget(),
			new TMethodEventJob<CUSBDataLinkListener>(this, &CUSBDataLinkListener::handleData, dataLink));

		dataLink->bind(addr, this);

		m_bindedLinks.insert(dataLink);

		{
			const CUSBAddress& usbAddress = reinterpret_cast<const CUSBAddress&>(addr);
			m_addressMap[dataLink] = usbAddress;
		}
	}
	catch(...)
	{
		m_events->removeHandler(
			m_events->forIStream().inputReady(),
			dataLink->getEventTarget());

		delete dataLink;
		throw;
	}
}

void
CUSBDataLinkListener::close()
{
	Lock lock(m_mutex);

	for (CUSBLinkSet::const_iterator i=m_bindedLinks.begin(); i!=m_bindedLinks.end(); ++i)
	{
		delete *i;
	}
	m_bindedLinks.clear();

	for (CUSBLinkDeque::const_iterator i=m_waitingLinks.begin(); i!=m_waitingLinks.end(); ++i)
	{
		delete *i;
	}
	m_waitingLinks.clear();
}

void*
CUSBDataLinkListener::getEventTarget() const
{
	return const_cast<void*>(reinterpret_cast<const void*>(this));
}

IDataTransfer*
CUSBDataLinkListener::accept()
{
	LOG((CLOG_DEBUG "USB datalink listener: accept connection"));

	Lock lock(m_mutex);

	IDataTransfer* result = m_waitingLinks.front();

	// send kUsbAccept message to client in response to kUsbConnect
	std::string buf(kUsbAccept);
	result->write(buf.c_str(), buf.size());

	m_waitingLinks.pop_front();

	return result;
}

void CUSBDataLinkListener::handleData(const Event&, void* ctx)
{
	LOG((CLOG_DEBUG "USB datalink listener: process handshake"));

	IDataTransfer* dataLink = reinterpret_cast<IDataTransfer*>(ctx);

	// check if the client sent us kUsbConnect message
	std::string buf("");
		
	buf.resize(dataLink->getSize(), 0);
	dataLink->read((void*)buf.c_str(), buf.size());

	{
		Lock lock(m_mutex);

		m_bindedLinks.erase(dataLink);

		m_events->removeHandler(
				m_events->forIStream().inputReady(),
				dataLink->getEventTarget());

		if (buf == kUsbConnect)
		{
			LOG((CLOG_DEBUG "USB datalink listener: connection request detected"));

			m_waitingLinks.push_back(dataLink);
            m_events->addEvent(Event(m_events->forIListenSocket().connecting(), this, NULL));
		}
		else
		{
			LOG((CLOG_DEBUG "USB datalink listener: unexpected data during handshake, drop connection"));

			buf = kUsbReject;
			dataLink->write(buf.c_str(), buf.size());
			dataLink->flush();

			delete dataLink;
		}
	}
}

void CUSBDataLinkListener::handleDeleting(const Event& ev, void*)
{
	LOG((CLOG_DEBUG "USB datalink listener: datalink destroyed"));

	Lock lock(m_mutex);

	IDataTransfer* dataLink = (IDataTransfer*)ev.getData();

	CUSBAddress addr = m_addressMap[dataLink];

	m_addressMap.erase(dataLink);

	bind(addr);
}