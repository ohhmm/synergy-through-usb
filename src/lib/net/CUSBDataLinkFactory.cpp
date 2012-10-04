#include "CUSBDataLinkFactory.h"
#include "CUSBDataLink.h"
#include "CUSBDataLinkListener.h"

CUSBDataLinkFactory::CUSBDataLinkFactory(void)
{
}

CUSBDataLinkFactory::~CUSBDataLinkFactory(void)
{
}

IDataTransfer* CUSBDataLinkFactory::create() const
{
	return new CUSBDataLink;
}

IListenSocket* CUSBDataLinkFactory::createListen() const
{
	return new CUSBDataLinkListener;
}


