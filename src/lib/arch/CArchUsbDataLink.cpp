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

//
// DEVICE
//

USBDeviceHandle CArchUsbDataLink::usbOpenDevice(int vid, int pid, int ifid)
{
	USBDeviceHandle dev = libusb_open_device_with_vid_pid(m_usbContext, vid, pid);
	if (!dev)
	{
		throw XArchNetwork("libusb_open_device_with_vid_pid failed");
	}

	int rc = libusb_claim_interface(dev, ifid);
	if (rc != 0)
	{
		libusb_close(dev);
		throw XArchNetwork("libusb_claim_interface failed");
	}

	return dev;
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
