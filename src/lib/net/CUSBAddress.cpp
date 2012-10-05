#include "CUSBAddress.h"
#include "CStringUtil.h"
#include <cstdlib>

CUSBAddress::CUSBAddress(void)
{
	nVID=0;
	nPID=0;
	nBulkIN=0;
	nBulkOut=0;
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

CUSBAddress::CUSBAddress(const CString& devicepath)
{
	if( !setUSBHostName(devicepath) )
	{
		nVID=0;
		nPID=0;
		nBulkIN=0;
		nBulkOut=0;
		nBus=0;
		nDeviceOnBus=0;
	}
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
	res = (nVID==address.nVID);
	res = res&&(nPID==address.nPID);
	res = res&&(nBulkIN==address.nBulkIN);
	res = res&&(nBulkOut==address.nBulkOut);
	res = res&&(nBus==address.nBus);
	res = res&&(nDeviceOnBus==address.nDeviceOnBus);
	return res;
}

bool CUSBAddress::isValid() const
{
	bool res;
	res = (nVID!=0);
	res = res&&(nPID!=0);
	res = res&&(nBulkIN!=0);
	res = res&&(nBulkOut!=0);
	return res;
}

bool CUSBAddress::operator!=(const CUSBAddress& address) const
{
	return !operator==(address);
}

CUSBAddress::~CUSBAddress(void)
{
}

CString CUSBAddress::getUSBHostname() const
{
	CString res;
	res=CStringUtil::print("%#4x,%#4x,%#2x,%#2x,%#2x,%#2x", nVID, nPID, nBulkIN, nBulkOut, nBus, nDeviceOnBus);
	return res;
}

bool CUSBAddress::setUSBHostName(const CString& name)
{
	long value[6];
	char * end;
	int index;
	bool res = false;
	CString::size_type i=0;
	index = 0;

	if( i > name.size() )
	{
		value[index++] = strtol(&name[i], &end, 16);
		while( ( i = name.find(',', i+1) ) != CString::npos )
		{
			if( index >= sizeof(value)/sizeof(value[0]) && i+1 >= name.size() )
				break;
			value[index++] = strtol(&name[i+1], &end, 16);
		}
	
	}
	if( index >= 4 )
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
	return CBaseAddress::USB;
}

CString
CUSBAddress::getName() const
{
	return getUSBHostname();
}

CUSBAddress*
CUSBAddress::clone() const
{
	return new CUSBAddress(*this);
}

UInt16 CUSBAddress::GetVID()
{
	return nVID;
}

UInt16 CUSBAddress::GetPID()
{
	return nPID;
}

UInt8 CUSBAddress::GetIDBulkIN()
{
	return nBulkIN;
}

UInt8 CUSBAddress::GetIDBulkOut()
{
	return nBulkOut;
}

UInt8 CUSBAddress::GetIDBus()
{
	return nBus;
}

UInt8 CUSBAddress::GetIDDeviceOnBus()
{
	return nDeviceOnBus;
}

