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

#pragma once

#include "base/String.h"

//! Generic address type
/*!
This class represents transport address.
*/
class BaseAddress {

public:

	enum AddressType { Network, USB, Firewire };

	virtual ~BaseAddress();

	//! Check address validity
	/*!
	Returns true if this is not the invalid address.
	*/
	virtual bool			isValid() const { return false; }

	virtual AddressType 	getAddressType() const = 0;

	virtual String			getName() const = 0;

	virtual BaseAddress* 	clone() const = 0;

	virtual bool			resolve() = 0;
};
