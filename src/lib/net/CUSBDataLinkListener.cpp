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
}

CUSBDataLinkListener::~CUSBDataLinkListener()
{
	close();
	delete m_mutex;
}

void
CUSBDataLinkListener::bind(const BaseAddress & addr)
{
	Lock lock(m_mutex);
	
	IDataSocket* dataLink = new CUSBDataLink(m_events);
	try
	{
		dataLink->bind(addr);
		
		m_events->adoptHandler(
            m_events->forIStream().inputReady(),
			dataLink->getEventTarget(),
			new TMethodEventJob<CUSBDataLinkListener>(this, &CUSBDataLinkListener::handleData, dataLink));

		m_bindedLinks.insert(dataLink);
	}
	catch(...)
	{
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

	for (CUSBLinkSet::const_iterator i=m_activeLinks.begin(); i!=m_activeLinks.end(); ++i)
	{
		delete *i;
	}
	m_activeLinks.clear();
}

void*
CUSBDataLinkListener::getEventTarget() const
{
	return const_cast<void*>(reinterpret_cast<const void*>(this));
}

IDataSocket*
CUSBDataLinkListener::accept()
{
	Lock lock(m_mutex);

	IDataSocket* result = m_waitingLinks.front();
	m_waitingLinks.pop_front();

	m_activeLinks.insert(result);

	std::string buf("USB_ACCEPT");
	result->write(buf.c_str(), buf.size());

	return result;
}

void CUSBDataLinkListener::handleData(const Event&, void* ctx)
{
	LOG((CLOG_PRINT "CUSBDataLinkListener::handleData"));

	IDataSocket* dataLink = reinterpret_cast<IDataSocket*>(ctx);

	m_events->removeHandler(
			m_events->forIStream().inputReady(),
			dataLink->getEventTarget());

	std::string buf("");
		
	buf.resize(dataLink->getSize(), 0);
	dataLink->read((void*)buf.c_str(), buf.size());

	assert(buf == "USB_CONNECT");

	{
		Lock lock(m_mutex);

		m_bindedLinks.erase(dataLink);
		m_waitingLinks.push_back(dataLink);
	}

	m_events->addEvent(Event(m_events->forIListenSocket().connecting(), this, NULL));
}
