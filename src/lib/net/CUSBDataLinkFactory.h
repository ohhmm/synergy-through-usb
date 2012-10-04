#pragma once
#include "isocketfactory.h"

//! Factory for USB data link
class CUSBDataLinkFactory : public ISocketFactory
{
public:
	CUSBDataLinkFactory(void);
	virtual ~CUSBDataLinkFactory(void);
	// ISocketFactory overrides
	virtual IDataTransfer*	create() const;
	virtual IListenSocket*	createListen() const;

};

