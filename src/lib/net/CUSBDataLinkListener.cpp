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

#include "CLog.h"
#include "CUSBAddress.h"
#include "CUSBDataLink.h"
#include "CUSBDataLinkListener.h"
#include "CMutex.h"
#include "TMethodEventJob.h"

//
// CUSBDataLinkListener
//

CUSBDataLinkListener::CUSBDataLinkListener()
{
	m_mutex = new CMutex;

	EVENTQUEUE->adoptHandler(
			CUSBDataLink::getDeletingEvent(),
			getEventTarget(),
			new TMethodEventJob<CUSBDataLinkListener>(this, &CUSBDataLinkListener::handleDeleting, NULL));
}

CUSBDataLinkListener::~CUSBDataLinkListener()
{
	EVENTQUEUE->removeHandler(
			CUSBDataLink::getDeletingEvent(),
			getEventTarget());

	close();
	delete m_mutex;
}

void
CUSBDataLinkListener::bind(const CBaseAddress& addr)
{
	assert(addr.getAddressType() == CBaseAddress::USB);
	CLock lock(m_mutex);
	
	CUSBDataLink* dataLink = new CUSBDataLink();
	try
	{
		EVENTQUEUE->adoptHandler(
			dataLink->getInputReadyEvent(),
			dataLink->getEventTarget(),
			new TMethodEventJob<CUSBDataLinkListener>(this, &CUSBDataLinkListener::handleData, dataLink));

		dataLink->bind(addr, this);

		m_bindedLinks.insert(dataLink);
		m_addressMap[dataLink] = reinterpret_cast<const CUSBAddress&>(addr);
	}
	catch(...)
	{
		EVENTQUEUE->removeHandler(
			dataLink->getInputReadyEvent(),
			dataLink->getEventTarget());

		delete dataLink;
		throw;
	}
}

void
CUSBDataLinkListener::close()
{
	CLock lock(m_mutex);

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

	CLock lock(m_mutex);

	IDataTransfer* result = m_waitingLinks.front();

	// send kUsbAccept message to client in response to kUsbConnect
	std::string buf(kUsbAccept);
	result->write(buf.c_str(), buf.size());

	m_waitingLinks.pop_front();

	return result;
}

void CUSBDataLinkListener::handleData(const CEvent&, void* ctx)
{
	LOG((CLOG_DEBUG "USB datalink listener: process handshake"));

	IDataTransfer* dataLink = reinterpret_cast<IDataTransfer*>(ctx);

	// check if the client sent us kUsbConnect message
	std::string buf("");
		
	buf.resize(dataLink->getSize(), 0);
	dataLink->read((void*)buf.c_str(), buf.size());

	{
		CLock lock(m_mutex);

		m_bindedLinks.erase(dataLink);

		EVENTQUEUE->removeHandler(
				dataLink->getInputReadyEvent(),
				dataLink->getEventTarget());

		if (buf == kUsbConnect)
		{
			LOG((CLOG_DEBUG "USB datalink listener: connection request detected"));

			m_waitingLinks.push_back(dataLink);
			EVENTQUEUE->addEvent(CEvent(getConnectingEvent(), this, NULL));
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

void CUSBDataLinkListener::handleDeleting(const CEvent& ev, void*)
{
	LOG((CLOG_DEBUG "USB datalink listener: datalink destroyed"));

	CLock lock(m_mutex);

	IDataTransfer* dataLink = (IDataTransfer*)ev.getData();

	CUSBAddress addr = m_addressMap[dataLink];

	m_addressMap.erase(dataLink);

	bind(addr);
}
