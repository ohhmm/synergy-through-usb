#include "CUSBAddress.h"
#include "arch/Arch.h"
#include "CUSBDataLink.h"
#include <cstdlib>
#include <sstream>
#include <iomanip>

const char * UsbAddressFormat = "USB\\VID_%#4x&PID%#4x,%#2x,%#2x,%#2x,%#2x";
const char * UsbFirstID = "USB\\VID_";
const char * UsbSecondID = "&PID_";
const char UsbParametersDivider = ',';
const int UsbParameterCount = 6;
const int UsbMandatoryParametersCount = 2;

struct UsbDeviceType {
	UInt16 vendorId;
	UInt16 productId;

	static const int MaxStringRepresentationLength = 9; // vid:pid

	UsbDeviceType(int vid, int pid) :
			vendorId(vid), productId(pid) {
	}

	UsbDeviceType(const String& str)
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

	String toString() const {
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

String CUSBAddress::getConnectedCompatibleDeviceNames() {
	std::stringstream ss;

	USBDeviceEnumerator* devices;
	for(size_t count = ARCH->usbGetDeviceList(&devices); count--; ) {

		USBDeviceInfo info;
		ARCH->usbGetDeviceInfo(devices[count], info);

		UsbDeviceType deviceTypeConnected(info.idVendor, info.idProduct);
		if(deviceTypeConnected.isCompatible())
			ss << deviceTypeConnected.toString() << std::endl;
	}

	ARCH->usbFreeDeviceList(devices);

	return ss.str();
}

bool CUSBAddress::equal(BaseAddress* obj) const {
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

CUSBAddress::CUSBAddress(const String& devicePath)
	: fullPathSpecified(false)
{
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

String CUSBAddress::getUSBHostname() const {
	String res;
	res = synergy::string::sprintf(UsbAddressFormat, vendorId, productId, inputEndpoint, outputEndpoint,
			busNumber, deviceAddress);
	return res;
}

bool CUSBAddress::setUSBHostName(const String& path) {

	this->path = path;

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
	String::size_type i=0;
	int j;
	for (j = 0; j < sizeof(value) / sizeof(value[0]); j++)
		value[j] = 0;
	value[4] = 0xff;
	value[5] = 0xff;
	index = 0;
	if ((i = path.find(UsbFirstID, i)) != String::npos) {
		i += strlen(UsbFirstID);
		value[index++] = strtol(&path[i], &end, 16);
		if ((i = path.find(UsbSecondID, i)) != String::npos) {
			i += strlen(UsbSecondID);
			value[index++] = strtol(&path[i], &end, 16);
			i += static_cast<int>(end - &path[i]);
		}
		if (i < path.size()) {
			while ((i = path.find(UsbParametersDivider, i)) != String::npos) {
				i++;
				if ( index >= sizeof(value) / sizeof(value[0]) || i >= path.length() )
					break;
				value[index++] = strtol(&path[i], &end, 16);
			}
		}
	}
	if (index >= UsbMandatoryParametersCount) {
		res = true;
	}
	CUSBAddress temp(static_cast<UInt16>(value[0]),
			static_cast<UInt16>(value[1]), static_cast<UInt8>(value[2]),
			static_cast<UInt8>(value[3]), static_cast<UInt8>(value[4]),
			static_cast<UInt8>(value[5]));
	res = res && temp.isValid();

	if (res) {
		*this = temp;
		fullPathSpecified = true;
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

