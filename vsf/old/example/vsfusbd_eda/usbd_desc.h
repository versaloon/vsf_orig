/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __USBD_DESC_H_INCLUDED__
#define __USBD_DESC_H_INCLUDED__

static const uint8_t USB_DeviceDescriptor[] =
{
	0x12,	// bLength = 18
	USB_DT_DEVICE,	// USB_DESC_TYPE_DEVICE
	0x00,
	0x02,	// bcdUSB
	0xEF,	// device class: IAD
	0x02,	// device sub class
	0x01,	// device protocol
	0x40,	// max packet size
	0x83,
	0x04,	// vendor
	0x3A,
	0xA0,	// product
	0x00,
	0x02,	// bcdDevice
	1,		// manu facturer
	2,		// product
	3,		// serial number
	0x01	// number of configuration
};

static const uint8_t USB_ConfigDescriptor[] =
{
	// Configuation Descriptor
	0x09,	// bLength: Configuation Descriptor size
	USB_DT_CONFIG,
			// bDescriptorType: Configuration
	172,	// wTotalLength:no of returned bytes*
	0x00,
	0x05,	// bNumInterfaces: 5 interface
	0x01,	// bConfigurationValue: Configuration value
	0x00,	// iConfiguration: Index of string descriptor describing the configuration
	0x80,	// bmAttributes: bus powered
	0x64,	// MaxPower 200 mA

	// IAD for RNDIS
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,
			// bDescriptorType: IAD
	0,		// bFirstInterface
	2,		// bInterfaceCount
	0x02,	// bFunctionClass
	0x06,	// bFunctionSubClass
	0x00,	// bFunctionProtocol
	0x04,	// iFunction

	// Interface Descriptor for CDC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0,		// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x01,	// bNumEndpoints: One endpoints used
	0x02,	// bInterfaceClass: Communication Interface Class
	0x02,	// bInterfaceSubClass: Abstract Control Model
	0xFF,	// bInterfaceProtocol: Vendor Specific
	0x04,	// iInterface:

	// Header Functional Descriptor
	0x05,	// bLength: Endpoint Descriptor size
	0x24,	// bDescriptorType: CS_INTERFACE
	0x00,	// bDescriptorSubtype: Header Func Desc
	0x10,	// bcdCDC: spec release number
	0x01,

	// Call Managment Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x01,	// bDescriptorSubtype: Call Management Func Desc
	0x00,	// bmCapabilities: D0+D1
	0x01,	// bDataInterface: 1

	// ACM Functional Descriptor
	0x04,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x02,	// bDescriptorSubtype: Abstract Control Management desc
	0x00,	// bmCapabilities

	// Union Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x06,	// bDescriptorSubtype: Union func desc
	0,		// bMasterInterface: Communication class interface
	1,		// bSlaveInterface0: Data Class Interface

	// Endpoint 1 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x81,	// bEndpointAddress: (IN1)
	0x03,	// bmAttributes: Interrupt
	8,		// wMaxPacketSize:
	0x00,
	0xFF,	// bInterval:

	// Data class interface descriptor
	0x09,	// bLength: Endpoint Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	1,		// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x0A,	// bInterfaceClass: CDC
	0x00,	// bInterfaceSubClass:
	0x00,	// bInterfaceProtocol:
	0x04,	// iInterface:

	// Endpoint 2 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x02,	// bEndpointAddress: (OUT2)
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer

	// Endpoint 2 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x82,	// bEndpointAddress: (IN2)
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval

	// IAD for CDC
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,
			// bDescriptorType: IAD
	2,		// bFirstInterface
	2,		// bInterfaceCount
	0x02,	// bFunctionClass
	0x02,	// bFunctionSubClass
	0x01,	// bFunctionProtocol
	0x05,	// iFunction

	// Interface Descriptor for CDC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	2,		// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x01,	// bNumEndpoints: One endpoints used
	0x02,	// bInterfaceClass: Communication Interface Class
	0x02,	// bInterfaceSubClass: Abstract Control Model
	0x01,	// bInterfaceProtocol: Common AT commands
	0x05,	// iInterface:

	// Header Functional Descriptor
	0x05,	// bLength: Endpoint Descriptor size
	0x24,	// bDescriptorType: CS_INTERFACE
	0x00,	// bDescriptorSubtype: Header Func Desc
	0x10,	// bcdCDC: spec release number
	0x01,

	// Call Managment Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x01,	// bDescriptorSubtype: Call Management Func Desc
	0x00,	// bmCapabilities: D0+D1
	0x01,	// bDataInterface: 1

	// ACM Functional Descriptor
	0x04,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x02,	// bDescriptorSubtype: Abstract Control Management desc
	0x02,	// bmCapabilities

	// Union Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x06,	// bDescriptorSubtype: Union func desc
	2,		// bMasterInterface: Communication class interface
	3,		// bSlaveInterface0: Data Class Interface

	// Endpoint 3 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x83,	// bEndpointAddress: (IN3)
	0x03,	// bmAttributes: Interrupt
	8,		// wMaxPacketSize:
	0x00,
	0xFF,	// bInterval:

	// Data class interface descriptor
	0x09,	// bLength: Endpoint Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	3,		// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x0A,	// bInterfaceClass: CDC
	0x00,	// bInterfaceSubClass:
	0x00,	// bInterfaceProtocol:
	0x00,	// iInterface:

	// Endpoint 4 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x04,	// bEndpointAddress: (OUT4)
	0x02,	// bmAttributes: Bulk
	32,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer

	// Endpoint 4 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x84,	// bEndpointAddress: (IN4)
	0x02,	// bmAttributes: Bulk
	32,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval

	// IAD for MSC
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,
			// bDescriptorType: IAD
	4,		// bFirstInterface
	1,		// bInterfaceCount
	0x08,	// bFunctionClass
	0x06,	// bFunctionSubClass
	0x50,	// bFunctionProtocol
	0x06,	// iFunction

	// Interface Descriptor for MSC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	4,		// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x08,	// bInterfaceClass: MSC
	0x06,	// bInterfaceSubClass: SCSI
	0x50,	// bInterfaceProtocol:
	0x06,	// iInterface:

	// Endpoint 5 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x85,	// bEndpointAddress: (IN5)
	0x02,	// bmAttributes: Bulk
	32,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer

	// Endpoint 5 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x05,	// bEndpointAddress: (OUT5)
	0x02,	// bmAttributes: Bulk
	32,		// wMaxPacketSize:
	0x00,
	0x00	// bInterval
};

