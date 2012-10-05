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

#ifndef CTCPSOCKETFACTORY_H
#define CTCPSOCKETFACTORY_H

#include "ITransportFactory.h"

//! Socket factory for TCP sockets
class CTCPSocketFactory : public ITransportFactory {
public:
	CTCPSocketFactory();
	virtual ~CTCPSocketFactory();

	// ITransportFactory overrides
	virtual IDataTransfer*	create() const;
	virtual IListenSocket*	createListen() const;
};

#endif
