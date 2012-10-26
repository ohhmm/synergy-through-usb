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

#ifndef CBASEADDRESS_H
#define CBASEADDRESS_H

#include "CString.h"

//! Generic address type
/*!
This class represents transport address.
*/
class CBaseAddress {

public:

	enum AddressType { Network, USB, Firewire };

	virtual ~CBaseAddress();

	//! Check address validity
	/*!
	Returns true if this is not the invalid address.
	*/
	virtual bool			isValid() const { return false; }

	virtual AddressType 	getAddressType() const = 0;

	virtual CString			getName() const = 0;

	virtual CBaseAddress* 	clone() const = 0;

	virtual bool			resolve() = 0;

	virtual bool 			equal(CBaseAddress*) const;
};

#endif
