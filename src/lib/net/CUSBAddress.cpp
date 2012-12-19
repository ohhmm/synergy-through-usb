#include "CUSBAddress.h"
#include "CStringUtil.h"
#include "CArch.h"
#include "CUSBDataLink.h"
#include <cstdlib>
#include <sstream>
#include <iomanip>

const char * UsbAddressFormat = "USB\\VID_%04x&PID_%04x,%02x,%02x";
const char * UsbFirstID = "USB\\VID_";
const char * UsbSecondID = "&PID_";
const char UsbParametersDivider = ',';
const int UsbParameterCount = 4;
const int UsbMandatoryParametersCount = 2;

struct UsbDeviceType {
	UInt16 vendorId;
	UInt16 productId;

	static const int MaxStringRepresentationLength = 9; // vid:pid

	UsbDeviceType(int vid, int pid) :
			vendorId(vid), productId(pid) {
	}

	UsbDeviceType(const CString& str)
	{
		std::stringstream ss(str);
		unsigned uint = 0;
		ss >> std::hex >> uint;
		vendorId = uint;

		char delimeter;
		ss >> delimeter;

		ss >> uint;
		productId = uint;
	}

	CString toString() const {
		std::stringstream ss;
		ss << std::hex << vendorId << ':' << productId;
		return ss.str();
	}

	bool operator==(const UsbDeviceType& usbDeviceType) const {
		return usbDeviceType.vendorId == vendorId && usbDeviceType.productId == productId;
	}

	bool operator==(const USBDeviceInfo& info) const {
		return info.idVendor == vendorId && info.idProduct == productId;
	}

	bool isCompatible() const;
};

const UsbDeviceType CompatibleDevices[] = {
		UsbDeviceType(0x0402, 0x5632),
		UsbDeviceType(0x067b, 0x25a1) };

bool UsbDeviceType::isCompatible() const {
	bool result = false;
	for(int i = sizeof(CompatibleDevices) / sizeof(*CompatibleDevices);
			!result && i--; ){
		result = *this==CompatibleDevices[i];
	}
	return result;
}

bool CUSBAddress::resolve() {
	bool result = false;
	USBDeviceEnumerator* devices;
	size_t count = ARCH->usbGetDeviceList(&devices);
	USBDeviceInfo info;
	for(; !result && count--; ) {
		ARCH->usbGetDeviceInfo(devices[count], info);
		result = info.idVendor == vendorId && info.idProduct==productId && info.validEndpointInfo &&
			(fullPathSpecified ? info.busNumber == busNumber && info.deviceAddress == deviceAddress : true);
		if( result ){
			break;
		}
	}

	if(fullPathSpecified && !result) {
		fullPathSpecified = false;
		ARCH->usbFreeDeviceList(devices);
		return resolve();
	}

	fullPathSpecified = result;
	if( result ){
		busNumber = info.busNumber;
		deviceAddress = info.deviceAddress;
		inputEndpoint = info.inputEndpoint;
		outputEndpoint = info.outputEndpoint;
	}

	ARCH->usbFreeDeviceList(devices);
	return result;
}

CString CUSBAddress::getConnectedCompatibleDeviceNames() {

	USBDeviceEnumerator* devices;
	size_t count = ARCH->usbGetDeviceList(&devices);

	unsigned compatibleDevicesCount = 0;
	for( size_t i = 0; i < count; i++ ) {
		USBDeviceInfo info;
		ARCH->usbGetDeviceInfo(devices[i], info);
		UsbDeviceType deviceTypeConnected(info.idVendor, info.idProduct);
		if(deviceTypeConnected.isCompatible())
			++compatibleDevicesCount;
	}

	bool showShortName = compatibleDevicesCount == 1;
	std::stringstream ss;
	for( size_t i = 0; i < count; i++ ) {
		USBDeviceInfo info;
		ARCH->usbGetDeviceInfo(devices[i], info);
		UsbDeviceType deviceTypeConnected(info.idVendor, info.idProduct);
		if(deviceTypeConnected.isCompatible()) {
			if(showShortName) {
				ss << deviceTypeConnected.toString() << std::endl;
			} else {
				ss	<< CUSBAddress(info.idVendor, info.idProduct, info.inputEndpoint, info.outputEndpoint, info.busNumber, info.deviceAddress).getName().c_str()
					<< std::endl;
			}
		}
	}

	ARCH->usbFreeDeviceList(devices);

	return ss.str();
}

bool CUSBAddress::equal(CBaseAddress* obj) const {
	return getAddressType() == obj->getAddressType() &&
				operator ==(*reinterpret_cast<CUSBAddress*>(obj));
}

