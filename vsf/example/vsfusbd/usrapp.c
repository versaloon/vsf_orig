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
#include "vsf.h"
#include "usrapp.h"

#include "fakefat32_fs.h"

static const struct app_hwcfg_t app_hwcfg =
{
	.usbd.pullup.port = USB_PULLUP_PORT,
	.usbd.pullup.pin = USB_PULLUP_PIN,
};

struct usrapp_param_t
{
	struct
	{
		uint8_t DeviceDescriptor[18];
		uint8_t ConfigDescriptor[106];
		uint8_t StringLangID[4];
		uint8_t StringVendor[20];
		uint8_t StringSerial[50];
		uint8_t StringProduct[22];
		uint8_t StringFunc_RNDIS[18];
		uint8_t StringFunc_MSC[14];
		struct vsfusbd_desc_filter_t StdDesc[9];
	} usbd;
} static const usrapp_param =
{
	.usbd.DeviceDescriptor =
	{
		USB_DT_DEVICE_SIZE,
		USB_DT_DEVICE,
		0x00, 0x02,	// bcdUSB
		0xEF,		// device class: IAD
		0x02,		// device sub class
		0x01,		// device protocol
		64,			// max packet size
		0xA1, 0xA2,	// vendor
		0x08, 0x07,	// product
		0x00, 0x01,	// bcdDevice
		1,			// manu facturer
		2,			// product
		3,			// serial number
		1,			// number of configuration 
	},
	.usbd.ConfigDescriptor =
	{
		USB_DT_CONFIG_SIZE,
		USB_DT_CONFIG,
		106, 0,		// wTotalLength
		0x03,		// bNumInterfaces: 3 interfaces
		0x01,		// bConfigurationValue: Configuration value
		0x00,		// iConfiguration: Index of string descriptor describing the configuration
		0x80,		// bmAttributes: bus powered
		0x64,		// MaxPower

		// IDA for RNDIS
		USB_DT_INTERFACE_ASSOCIATION_SIZE,
		USB_DT_INTERFACE_ASSOCIATION,
		0,			// bFirstInterface
		2,			// bInterfaceCount
		0x02,		// bFunctionClass
		0x06,		// bFunctionSubClass
		0x00,		// bFunctionProtocol
		0x04,		// iFunction

		USB_DT_INTERFACE_SIZE,
		USB_DT_INTERFACE,
		0x00,		// bInterfaceNumber: Number of Interface
		0x00,		// bAlternateSetting: Alternate setting
		0x01,		// bNumEndpoints
		0x02,		// bInterfaceClass
		0x02,		// bInterfaceSubClass
		0xFF,		// nInterfaceProtocol
		0x04,		// iInterface:

		// Header Functional Descriptor
		0x05,		// bLength: Endpoint Descriptor size
		0x24,		// bDescriptorType: CS_INTERFACE
		0x00,		// bDescriptorSubtype: Header Func Desc
		0x10,		// bcdCDC: spec release number
		0x01,

		// Call Managment Functional Descriptor
		0x05,		// bFunctionLength
		0x24,		// bDescriptorType: CS_INTERFACE
		0x01,		// bDescriptorSubtype: Call Management Func Desc
		0x00,		// bmCapabilities: D0+D1
		0x01,		// bDataInterface: 1

		// ACM Functional Descriptor
		0x04,		// bFunctionLength
		0x24,		// bDescriptorType: CS_INTERFACE
		0x02,		// bDescriptorSubtype: Abstract Control Management desc
		0x00,		// bmCapabilities

		// Union Functional Descriptor
		0x05,		// bFunctionLength
		0x24,		// bDescriptorType: CS_INTERFACE
		0x06,		// bDescriptorSubtype: Union func desc
		0,			// bMasterInterface: Communication class interface
		1,			// bSlaveInterface0: Data Class Interface

		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		0x81,		// bEndpointAddress: (IN1)
		0x03,		// bmAttributes: Interrupt
		8,			// wMaxPacketSize:
		0x00,
		0xFF,		// bInterval:

		USB_DT_INTERFACE_SIZE,
		USB_DT_INTERFACE,
		0x01,		// bInterfaceNumber: Number of Interface
		0x00,		// bAlternateSetting: Alternate setting
		0x02,		// bNumEndpoints
		0x0A,		// bInterfaceClass
		0x00,		// bInterfaceSubClass
		0x00,		// nInterfaceProtocol
		0x04,		// iInterface:

		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		0x82,		// bEndpointAddress: (IN2)
		0x02,		// bmAttributes: Interrupt
		64,			// wMaxPacketSize:
		0x00,
		0x00,		// bInterval:

		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		0x02,		// bEndpointAddress: (OUT2)
		0x02,		// bmAttributes: Interrupt
		64,			// wMaxPacketSize:
		0x00,
		0x00,		// bInterval:

		// IDA for MSC
		USB_DT_INTERFACE_ASSOCIATION_SIZE,
		USB_DT_INTERFACE_ASSOCIATION,
		2,			// bFirstInterface
		1,			// bInterfaceCount
		0x08,		// bFunctionClass
		0x06,		// bFunctionSubClass
		0x50,		// bFunctionProtocol
		0x05,		// iFunction

		USB_DT_INTERFACE_SIZE,
		USB_DT_INTERFACE,
		0x02,		// bInterfaceNumber: Number of Interface
		0x00,		// bAlternateSetting: Alternate setting
		0x02,		// bNumEndpoints
		0x08,		// bInterfaceClass: MSC
		0x06,		// bInterfaceSubClass: SCSI
		0x50,		// nInterfaceProtocol:
		0x05,		// iInterface:

		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		0x83,		// bEndpointAddress: (IN3)
		0x02,		// bmAttributes: Interrupt
		32,			// wMaxPacketSize:
		0x00,
		0x00,		// bInterval:

		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		0x03,		// bEndpointAddress: (OUT3)
		0x02,		// bmAttributes: Interrupt
		32,			// wMaxPacketSize:
		0x00,
		0x00,		// bInterval:
	},
	.usbd.StringLangID =
	{
		4,
		USB_DT_STRING,
		0x09,
		0x04,
	},
	.usbd.StringVendor =
	{
		20,
		USB_DT_STRING,
		'S', 0, 'i', 0, 'm', 0, 'o', 0, 'n', 0, 'Q', 0, 'i', 0, 'a', 0,
		'n', 0,
	},
	.usbd.StringSerial =
	{
		50,
		USB_DT_STRING,
		'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0,
		'8', 0, '9', 0, 'A', 0, 'B', 0, 'C', 0, 'D', 0, 'E', 0, 'F', 0,
		'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
	},
	.usbd.StringProduct =
	{
		14,
		USB_DT_STRING,
		'V', 0, 'S', 0, 'F', 0, 'U', 0, 'S', 0, 'B', 0,
	},
	.usbd.StringFunc_RNDIS =
	{
		18,
		USB_DT_STRING,
		'V', 0, 'S', 0, 'F', 0, 'R', 0, 'N', 0, 'D', 0, 'I', 0, 'S', 0
	},
	.usbd.StringFunc_MSC =
	{
		14,
		USB_DT_STRING,
		'V', 0, 'S', 0, 'F', 0, 'M', 0, 'S', 0, 'C', 0
	},
	.usbd.StdDesc =
	{
		VSFUSBD_DESC_DEVICE(0, usrapp_param.usbd.DeviceDescriptor, sizeof(usrapp_param.usbd.DeviceDescriptor)),
		VSFUSBD_DESC_CONFIG(0, 0, usrapp_param.usbd.ConfigDescriptor, sizeof(usrapp_param.usbd.ConfigDescriptor)),
		VSFUSBD_DESC_STRING(0, 0, usrapp_param.usbd.StringLangID, sizeof(usrapp_param.usbd.StringLangID)),
		VSFUSBD_DESC_STRING(0x0409, 1, usrapp_param.usbd.StringVendor, sizeof(usrapp_param.usbd.StringVendor)),
		VSFUSBD_DESC_STRING(0x0409, 2, usrapp_param.usbd.StringProduct, sizeof(usrapp_param.usbd.StringProduct)),
		VSFUSBD_DESC_STRING(0x0409, 3, usrapp_param.usbd.StringSerial, sizeof(usrapp_param.usbd.StringSerial)),
		VSFUSBD_DESC_STRING(0x0409, 4, usrapp_param.usbd.StringFunc_RNDIS, sizeof(usrapp_param.usbd.StringFunc_RNDIS)),
		VSFUSBD_DESC_STRING(0x0409, 5, usrapp_param.usbd.StringFunc_MSC, sizeof(usrapp_param.usbd.StringFunc_MSC)),
		VSFUSBD_DESC_NULL,
	},
};

