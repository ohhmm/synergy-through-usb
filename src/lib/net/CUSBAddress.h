#pragma once

#include "CString.h"
#include "BasicTypes.h"
#include "CBaseAddress.h"

class CUSBAddress : public CBaseAddress
{
public:
	/*!
	Constructs the invalid address
	*/
	CUSBAddress(void);

	/*!
	Construct device path. Consist of Vendor ID (nnVID), Product ID (nnPID), 
	in endpoint number (nnBulkIN), out endpoint number (nnBulkOUT),
	bus number (nnBus), device number (nnDeviceOnBus)
	nnBus, nnDeviceOnBus could be entered here optionally
	*/
	CUSBAddress(const UInt16 nnVID, const UInt16 nnPID, const UInt8 nnBulkIN, 
		const UInt8 nnBulkOUT, const UInt8 nnBus, const UInt8 nnDeviceOnBus);

	/*!
	Construct device path. Consists of string values of Vendor ID, 
	Product ID, in endpoint number, out endpoint number, bus number, 
	device number (nnDeviceOnBus)
	nnBus, nnDeviceOnBus could be entered here optionally
	*/
	CUSBAddress(const CString& devicepath);

	//!Copy constructor
	CUSBAddress(const CUSBAddress&);

	~CUSBAddress();

	CUSBAddress&			operator=(const CUSBAddress&);

	//@}
	//! @name accessors
	//@{

	//! Check address equality
	/*!
	Returns true if this address is equal to \p address.
	*/
	bool					operator==(const CUSBAddress& address) const;

	//! Check address inequality
	/*!
	Returns true if this address is not equal to \p address.
	*/
	bool					operator!=(const CUSBAddress& address) const;

	//! Check address validity
	/*!
	Returns true if this is not the invalid address.
	*/
	virtual bool			isValid() const;

	virtual AddressType 	getAddressType() const;

	virtual CString			getName() const;

	virtual CUSBAddress* 	clone() const;

	bool 					equal(CBaseAddress*) const;

	virtual bool 			resolve();

	//! Return Vendor ID
	UInt16 GetVID() const;
	//! Return Product ID
	UInt16 GetPID() const;
	//! Return Bilk In ID
	UInt8 GetIDBulkIN() const;
	//! Return Bulk Out ID
	UInt8 GetIDBulkOut() const;
	//! Return Bus ID
	UInt8 GetIDBus() const;
	//! Return Device ID on USB Bus
	UInt8 GetIDDeviceOnBus() const;
	/*! Return string contains hexadecimal Vendor ID, Product ID, in endpoint 
	number, out endpoint number, bus number, device number, divided by spaces
	*/
	CString getUSBHostname() const;
	/*!
	Construct device path. Consists of string values of Vendor ID, 
	Product ID, in endpoint number, out endpoint number, bus number, 
	device number (nnDeviceOnBus)
	nnBus, nnDeviceOnBus could be entered here optionally
	If device path is invalid function will return false
	Path has to look similar to example: USB\VID_0402&PID_5632,81,02
	(you can get it at "device properties"->"property page"->"Details", "Property"->"Hardware Ids in Windows)
	*/
	bool setUSBHostName(const CString& name);

	//@}

	static CString getConnectedCompatibleDeviceNames();

private:

	CString path;
	UInt16 nVID;
	UInt16 nPID;
	UInt8 nBulkIN;
	UInt8 nBulkOut;
	UInt8 nBus;
	UInt8 nDeviceOnBus;

	void assignDefaults();
};


