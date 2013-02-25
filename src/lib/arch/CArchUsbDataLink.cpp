#include "CArch.h"
#include "XArch.h"
#include "CArchUsbDataLink.h"
#include <libusb.h>

CArchUsbDataLink::CArchUsbDataLink() :
	m_thread(NULL),
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
		usbShut();
		throw XArchNetwork("libusb_init failed");
	}

	m_thread = ARCH->newThread(&CArchUsbDataLink::threadFunc, m_usbContext);
	if (m_thread == NULL) 
	{
		usbShut();
		throw XArchNetwork("cannot create working thread");		
	}
}

void CArchUsbDataLink::usbShut()
{
	if (m_thread)
	{
		ARCH->cancelThread(m_thread);
		ARCH->wait(m_thread, INFINITE);
		ARCH->closeThread(m_thread);
		m_thread = NULL;
	}

	if (m_usbContext)
	{
		libusb_exit(m_usbContext);
		m_usbContext = NULL;
	}
}

void* CArchUsbDataLink::threadFunc(void* ctx)
{
	USBContextHandle usbContext = reinterpret_cast<USBContextHandle>(ctx);
	void* result = NULL;

//	// get this thread's id for logging
//	IArchMultithread::ThreadID id;
//	{
//		CArchThread thread = ARCH->newCurrentThread();
//		id = ARCH->getIDOfThread(thread);
//		ARCH->closeThread(thread);
//	}

	try {
		// go
		//LOG((CLOG_DEBUG1 "thread 0x%08x entry", id));

		for (;;) {
			ARCH->testCancelThread();

			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 100;
			libusb_handle_events_timeout(usbContext, &tv);

			//libusb_handle_events(usbContext);
		}

		//LOG((CLOG_DEBUG1 "thread 0x%08x exit", id));
	}

	catch (const XThreadCancel& a) {
		// client called cancel()
//		LOG((CLOG_DEBUG1 "caught cancel on thread 0x%08x", id));
		throw a;//throw a;
	}
//	catch (XThreadExit&) {
		// client called exit()
//		result = e.m_result;
//		LOG((CLOG_DEBUG1 "caught exit on thread 0x%08x, result %p", id, result));
//	}
//	catch (XBase& e) {
//		LOG((CLOG_ERR "exception on thread 0x%08x: %s", id, e.what()));
//		throw;
//	}
	catch (...) {
		//LOG((CLOG_ERR "exception on thread 0x%08x: <unknown>", id));
		throw;
	}

	// return exit result
	return result;
}

USBContextHandle CArchUsbDataLink::usbGetContext()
{
	return m_usbContext;
}

size_t CArchUsbDataLink::usbGetDeviceList(USBDeviceEnumerator **list)
{
	size_t count = libusb_get_device_list(m_usbContext, list);
	if (count < 0)
	{
		throw XArchNetwork("libusb_get_device_list failed");
	}
	return count;
}

void CArchUsbDataLink::usbFreeDeviceList(USBDeviceEnumerator *list)
{
	libusb_free_device_list(list, 1);
}

void CArchUsbDataLink::usbGetDeviceInfo(USBDeviceEnumerator devEnum, struct USBDeviceInfo &info)
{
	libusb_device_descriptor desc;
	libusb_config_descriptor* config_desc;

	int r = libusb_get_device_descriptor(devEnum, &desc);
	if (r < 0) {
		throw XArchNetwork("libusb_get_device_descriptor failed");
	}
	info.idVendor = desc.idVendor;
	info.idProduct = desc.idProduct;
	info.busNumber = libusb_get_bus_number(devEnum);
	info.deviceAddress = libusb_get_device_address(devEnum);

	r = libusb_get_active_config_descriptor(devEnum, &config_desc);
	if( r>=0 )
	{
		//Investigate device interfaces. Detect 'in' and 'out' bulk endpoints
		//in specific interface and save their numbers.
		info.validEndpointInfo = false;
		if( config_desc->interface != NULL )
		{
			for(int n=0; n < config_desc->interface->num_altsetting; n++ )
			{
				const libusb_interface_descriptor* interface_desc = config_desc->interface->altsetting;
				if( interface_desc->bNumEndpoints>=2 )
				{
					bool bBulkINfound = false;
					bool bBulkOutfound = false;
					for(int i=0; i<interface_desc->bNumEndpoints; i++)
					{
						const libusb_endpoint_descriptor* endpoint_desc = &interface_desc->endpoint[i];
						if( (endpoint_desc->bmAttributes&3) == LIBUSB_TRANSFER_TYPE_BULK )
						{
							if( endpoint_desc->bEndpointAddress&0x80 )
							{
								info.inputEndpoint = endpoint_desc->bEndpointAddress;
								info.inputEndpointMaxPacketSize = endpoint_desc->wMaxPacketSize;
								bBulkINfound = true;
							}
							else
							{
								info.outputEndpoint = endpoint_desc->bEndpointAddress;
								info.outputEndpointMaxPacketSize = endpoint_desc->wMaxPacketSize;
								bBulkOutfound = true;
							}
						}
						if( bBulkINfound && bBulkOutfound )
						{
							info.validEndpointInfo = true;
							info.interfaceNumber = n;
							break;
						}
					}
					if( info.validEndpointInfo )
					{
						break;
					}
				}
			}
		}

		libusb_free_config_descriptor(config_desc);
	}
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
		if(r == LIBUSB_ERROR_ACCESS)
			throw XArchNetwork("Access denied (insufficient permissions)");
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
	USBDeviceEnumerator* devs;
	USBDeviceEnumerator iter;
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
				(info.busNumber == devInfo.busNumber || devInfo.busNumber == (unsigned char)-1) &&
				(info.deviceAddress == devInfo.deviceAddress || devInfo.deviceAddress == (unsigned char)-1))
			{
				handle = usbOpenDevice(iter, ifid);
				break;
			}
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

int CArchUsbDataLink::usbBulkTransfer(USBDeviceHandle dev, bool write, unsigned char port, char* buf, unsigned int len, unsigned int timeout)
{
	unsigned char iomode = write ? LIBUSB_ENDPOINT_OUT : LIBUSB_ENDPOINT_IN;

	int transferred = 0;
	int rc = libusb_bulk_transfer(dev, port | iomode, (unsigned char*)buf, len, &transferred, timeout);
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

int CArchUsbDataLink::usbTryBulkTransfer(USBDeviceHandle dev, bool write, unsigned char port, char* buf, unsigned int len, unsigned int timeout)
{
	unsigned char iomode = write ? LIBUSB_ENDPOINT_OUT : LIBUSB_ENDPOINT_IN;

	int transferred = 0;
	int rc = libusb_bulk_transfer(dev, port | iomode, (unsigned char*)buf, len, &transferred, timeout);
	if (rc != 0)
	{
		transferred = 0;
	}

	return transferred;
}
