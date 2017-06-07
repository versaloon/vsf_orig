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

#include "component/file/fs/malfs/vsf_malfs.h"
#include "component/file/fs/malfs/fat/vsffat.h"

#define APPCFG_VSFTIMER_NUM				16
#define APPCFG_VSFSM_PENDSVQ_LEN		16
//#define APPCFG_VSFSM_MAINQ_LEN			16

#define APPCFG_VSFILE_NUM				8

// Note: shell also depend on APPCFG_BUFMGR_SIZE
#define APPCFG_BUFMGR_SIZE				(8 * 1024)

#include "fakefat32_fs.h"

// USB descriptors
static const uint8_t USB_DeviceDescriptor[] =
{
	0x12,	// bLength = 18
	USB_DT_DEVICE,	// USB_DESC_TYPE_DEVICE
	0x00,
	0x02,	// bcdUSB
	0x00,	// device class:
	0x00,	// device sub class
	0x00,	// device protocol
	0x40,	// max packet size
	0x83,
	0x04,	// vendor
	0x20,
	0x57,	// product
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
	32,		// wTotalLength:no of returned bytes*
	0x00,
	0x01,	// bNumInterfaces: 1 interface
	0x01,	// bConfigurationValue: Configuration value
	0x00,	// iConfiguration: Index of string descriptor describing the configuration
	0x80,	// bmAttributes: bus powered
	0x64,	// MaxPower 200 mA

	// Interface Descriptor for MSC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0,		// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x08,	// bInterfaceClass: MSC
	0x06,	// bInterfaceSubClass: SCSI
	0x50,	// bInterfaceProtocol:
	0x00,	// iInterface:

	// Endpoint 1 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x81,	// bEndpointAddress: (IN1)
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer

	// Endpoint 1 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x01,	// bEndpointAddress: (OUT1)
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
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
	'V', 0, 'S', 0, 'F', 0, 'M', 0, 'S', 0, 'C', 0
};

static const struct vsfusbd_desc_filter_t USB_descriptors[] =
{
	VSFUSBD_DESC_DEVICE(0, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor)),
	VSFUSBD_DESC_CONFIG(0, 0, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor)),
	VSFUSBD_DESC_STRING(0, 0, USB_StringLangID, sizeof(USB_StringLangID)),
	VSFUSBD_DESC_STRING(0x0409, 1, USB_StringVendor, sizeof(USB_StringVendor)),
	VSFUSBD_DESC_STRING(0x0409, 2, USB_StringProduct, sizeof(USB_StringProduct)),
	VSFUSBD_DESC_STRING(0x0409, 3, USB_StringSerial, sizeof(USB_StringSerial)),
	VSFUSBD_DESC_NULL
};

static vsf_err_t app_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
#if defined(APPCFG_VSFSM_PENDSVQ_LEN) && (APPCFG_VSFSM_PENDSVQ_LEN > 0)
static void app_pendsv_activate(struct vsfsm_evtq_t *q);
#endif

struct vsfapp_t
{
	struct usb_pullup_port_t
	{
		uint8_t port;
		uint8_t pin;
	} usb_pullup;

	// usb
	struct
	{
		struct
		{
			struct vsfusbd_MSCBOT_param_t param;
		} msc;
		struct vsfusbd_iface_t ifaces[1];
		struct vsfusbd_config_t config[1];
		struct vsfusbd_device_t device;
	} usbd;

	// mal
	struct
	{
		struct vsfscsi_device_t scsi_dev;
		struct vsfscsi_lun_t lun[1];

		struct vsf_scsistream_t scsistream;
		struct vsf_mal2scsi_t mal2scsi;
		struct vsfmal_t mal;
		struct fakefat32_param_t fakefat32;
		uint8_t *pbuffer[2];
		uint8_t buffer[2][512];
	} mal;

	struct vsfmim_t mim;
	struct vsffat_t fatfs;

	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t caller_pt;
	struct vsfsm_t sm;

