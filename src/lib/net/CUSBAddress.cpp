#include "CUSBAddress.h"
#include <cstdlib>

const char * UsbAddressFormat = "USB\\VID_%#4x&PID%#4x,%#2x,%#2x,%#2x,%#2x";
const char * UsbFirstID = "USB\\VID_";
const char * UsbSecondID = "&PID_";
const char UsbParametersDivider = ',';
const int UsbParameterCount = 6;
const int UsbMandatoryParametersCount = 4;

CUSBAddress::CUSBAddress(void)
{
	nVID=0x0402;
	nPID=0x5632;
	nBulkIN=0x81;
	nBulkOut=0x2;
	nBus=0;
	nDeviceOnBus=0;
}

CUSBAddress::CUSBAddress(const UInt16 nnVID, const UInt16 nnPID, const UInt8 nnBulkIN, 
		const UInt8 nnBulkOUT, const UInt8 nnBus, const UInt8 nnDeviceOnBus)
{
	nVID=nnVID;
	nPID=nnPID;
	nBulkIN=nnBulkIN;
	nBulkOut=nnBulkOUT;
	nBus=nnBus;
	nDeviceOnBus=nnDeviceOnBus;
}

CUSBAddress::CUSBAddress(const String& devicePath)
{
	if( !setUSBHostName(devicePath) )
	{
		nVID=0;
		nPID=0;
		nBulkIN=0;
		nBulkOut=0;
		nBus=0;
		nDeviceOnBus=0;
	}
}

CUSBAddress::CUSBAddress(const CUSBAddress& deviceAddress)
{
	*this=operator=(deviceAddress);
}

CUSBAddress& CUSBAddress::operator=(const CUSBAddress& address)
{
	nVID=address.nVID;
	nPID=address.nPID;
	nBulkIN=address.nBulkIN;
	nBulkOut=address.nBulkOut;
	nBus=address.nBus;
	nDeviceOnBus=address.nDeviceOnBus;
	return *this;
}

bool CUSBAddress::operator==(const CUSBAddress& address) const
{
	bool res;
	res = (nVID == address.nVID);
	res = res && (nPID == address.nPID);
	res = res && (nBulkIN == address.nBulkIN);
	res = res && (nBulkOut == address.nBulkOut);
	res = res && (nBus == address.nBus);
	res = res && (nDeviceOnBus == address.nDeviceOnBus);
	return res;
}

bool CUSBAddress::isValid() const
{
	bool res;
	res = (nVID != 0);
	res = res && (nPID != 0);
	res = res && (nBulkIN != 0);
	res = res && (nBulkOut != 0);
	return res;
}

bool CUSBAddress::operator!=(const CUSBAddress& address) const
{
	return !operator==(address);
}

CUSBAddress::~CUSBAddress(void)
{
}

String CUSBAddress::getUSBHostname() const
{
	return synergy::string::sprintf(UsbAddressFormat, nVID, nPID, nBulkIN, nBulkOut, nBus, nDeviceOnBus);
}

bool CUSBAddress::setUSBHostName(const String& name)
{
	long value[UsbParameterCount];
	char * end;
	int index;
	bool res = false;
	String::size_type i=0;
	int j;
	for( j=0; j<sizeof(value)/sizeof(value[0]); j++ )
		value[j] = 0;
	index = 0;
	if( ( i = name.find(UsbFirstID, i) ) != String::npos )
	{
		i += strlen(UsbFirstID);
		value[index++] = strtol(&name[i], &end, 16);
		if( ( i = name.find(UsbSecondID, i) ) != String::npos )
		{
			i += strlen(UsbSecondID);
			value[index++] = strtol(&name[i], &end, 16);
			i += static_cast<int>(end - &name[i]);
		}
		if( i < name.size() )
		{
			while( ( i = name.find(UsbParametersDivider, i) ) != String::npos )
			{
				i++;
				if( index >= sizeof(value)/sizeof(value[0]) && i >= name.size() )
					break;
				value[index++] = strtol(&name[i], &end, 16);
			}
		}
	}
	if( index >= UsbMandatoryParametersCount )
	{
		res = true;
	}
	CUSBAddress temp(	static_cast<UInt16>(value[0]), static_cast<UInt16>(value[1]),
						static_cast<UInt8>(value[2]), static_cast<UInt8>(value[3]), 
						static_cast<UInt8>(value[4]), static_cast<UInt8>(value[5]));
	res = res && temp.isValid();
	if( res )
	{
		*this = temp;
	}
	return res;
}

CUSBAddress::AddressType
CUSBAddress::getAddressType() const
{
	return BaseAddress::USB;
}

String
CUSBAddress::getName() const
{
	return getUSBHostname();
}

CUSBAddress*
CUSBAddress::clone() const
{
	return new CUSBAddress(*this);
}

UInt16 CUSBAddress::GetVID() const
{
	return nVID;
}

UInt16 CUSBAddress::GetPID() const
{
	return nPID;
}

UInt8 CUSBAddress::GetIDBulkIN() const
{
	return nBulkIN;
}

UInt8 CUSBAddress::GetIDBulkOut() const
{
	return nBulkOut;
}

UInt8 CUSBAddress::GetIDBus() const
{
	return nBus;
}

UInt8 CUSBAddress::GetIDDeviceOnBus() const
{
	return nDeviceOnBus;
}