static const uint8_t USB_StringLangID[] =
{
	4,
	USB_DT_STRING,
	0x09,
	0x04
};

static const uint8_t USB_StringVendor[] =
{
	20,
	USB_DT_STRING,
	'S', 0, 'i', 0, 'm', 0, 'o', 0, 'n', 0, 'Q', 0, 'i', 0, 'a', 0,
	'n', 0
};

static const uint8_t USB_StringSerial[50] =
{
	50,
	USB_DT_STRING,
	'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0,
	'8', 0, '9', 0, 'A', 0, 'B', 0, 'C', 0, 'D', 0, 'E', 0, 'F', 0,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
};

static const uint8_t USB_StringProduct[] =
{
	14,
	USB_DT_STRING,
	'V', 0, 'S', 0, 'F', 0, 'U', 0, 'S', 0, 'B', 0
};

static const uint8_t RNDIS_StringFunc[] =
{
	18,
	USB_DT_STRING,
	'V', 0, 'S', 0, 'F', 0, 'R', 0, 'N', 0, 'D', 0, 'I', 0, 'S', 0
};

static const uint8_t CDC_StringFunc[] =
{
	14,
	USB_DT_STRING,
	'V', 0, 'S', 0, 'F', 0, 'C', 0, 'D', 0, 'C', 0
};

static const uint8_t MSC_StringFunc[] =
{
	14,
	USB_DT_STRING,
	'V', 0, 'S', 0, 'F', 0, 'M', 0, 'S', 0, 'C', 0
};

#endif		// __USBD_DESC_H_INCLUDED__
