#include "XArch.h"
#include "CArchUsbDataLink.h"
#include <libusb.h>

CArchUsbDataLink::CArchUsbDataLink() :
	m_usbContext(NULL)
{
}

CArchUsbDataLink::~CArchUsbDataLink()
{
	usbShut();
}

//
// CONTEXT
//

void CArchUsbDataLink::init()
{
	usbInit();
}

void CArchUsbDataLink::usbInit()
{
	usbShut();

	int rc = libusb_init(&m_usbContext);
	if (rc != 0)
	{
		m_usbContext = NULL;
		throw XArchNetwork("libusb_init failed");
	}
}

void CArchUsbDataLink::usbShut()
{
	if (m_usbContext)
	{
		libusb_exit(m_usbContext);
		m_usbContext = NULL;
	}
}

USBContextHandle CArchUsbDataLink::usbGetContext()
{
	return m_usbContext;
}

void CArchUsbDataLink::usbGetDeviceList(USBDeviceEnumerator **list)
{
	if (libusb_get_device_list(m_usbContext, list) < 0)
	{
		throw XArchNetwork("libusb_get_device_list failed");
	}
}

void CArchUsbDataLink::usbFreeDeviceList(USBDeviceEnumerator *list)
{
	libusb_free_device_list(list, 1);
}

void CArchUsbDataLink::usbGetDeviceInfo(USBDeviceEnumerator devEnum, struct USBDeviceInfo &info)
{
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(devEnum, &desc);
	if (r < 0) {
		throw XArchNetwork("libusb_get_device_descriptor failed");
	}

	info.idVendor = desc.idVendor;
	info.idProduct = desc.idProduct;
	info.busNumber = libusb_get_bus_number(devEnum);
	info.devAddress = libusb_get_device_address(devEnum); 
}


//
// DEVICE
//

USBDeviceHandle CArchUsbDataLink::usbOpenDevice(USBDeviceEnumerator devEnum, int ifid)
{
	USBDeviceHandle handle = NULL;	
	int r;

	r = libusb_open(devEnum, &handle);
	if (r < 0)
	{
		throw XArchNetwork("libusb_open failed");
	}

	int rc = libusb_claim_interface(handle, ifid);
	if (rc != 0)
	{
		libusb_close(handle);
		throw XArchNetwork("libusb_claim_interface failed");
	}

	return handle; 
}

USBDeviceHandle CArchUsbDataLink::usbOpenDevice(struct USBDeviceInfo &devInfo, int ifid)
{
	USBDeviceEnumerator *devs;
	USBDeviceEnumerator iter;
	USBDeviceEnumerator found = NULL;
	USBDeviceHandle handle = NULL;	

	size_t i = 0;

	usbGetDeviceList(&devs);

	try 
	{
		while ((iter = devs[i++]) != NULL) 
		{
			USBDeviceInfo info;
			usbGetDeviceInfo(iter, info);

			if (info.idVendor == devInfo.idVendor &&
				info.idProduct == devInfo.idProduct &&
				info.busNumber == devInfo.busNumber &&
				info.devAddress == devInfo.devAddress)
			{
				found = iter;
				break;
			}
		}

		if (found) {
			handle = usbOpenDevice(found, ifid);
		}

	} 
	catch (...) 
	{
		usbFreeDeviceList(devs);
		throw;
	}

	usbFreeDeviceList(devs);

	return handle; 
}

void CArchUsbDataLink::usbCloseDevice(USBDeviceHandle dev, int ifid)
{
	if (dev)
	{
		libusb_release_interface(dev, ifid);
		libusb_close(dev);
	}
}

int CArchUsbDataLink::usbBulkTransfer(USBDeviceHandle dev, bool write, unsigned char port, unsigned char* buf, unsigned int len, unsigned int timeout)
{
	unsigned char iomode = write ? LIBUSB_ENDPOINT_OUT : LIBUSB_ENDPOINT_IN;

	int transferred = 0;
	int rc = libusb_bulk_transfer(dev, port | iomode, buf, len, &transferred, timeout);
	if (rc != 0) // libusb error
	{
		if (rc == LIBUSB_ERROR_TIMEOUT)
		{
			return 0; // not error, just zero bytes transferred
		}

		throw XArchUsbTransferFailure("libusb_bulk_transfer failed");
	}

	return transferred;
}

int CArchUsbDataLink::usbTryBulkTransfer(USBDeviceHandle dev, bool write, unsigned char port, unsigned char* buf, unsigned int len, unsigned int timeout)
{
	unsigned char iomode = write ? LIBUSB_ENDPOINT_OUT : LIBUSB_ENDPOINT_IN;

	int transferred = 0;
	int rc = libusb_bulk_transfer(dev, port | iomode, buf, len, &transferred, timeout);
	if (rc != 0)
	{
		transferred = 0;
	}

	return transferred;
}
