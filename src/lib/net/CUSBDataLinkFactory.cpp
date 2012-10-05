#include "CUSBDataLinkFactory.h"
#include "CUSBDataLink.h"
#include "CUSBDataLinkListener.h"

CUSBDataLinkFactory::CUSBDataLinkFactory(IEventQueue* events)
: m_events(events)
{
}

CUSBDataLinkFactory::~CUSBDataLinkFactory(void)
{
}

IDataSocket* CUSBDataLinkFactory::create() const
{
	return new CUSBDataLink(m_events);
}

IListenSocket* CUSBDataLinkFactory::createListen() const
{
	return new CUSBDataLinkListener();
}


