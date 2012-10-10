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

#include "ITransportFactory.h"
#include "CTCPSocketFactory.h"
#include "CUSBDataLinkFactory.h"
#include <assert.h>

ITransportFactory* ITransportFactory::createFactory(CBaseAddress::AddressType addressType) {
	ITransportFactory* transportFactory = NULL;
	switch(addressType)
	{
	case CBaseAddress::Network:
		transportFactory = new CTCPSocketFactory;
		break;
	case CBaseAddress::USB:
		transportFactory = new CUSBDataLinkFactory;
		break;
	default:
		assert(!"Not implemented support of new address type");
		break;
	}
	return transportFactory;
}