void CUSBAddress::assignDefaults() {
	vendorId = 0x0402;
	productId = 0x5632;
	inputEndpoint = 0x81;
	outputEndpoint = 0x2;
	busNumber = 0;
	deviceAddress = 0;
}

CUSBAddress::CUSBAddress(void) : fullPathSpecified(false) {
	assignDefaults();
}

CUSBAddress::CUSBAddress(const UInt16 nnVID, const UInt16 nnPID,
		const UInt8 nnBulkIN, const UInt8 nnBulkOUT, const UInt8 nnBus,
		const UInt8 nnDeviceOnBus) : fullPathSpecified(false) {
	vendorId = nnVID;
	productId = nnPID;
	inputEndpoint = nnBulkIN;
	outputEndpoint = nnBulkOUT;
	busNumber = nnBus;
	deviceAddress = nnDeviceOnBus;
}

CUSBAddress::CUSBAddress(const CString& devicePath) : fullPathSpecified(false) {
	if (!setUSBHostName(devicePath)) {
		assignDefaults();
	}
}

bool CUSBAddress::operator==(const CUSBAddress& address) const {
	bool res;
	res = (vendorId == address.vendorId);
	res = res && (productId == address.productId);
	res = res && (inputEndpoint == address.inputEndpoint);
	res = res && (outputEndpoint == address.outputEndpoint);
	res = res && (busNumber == address.busNumber);
	res = res && (deviceAddress == address.deviceAddress);
	return res;
}

bool CUSBAddress::isValid() const {
	bool res;
	res = (vendorId != 0);
	res = res && (productId != 0);
	//res = res && (nBulkIN != 0);
	//res = res && (nBulkOut != 0);
	return res;
}

bool CUSBAddress::operator!=(const CUSBAddress& address) const {
	return !operator==(address);
}

CUSBAddress::~CUSBAddress(void) {
}

CString CUSBAddress::getUSBHostname() const {
	CString res;
	res = CStringUtil::print(UsbAddressFormat, vendorId, productId, busNumber, deviceAddress);
	return res;
}

bool CUSBAddress::setUSBHostName(const CString& path) {

	this->path = path;
	fullPathSpecified = false;

	if (path.length() <= UsbDeviceType::MaxStringRepresentationLength) {
		UsbDeviceType usbDeviceType(path);
		this->vendorId = usbDeviceType.vendorId;
		this->productId = usbDeviceType.productId;
		return resolve();
	}

	long value[UsbParameterCount];
	char * end;
	int index;
	bool res = false;
	CString::size_type i = 0;
	int j;
	for (j = 0; j < sizeof(value) / sizeof(value[0]); j++)
		value[j] = 0;
	index = 0;
	if ((i = path.find(UsbFirstID, i)) != CString::npos) {
		i += strlen(UsbFirstID);
		value[index++] = strtol(&path[i], &end, 16);
		if ((i = path.find(UsbSecondID, i)) != CString::npos) {
			i += strlen(UsbSecondID);
			value[index++] = strtol(&path[i], &end, 16);
			i += static_cast<int>(end - &path[i]);
		}
		if (i < path.size()) {
			while ((i = path.find(UsbParametersDivider, i)) != CString::npos) {
				i++;
				if ( index >= sizeof(value) / sizeof(value[0]) || i >= path.length() )
					break;
				value[index++] = strtol(&path[i], &end, 16);
			}
		}
	}
	if (index >= UsbMandatoryParametersCount) {
		res = true;
		if( index == UsbParameterCount ) {
			fullPathSpecified = true;
		}
	}

	vendorId = static_cast<UInt16>(value[0]);
	productId = static_cast<UInt16>(value[1]);
	inputEndpoint = 0;
	outputEndpoint = 0;
	busNumber = static_cast<UInt8>(value[2]);
	deviceAddress = static_cast<UInt8>(value[3]);

	res = res && isValid();
	return res;
}

CUSBAddress::AddressType CUSBAddress::getAddressType() const {
	return CBaseAddress::USB;
}

CString CUSBAddress::getName() const {
	return getUSBHostname();
}

CUSBAddress*
CUSBAddress::clone() const {
	return new CUSBAddress(*this);
}

UInt16 CUSBAddress::GetVID() const {
	return vendorId;
}

UInt16 CUSBAddress::GetPID() const {
	return productId;
}

UInt8 CUSBAddress::GetIDBulkIN() const {
	return inputEndpoint;
}

UInt8 CUSBAddress::GetIDBulkOut() const {
	return outputEndpoint;
}

UInt8 CUSBAddress::GetIDBus() const {
	return busNumber;
}

UInt8 CUSBAddress::GetIDDeviceOnBus() const {
	return deviceAddress;
}