static void usrapp_rndis_on_connect(void *param);
struct usrapp_t usrapp =
{
	.hwcfg									= &app_hwcfg,

	.mal.fakefat32.sector_size				= 512,
	.mal.fakefat32.sector_number			= 0x00001000,
	.mal.fakefat32.sectors_per_cluster		= 8,
	.mal.fakefat32.volume_id				= 0x0CA93E47,
	.mal.fakefat32.disk_id					= 0x12345678,
	.mal.fakefat32.root[0].memfile.file.name= "ROOT",
	.mal.fakefat32.root[0].memfile.d.child	= (struct vsfile_memfile_t *)fakefat32_root_dir,

	.mal.mal.drv							= &fakefat32_mal_drv,
	.mal.mal.param							= &usrapp.mal.fakefat32,
	.mal.scsistream.mbuf.count				= dimof(usrapp.mal.buffer),
	.mal.scsistream.mbuf.size				= sizeof(usrapp.mal.buffer[0]),
	.mal.scsistream.mbuf.buffer_list		= usrapp.mal.pbuffer,

	.mal.mal2scsi.malstream.mal				= &usrapp.mal.mal,
	.mal.mal2scsi.cparam.block_size			= 512,
	.mal.mal2scsi.cparam.removable			= false,
	.mal.mal2scsi.cparam.vendor				= "Simon   ",
	.mal.mal2scsi.cparam.product			= "VSFDriver       ",
	.mal.mal2scsi.cparam.revision			= "1.00",
	.mal.mal2scsi.cparam.type				= SCSI_PDT_DIRECT_ACCESS_BLOCK,

