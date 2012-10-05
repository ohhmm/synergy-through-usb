#pragma once
#include "ITransportFactory.h"
#include "base/IEventQueue.h"

//! Factory for USB data link
class CUSBDataLinkFactory : public ITransportFactory
{
	IEventQueue* m_events;

public:
	CUSBDataLinkFactory(IEventQueue* events);
	virtual ~CUSBDataLinkFactory(void);
	// ISocketFactory overrides
	virtual IDataTransfer*	create() const;
	virtual IListenSocket*	createListen() const;

};

