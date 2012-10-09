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
#include "net/TCPSocketFactory.h"
#include "net/CUSBDataLinkFactory.h"
#include <assert.h>

ITransportFactory * ITransportFactory::createFactory(BaseAddress::AddressType addressType, IEventQueue *events, void *param) {
	ITransportFactory* transportFactory = NULL;
	switch(addressType)
	{
	case BaseAddress::Network:
		transportFactory = new CTCPSocketFactory(events, static_cast<SocketMultiplexer*>(param));
		break;
	case BaseAddress::USB:
		transportFactory = new CUSBDataLinkFactory(events);
		break;
	default:
		assert(!"Not implemented support of new address type");
		break;
    }
	return transportFactory;
}