	.mal.lun[0].op							= (struct vsfscsi_lun_op_t *)&vsf_mal2scsi_op,
	// lun->stream MUST be scsistream for mal2scsi
	.mal.lun[0].stream						= (struct vsf_stream_t *)&usrapp.mal.scsistream,
	.mal.lun[0].param						= &usrapp.mal.mal2scsi,
	.mal.scsi_dev.max_lun					= 0,
	.mal.scsi_dev.lun						= usrapp.mal.lun,

	.usbd.rndis.param.CDCACM.CDC.ep_notify	= 1,
	.usbd.rndis.param.CDCACM.CDC.ep_out		= 2,
	.usbd.rndis.param.CDCACM.CDC.ep_in		= 2,
	.usbd.rndis.param.mac.size				= 6,
	.usbd.rndis.param.mac.addr.s_addr64		= 0x0605040302E0,
	.usbd.rndis.param.cb.param				= &usrapp,
	.usbd.rndis.param.cb.on_connect			= usrapp_rndis_on_connect,
	.usbd.msc.param.ep_in					= 3,
	.usbd.msc.param.ep_out					= 3,
	.usbd.msc.param.scsi_dev				= &usrapp.mal.scsi_dev,
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_RNDISControl_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.rndis.param,
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_RNDISData_class,
	.usbd.ifaces[1].protocol_param			= &usrapp.usbd.rndis.param,
	.usbd.ifaces[2].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_MSCBOT_class,
	.usbd.ifaces[2].protocol_param			= &usrapp.usbd.msc.param,
	.usbd.config[0].num_of_ifaces			= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface					= usrapp.usbd.ifaces,
	.usbd.device.num_of_configuration		= dimof(usrapp.usbd.config),
	.usbd.device.config						= usrapp.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)usrapp_param.usbd.StdDesc,
	.usbd.device.device_class_iface			= 0,
	.usbd.device.drv						= (struct vsfhal_usbd_t *)&vsfhal_usbd,
	.usbd.device.int_priority				= 0,

	.usbd.rndis.param.netif.macaddr.size			= 6,
	.usbd.rndis.param.netif.macaddr.addr.s_addr64	= 0x0E0D0C0B0AE0,
	.usbd.rndis.param.netif.ipaddr.size				= 4,
	.usbd.rndis.param.netif.ipaddr.addr.s_addr		= 0x01202020,
	.usbd.rndis.param.netif.netmask.size			= 4,
	.usbd.rndis.param.netif.netmask.addr.s_addr		= 0x00FFFFFF,
	.usbd.rndis.param.netif.gateway.size			= 4,
	.usbd.rndis.param.netif.gateway.addr.s_addr		= 0x01202020,

	.tcpip.telnetd.telnetd.port						= 23,
	.tcpip.telnetd.telnetd.session_num				= dimof(usrapp.tcpip.telnetd.sessions),
	.tcpip.telnetd.sessions[0].stream_tx			= (struct vsf_stream_t *)&usrapp.tcpip.telnetd.stream_tx,
	.tcpip.telnetd.sessions[0].stream_rx			= (struct vsf_stream_t *)&usrapp.tcpip.telnetd.stream_rx,
	.tcpip.telnetd.stream_tx.stream.op				= &fifostream_op,
	.tcpip.telnetd.stream_tx.mem.buffer.buffer		= (uint8_t *)&usrapp.tcpip.telnetd.txbuff,
	.tcpip.telnetd.stream_tx.mem.buffer.size		= sizeof(usrapp.tcpip.telnetd.txbuff),
	.tcpip.telnetd.stream_rx.stream.op				= &fifostream_op,
	.tcpip.telnetd.stream_rx.mem.buffer.buffer		= (uint8_t *)&usrapp.tcpip.telnetd.rxbuff,
	.tcpip.telnetd.stream_rx.mem.buffer.size		= sizeof(usrapp.tcpip.telnetd.rxbuff),

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	.shell.echo								= false,
	.shell.stream_tx						= (struct vsf_stream_t *)&usrapp.tcpip.telnetd.stream_tx,
	.shell.stream_rx						= (struct vsf_stream_t *)&usrapp.tcpip.telnetd.stream_rx,
