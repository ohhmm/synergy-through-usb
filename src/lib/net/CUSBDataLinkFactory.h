#pragma once
#include "ISocketFactory.h"
#include "base/IEventQueue.h"

//! Factory for USB data link
class CUSBDataLinkFactory : public ISocketFactory
{
	IEventQueue* m_events;

public:
	CUSBDataLinkFactory(IEventQueue* events);
	virtual ~CUSBDataLinkFactory(void);
	// ISocketFactory overrides
	virtual IDataSocket*	create() const;
	virtual IListenSocket*	createListen() const;

};

