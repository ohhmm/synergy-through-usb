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
#include "CUSBDataLinkListener.h"
#include "CUSBAddress.h"
#include "CMutex.h"
#include "CArch.h"
#include "IArchUsbDataLink.h"
#include "CUSBDataLink.h"
#include "TMethodEventJob.h"
#include <algorithm>

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
	CLock lock(m_mutex);
	
	IDataTransfer* dataLink = new CUSBDataLink();
	try
	{
		dataLink->bind(addr);
		
		EVENTQUEUE->adoptHandler(
			dataLink->getInputReadyEvent(),
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
	CLock lock(m_mutex);

	for (CUSBLinkSet::const_iterator i=m_bindedLinks.cbegin(); i!=m_bindedLinks.cend(); ++i)
	{
		delete *i;
	}
	m_bindedLinks.clear();

	for (CUSBLinkDeque::const_iterator i=m_waitingLinks.cbegin(); i!=m_waitingLinks.cend(); ++i)
	{
		delete *i;
	}
	m_waitingLinks.clear();

	for (CUSBLinkSet::const_iterator i=m_activeLinks.cbegin(); i!=m_activeLinks.cend(); ++i)
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

IDataTransfer*
CUSBDataLinkListener::accept()
{
	CLock lock(m_mutex);

	IDataTransfer* result = m_waitingLinks.front();
	m_waitingLinks.pop_front();

	m_activeLinks.insert(result);

	std::string buf("USB_ACCEPT");
	result->write(buf.c_str(), buf.size());

	return result;
}

void CUSBDataLinkListener::handleData(const CEvent&, void* ctx)
{
	LOG((CLOG_PRINT "CUSBDataLinkListener::handleData"));

	IDataTransfer* dataLink = reinterpret_cast<IDataTransfer*>(ctx);

	EVENTQUEUE->removeHandler(
			dataLink->getInputReadyEvent(),
			dataLink->getEventTarget());

	std::string buf("");
		
	buf.resize(dataLink->getSize(), 0);
	dataLink->read((void*)buf.c_str(), buf.size());

	assert(buf == "USB_CONNECT");

	{
		CLock lock(m_mutex);

		m_bindedLinks.erase(dataLink);
		m_waitingLinks.push_back(dataLink);
	}

	EVENTQUEUE->addEvent(CEvent(getConnectingEvent(), this, NULL));
}