#endif
};

// vsfip buffer manager
static struct vsfip_buffer_t* app_vsfip_get_buffer(uint32_t size)
{
	return VSFPOOL_ALLOC(&usrapp.tcpip.buffer_pool, struct vsfip_buffer_t);
}

static void app_vsfip_release_buffer(struct vsfip_buffer_t *buffer)
{
	VSFPOOL_FREE(&usrapp.tcpip.buffer_pool, buffer);
}

static struct vsfip_socket_t* app_vsfip_get_socket(void)
{
	return VSFPOOL_ALLOC(&usrapp.tcpip.socket_pool, struct vsfip_socket_t);
}

static void app_vsfip_release_socket(struct vsfip_socket_t *socket)
{
	VSFPOOL_FREE(&usrapp.tcpip.socket_pool, socket);
}

static struct vsfip_tcppcb_t* app_vsfip_get_tcppcb(void)
{
#if APPCFG_VSFIP_TCPPCB_NUM > 0
	return VSFPOOL_ALLOC(&usrapp.tcpip.tcppcb_pool, struct vsfip_tcppcb_t);
#else
	return NULL;
#endif
}

static void app_vsfip_release_tcppcb(struct vsfip_tcppcb_t *tcppcb)
{
#if APPCFG_VSFIP_TCPPCB_NUM > 0
	VSFPOOL_FREE(&usrapp.tcpip.tcppcb_pool, tcppcb);
#endif
}

const struct vsfip_mem_op_t app_vsfip_mem_op =
{
	.get_buffer		= app_vsfip_get_buffer,
	.release_buffer	= app_vsfip_release_buffer,
	.get_socket		= app_vsfip_get_socket,
	.release_socket	= app_vsfip_release_socket,
	.get_tcppcb		= app_vsfip_get_tcppcb,
	.release_tcppcb	= app_vsfip_release_tcppcb,
};

// rndis on_connect
static void usrapp_rndis_on_connect(void *param)
{
	struct usrapp_t *app = (struct usrapp_t *)param;
	vsfip_dhcpd_start(&app->usbd.rndis.param.netif, &app->usbd.rndis.dhcpd);
}

static void usrapp_usbd_conn(void *p)
{
	struct usrapp_t *app = (struct usrapp_t *)p;

	// mal init
	for (int i = 0; i < APPCFG_VSFIP_BUFFER_NUM; i++)
		app->mal.pbuffer[i] = app->mal.buffer[i];

	// vsfip init
	for (int i = 0; i < dimof(app->mal.pbuffer); i++)
		app->tcpip.buffer_pool.buffer[i].buffer = app->tcpip.buffer_mem[i];
	VSFPOOL_INIT(&app->tcpip.buffer_pool, struct vsfip_buffer_t, APPCFG_VSFIP_BUFFER_NUM);
	VSFPOOL_INIT(&app->tcpip.socket_pool, struct vsfip_socket_t, APPCFG_VSFIP_SOCKET_NUM);
#if APPCFG_VSFIP_TCPPCB_NUM > 0
	VSFPOOL_INIT(&app->tcpip.tcppcb_pool, struct vsfip_tcppcb_t, APPCFG_VSFIP_TCPPCB_NUM);
#endif
	vsfip_init((struct vsfip_mem_op_t *)&app_vsfip_mem_op);

	// telnet init
	STREAM_INIT(&app->tcpip.telnetd.stream_rx);
	STREAM_INIT(&app->tcpip.telnetd.stream_tx);
	vsfip_telnetd_start(&app->tcpip.telnetd.telnetd);

	vsfscsi_init(&app->mal.scsi_dev);
	vsfusbd_device_init(&app->usbd.device);

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	vsfshell_init(&app->shell);
#endif

	app->usbd.device.drv->connect();
	if (app->hwcfg->usbd.pullup.port != VSFHAL_DUMMY_PORT)
		vsfhal_gpio_set(app->hwcfg->usbd.pullup.port, 1 << app->hwcfg->usbd.pullup.pin);
}

void usrapp_srt_init(struct usrapp_t *app)
{
	if (app->hwcfg->usbd.pullup.port != VSFHAL_DUMMY_PORT)
	{
		vsfhal_gpio_init(app->hwcfg->usbd.pullup.port);
		vsfhal_gpio_clear(app->hwcfg->usbd.pullup.port, 1 << app->hwcfg->usbd.pullup.pin);
		vsfhal_gpio_config_pin(app->hwcfg->usbd.pullup.port, app->hwcfg->usbd.pullup.pin, GPIO_OUTPP);
	}
	app->usbd.device.drv->disconnect();

	vsftimer_create_cb(200, 1, usrapp_usbd_conn, app);
}

