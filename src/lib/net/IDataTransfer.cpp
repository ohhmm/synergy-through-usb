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

#include "IDataTransfer.h"
#include "CEventQueue.h"

//
// IDataTransfer
//

CEvent::Type			IDataTransfer::s_connectedEvent = CEvent::kUnknown;
CEvent::Type			IDataTransfer::s_failedEvent    = CEvent::kUnknown;

CEvent::Type
IDataTransfer::getConnectedEvent()
{
	return EVENTQUEUE->registerTypeOnce(s_connectedEvent,
							"IDataTransfer::connected");
}

CEvent::Type
IDataTransfer::getConnectionFailedEvent()
{
	return EVENTQUEUE->registerTypeOnce(s_failedEvent,
							"IDataTransfer::failed");
}

void
IDataTransfer::close()
{
	// this is here to work around a VC++6 bug.  see the header file.
	assert(0 && "bad call");
}

void*
IDataTransfer::getEventTarget() const
{
	// this is here to work around a VC++6 bug.  see the header file.
	assert(0 && "bad call");
	return NULL;
}
