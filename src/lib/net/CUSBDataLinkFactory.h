#pragma once
#include "ITransportFactory.h"

//! Factory for USB data link
class CUSBDataLinkFactory : public ITransportFactory
{
public:
	CUSBDataLinkFactory(void);
	virtual ~CUSBDataLinkFactory(void);
	// ISocketFactory overrides
	virtual IDataTransfer*	create() const;
	virtual IListenSocket*	createListen() const;

};