	// private
	// buffer mamager
#if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
	VSFPOOL_DEFINE(vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
#endif

#if defined(APPCFG_VSFILE_NUM) && (APPCFG_VSFILE_NUM > 0)
	VSFPOOL_DEFINE(vfsfile_pool, struct vsfile_vfsfile_t, APPCFG_VSFILE_NUM);
#endif

#if defined(APPCFG_VSFSM_PENDSVQ_LEN) && (APPCFG_VSFSM_PENDSVQ_LEN > 0)
	struct vsfsm_evtq_t pendsvq;
	struct vsfsm_evtq_element_t pendsvq_ele[APPCFG_VSFSM_PENDSVQ_LEN];
#endif
#if defined(APPCFG_VSFSM_MAINQ_LEN) && (APPCFG_VSFSM_MAINQ_LEN > 0)
	struct vsfsm_evtq_t mainq;
	struct vsfsm_evtq_element_t mainq_ele[APPCFG_VSFSM_MAINQ_LEN];
#endif

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	uint8_t bufmgr_buffer[APPCFG_BUFMGR_SIZE];
#endif
} static app =
{
	.usb_pullup.port						= USB_PULLUP_PORT,
	.usb_pullup.pin							= USB_PULLUP_PIN,

	.usbd.msc.param.ep_in					= 1,
	.usbd.msc.param.ep_out					= 1,
	.usbd.msc.param.scsi_dev				= &app.mal.scsi_dev,

	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_MSCBOT_class,
	.usbd.ifaces[0].protocol_param			= &app.usbd.msc.param,
	.usbd.config[0].num_of_ifaces			= dimof(app.usbd.ifaces),
	.usbd.config[0].iface					= app.usbd.ifaces,
	.usbd.device.num_of_configuration		= dimof(app.usbd.config),
	.usbd.device.config						= app.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.device.drv						= (struct interface_usbd_t *)&core_interfaces.usbd,
	.usbd.device.int_priority				= 0,

	.mal.fakefat32.sector_size				= 512,
	.mal.fakefat32.sector_number			= 0x00001000,
	.mal.fakefat32.sectors_per_cluster		= 8,
	.mal.fakefat32.volume_id				= 0x0CA93E47,
	.mal.fakefat32.disk_id					= 0x12345678,
	.mal.fakefat32.root[0].memfile.file.name= "ROOT",
	.mal.fakefat32.root[0].memfile.d.child	= (struct vsfile_memfile_t *)fakefat32_root_dir,

	.mal.mal.drv							= &fakefat32_mal_drv,
	.mal.mal.param							= &app.mal.fakefat32,
	.mal.pbuffer[0]							= app.mal.buffer[0],
	.mal.pbuffer[1]							= app.mal.buffer[1],
	.mal.scsistream.mbuf.count				= dimof(app.mal.buffer),
	.mal.scsistream.mbuf.size				= sizeof(app.mal.buffer[0]),
	.mal.scsistream.mbuf.buffer_list		= app.mal.pbuffer,

	.mal.mal2scsi.malstream.mal				= &app.mal.mal,
	.mal.mal2scsi.cparam.block_size			= 512,
	.mal.mal2scsi.cparam.removable			= false,
	.mal.mal2scsi.cparam.vendor				= "Simon   ",
	.mal.mal2scsi.cparam.product			= "VSFDriver       ",
	.mal.mal2scsi.cparam.revision			= "1.00",
	.mal.mal2scsi.cparam.type				= SCSI_PDT_DIRECT_ACCESS_BLOCK,

	.mal.lun[0].op							= (struct vsfscsi_lun_op_t *)&vsf_mal2scsi_op,
	// lun->stream MUST be scsistream for mal2scsi
	.mal.lun[0].stream						= (struct vsf_stream_t *)&app.mal.scsistream,
	.mal.lun[0].param						= &app.mal.mal2scsi,
	.mal.scsi_dev.max_lun					= 0,
	.mal.scsi_dev.lun						= app.mal.lun,

	.mim.mal.drv							= &vsfmim_drv,
	.mim.realmal							= &app.mal.mal,
	.mim.addr								= 64 * 512,
	.mim.size								= (0x00001000 - 64) * 512,
	.fatfs.malfs.malstream.mal				= (struct vsfmal_t *)&app.mim,

	.pt.thread								= app_thread,

#if defined(APPCFG_VSFSM_PENDSVQ_LEN) && (APPCFG_VSFSM_PENDSVQ_LEN > 0)
	.pendsvq.size							= dimof(app.pendsvq_ele),
	.pendsvq.queue							= app.pendsvq_ele,
	.pendsvq.activate						= app_pendsv_activate,
#endif
#if defined(APPCFG_VSFSM_MAINQ_LEN) && (APPCFG_VSFSM_MAINQ_LEN > 0)
	.mainq.size								= dimof(app.mainq_ele),
	.mainq.queue							= app.mainq_ele,
	.mainq.activate							= NULL,
#endif
};

// vsftimer buffer manager
#if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
static struct vsftimer_t* vsftimer_memop_alloc(void)
{
	return VSFPOOL_ALLOC(&app.vsftimer_pool, struct vsftimer_t);
}

static void vsftimer_memop_free(struct vsftimer_t *timer)
{
	VSFPOOL_FREE(&app.vsftimer_pool, timer);
}

const struct vsftimer_mem_op_t vsftimer_memop =
{
	.alloc			= vsftimer_memop_alloc,
	.free			= vsftimer_memop_free,
};

// tickclk interrupt, simply call vsftimer_callback_int
static void app_tickclk_callback_int(void *param)
{
	vsftimer_callback_int();
}
#endif

// vsfile_memop
struct vsfile_vfsfile_t* app_vsfile_alloc_vfs(void)
{
	return VSFPOOL_ALLOC(&app.vfsfile_pool, struct vsfile_vfsfile_t);
}

static void app_vsfile_free_vfs(struct vsfile_vfsfile_t *vfsfile)
{
	VSFPOOL_FREE(&app.vfsfile_pool, vfsfile);
}

static const struct vsfile_memop_t app_vsfile_memop =
{
	.alloc_vfs = app_vsfile_alloc_vfs,
	.free_vfs = app_vsfile_free_vfs,
};

static vsf_err_t app_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	static struct vsfile_t *file;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	vsfhal_core_init(NULL);
	vsfhal_tickclk_init();
	vsfhal_tickclk_start();

#if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
	VSFPOOL_INIT(&app.vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
	vsftimer_init((struct vsftimer_mem_op_t *)&vsftimer_memop);
	vsfhal_tickclk_config_cb(app_tickclk_callback_int, NULL);
#endif

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	vsf_bufmgr_init(app.bufmgr_buffer, sizeof(app.bufmgr_buffer));
#endif

#if defined(APPCFG_VSFILE_NUM) && (APPCFG_VSFILE_NUM > 0)
	VSFPOOL_INIT(&app.vfsfile_pool, struct vsfile_vfsfile_t, APPCFG_VSFILE_NUM);
	vsfile_init((struct vsfile_memop_t *)&app_vsfile_memop);

	app.caller_pt.sm = pt->sm;

	app.caller_pt.state = 0;
	app.caller_pt.user_data = &app.mal.mal;
	vsfsm_pt_entry(pt);
	err = vsfmal_init(&app.caller_pt, evt);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	app.caller_pt.state = 0;
	app.caller_pt.user_data = &app.mim;
	vsfsm_pt_entry(pt);
	err = vsfmal_init(&app.caller_pt, evt);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	// create msc_root and fat_root under root
	app.caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_addfile(&app.caller_pt, evt, NULL, "msc_root", VSFILE_ATTR_DIRECTORY);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	app.caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_addfile(&app.caller_pt, evt, NULL, "fat_root", VSFILE_ATTR_DIRECTORY);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	// mount fakefat32 under /msc_root
	app.caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_getfile(&app.caller_pt, evt, NULL, "/msc_root", &file);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	app.caller_pt.state = 0;
	app.caller_pt.user_data = &app.mal.fakefat32;
	vsfsm_pt_entry(pt);
	err = vsfile_mount(&app.caller_pt, evt, (struct vsfile_fsop_t *)&fakefat32_fs_op, file);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	// mount fatfs under /fat_root
	app.caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_getfile(&app.caller_pt, evt, NULL, "/fat_root", &file);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	app.caller_pt.state = 0;
	app.caller_pt.user_data = &app.fatfs;
	vsfsm_pt_entry(pt);
	err = vsfile_mount(&app.caller_pt, evt, (struct vsfile_fsop_t *)&vsffat_op, file);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;

	// tester
	app.caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_getfile(&app.caller_pt, evt, NULL, "/fat_root/Driver/Windows/VSFRNDIS.inf", &file);
	if (err > 0) return err; else if (err < 0) return VSFERR_FAIL;
#endif

	vsfscsi_init(&app.mal.scsi_dev);
	vsfusbd_device_init(&app.usbd.device);

	if (app.usb_pullup.port != VSFHAL_DUMMY_PORT)
	{
		vsfhal_gpio_init(app.usb_pullup.port);
		vsfhal_gpio_clear(app.usb_pullup.port, 1 << app.usb_pullup.pin);
		vsfhal_gpio_config_pin(app.usb_pullup.port,
										app.usb_pullup.pin, GPIO_OUTPP);
	}
	app.usbd.device.drv->disconnect();

	vsfsm_pt_delay(pt, 200);

	if (app.usb_pullup.port != VSFHAL_DUMMY_PORT)
	{
		vsfhal_gpio_set(app.usb_pullup.port, 1 << app.usb_pullup.pin);
	}
	app.usbd.device.drv->connect();

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

#if defined(APPCFG_VSFSM_PENDSVQ_LEN) && (APPCFG_VSFSM_PENDSVQ_LEN > 0)
static void app_on_pendsv(void *param)
{
	struct vsfsm_evtq_t *evtq_cur = param, *evtq_old = vsfsm_evtq_set(evtq_cur);

	while (vsfsm_get_event_pending())
	{
		vsfsm_poll();
	}
	vsfsm_evtq_set(evtq_old);
}

static void app_pendsv_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_core_pendsv_trigger();
}
#endif

int main(void)
{
	vsf_enter_critical();
#if defined(APPCFG_VSFSM_MAINQ_LEN) && (APPCFG_VSFSM_MAINQ_LEN > 0)
	vsfsm_evtq_init(&app.mainq);
	vsfsm_evtq_set(&app.mainq);
#endif
#if defined(APPCFG_VSFSM_PENDSVQ_LEN) && (APPCFG_VSFSM_PENDSVQ_LEN > 0)
	vsfsm_evtq_init(&app.pendsvq);
	vsfsm_evtq_set(&app.pendsvq);
	vsfhal_core_pendsv_config(app_on_pendsv, &app.pendsvq);
#endif
	vsfsm_pt_init(&app.sm, &app.pt);

	vsf_leave_critical();

#if defined(APPCFG_VSFSM_MAINQ_LEN) && (APPCFG_VSFSM_MAINQ_LEN > 0)
	vsfsm_evtq_set(&app.mainq);
#else
	vsfsm_evtq_set(NULL);
#endif
	while (1)
	{
		// no thread runs in mainq, just sleep in main loop
		vsfhal_core_sleep(SLEEP_WFI);
	}
}
