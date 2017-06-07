#include "vsf.h"
#include "tool/fakefat32/fakefat32.h"

#define APPCFG_VSFTIMER_NUM				16
#define APPCFG_VSFSM_PENDSVQ_LEN		16

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

// fakefs
static struct fakefat32_file_t fakefat32_lost_dir[] =
{
	{
		.memfile.file.name = ".",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY,
	},
	{
		.memfile.file.name = "..",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY,
	},
	{
		.memfile.file.name = NULL,
	},
};
static uint8_t Win_recycle_DESKTOP_INI[] =
"[.ShellClassInfo]\r\n\
CLSID={645FF040-5081-101B-9F08-00AA002F954E}\r\n\
LocalizedResourceName=@%SystemRoot%\\system32\\shell32.dll,-8964\r\n";
static struct fakefat32_file_t fakefat32_recycle_dir[] =
{
	{
		.memfile.file.name = ".",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY,
	},
	{
		.memfile.file.name = "..",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY,
	},
	{
		.memfile.file.name = "DESKTOP.INI",
		.memfile.file.size = sizeof(Win_recycle_DESKTOP_INI) - 1,
		.memfile.file.attr = VSFILE_ATTR_ARCHIVE,
		.memfile.f.buff = Win_recycle_DESKTOP_INI,
	},
	{
		.memfile.file.name = NULL,
	},
};
static uint8_t Win10_IndexerVolumeGuid[] =
{
	0x7B,0x00,0x45,0x00,0x34,0x00,0x42,0x00,0x38,0x00,0x37,0x00,0x41,0x00,0x39,0x00,
	0x34,0x00,0x2D,0x00,0x39,0x00,0x32,0x00,0x32,0x00,0x39,0x00,0x2D,0x00,0x34,0x00,
	0x38,0x00,0x32,0x00,0x34,0x00,0x2D,0x00,0x41,0x00,0x44,0x00,0x39,0x00,0x31,0x00,
	0x2D,0x00,0x41,0x00,0x42,0x00,0x44,0x00,0x41,0x00,0x44,0x00,0x39,0x00,0x45,0x00,
	0x30,0x00,0x43,0x00,0x34,0x00,0x30,0x00,0x34,0x00,0x7D,0x00
};
static struct fakefat32_file_t fakefat32_systemvolumeinformation_dir[] =
{
	{
		.memfile.file.name = ".",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY,
	},
	{
		.memfile.file.name = "..",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY,
	},
	{
		.memfile.file.name = "IndexerVolumeGuid",
		.memfile.file.size = sizeof(Win10_IndexerVolumeGuid),
		.memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN,
		.memfile.f.buff = Win10_IndexerVolumeGuid,
	},
	{
		.memfile.file.name = NULL,
	},
};
static struct fakefat32_file_t fakefat32_root_dir[] =
{
	{
		.memfile.file.name = "MSCBoot",
		.memfile.file.attr = VSFILE_ATTR_VOLUMID,
	},
	// "LOST.DIR is nesessary to make Android happy"
	{
		.memfile.file.name = "LOST.DIR",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN,
		.memfile.d.child = (struct vsfile_memfile_t *)fakefat32_lost_dir,
	},
	// "$RECYCLE.BIN" is necessary to make Windows happy
	{
		.memfile.file.name = "$RECYCLE.BIN",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN,
		.memfile.d.child = (struct vsfile_memfile_t *)fakefat32_recycle_dir,
	},
	// "System Voumne Information" is necessary to make Win10 happy
	{
		.memfile.file.name = "System Volume Information",
		.memfile.file.attr = VSFILE_ATTR_DIRECTORY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN,
		.memfile.d.child = (struct vsfile_memfile_t *)fakefat32_systemvolumeinformation_dir,
	},
	{
		.memfile.file.name = NULL,
	},
};

// app state machine events
#define APP_EVT_USBPU_TO				VSFSM_EVT_USER_LOCAL_INSTANT + 0

static struct vsfsm_state_t* app_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt);
static void app_pendsv_activate(struct vsfsm_evtq_t *q);

struct vsfapp_t
{
	// hw
	struct usb_pullup_port_t
	{
		uint8_t port;
		uint8_t pin;
	} usb_pullup;

	// mal
	struct
	{
		struct vsfscsi_device_t scsi_dev;
		struct vsfscsi_lun_t lun[1];

		struct vsf_mal2scsi_t mal2scsi;
		struct vsfmal_t mal;
		struct fakefat32_param_t fakefat32;
		uint8_t *pbuffer[2];
		uint8_t buffer[2][512];
	} mal;

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

	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t caller_pt;
	struct vsfsm_t sm;

	struct vsfsm_evtq_t pendsvq;

	// private
	// buffer mamager
	VSFPOOL_DEFINE(vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
	VSFPOOL_DEFINE(vfsfile_pool, struct vsfile_vfsfile_t, 2);

	struct vsfsm_evtq_element_t pendsvq_ele[APPCFG_VSFSM_PENDSVQ_LEN];
} static app =
{
	.usb_pullup.port						= USB_PULLUP_PORT,
	.usb_pullup.pin							= USB_PULLUP_PIN,

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

	.mal.mal2scsi.malstream.mal				= &app.mal.mal,
	.mal.mal2scsi.multibuf.count			= dimof(app.mal.buffer),
	.mal.mal2scsi.multibuf.size				= sizeof(app.mal.buffer[0]),
	.mal.mal2scsi.multibuf.buffer_list		= app.mal.pbuffer,
	.mal.mal2scsi.cparam.block_size			= 512,
	.mal.mal2scsi.cparam.removable			= false,
	.mal.mal2scsi.cparam.vendor				= "Simon   ",
	.mal.mal2scsi.cparam.product			= "MSCBoot         ",
	.mal.mal2scsi.cparam.revision			= "1.00",
	.mal.mal2scsi.cparam.type				= SCSI_PDT_DIRECT_ACCESS_BLOCK,

	.mal.lun[0].op							= (struct vsfscsi_lun_op_t *)&vsf_mal2scsi_op,
	.mal.lun[0].param						= &app.mal.mal2scsi,
	.mal.scsi_dev.max_lun					= 0,
	.mal.scsi_dev.lun						= app.mal.lun,

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

	.sm.init_state.evt_handler				= app_evt_handler,

	.pendsvq.size							= dimof(app.pendsvq_ele),
	.pendsvq.queue							= app.pendsvq_ele,
	.pendsvq.activate						= app_pendsv_activate,
};

// tickclk interrupt, simply call vsftimer_callback_int
static void app_tickclk_callback_int(void *param)
{
	vsftimer_callback_int();
}

static void app_pendsv_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_core_pendsv_trigger();
}

// vsftimer buffer mamager
static struct vsftimer_t* vsftimer_memop_alloc(void)
{
	return VSFPOOL_ALLOC(&app.vsftimer_pool, struct vsftimer_t);
}

static void vsftimer_memop_free(struct vsftimer_t *timer)
{
	VSFPOOL_FREE(&app.vsftimer_pool, timer);
}

struct vsftimer_mem_op_t vsftimer_memop =
{
	.alloc	= vsftimer_memop_alloc,
	.free	= vsftimer_memop_free,
};

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

static struct vsfsm_state_t *
app_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	switch (evt)
	{
	case VSFSM_EVT_INIT:
		vsfhal_core_init(NULL);
		vsfhal_tickclk_init();
		vsfhal_tickclk_start();

		VSFPOOL_INIT(&app.vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
		vsftimer_init(&vsftimer_memop);
		vsfhal_tickclk_config_cb(app_tickclk_callback_int, NULL);

		// fs init: currently supported fs are non-block, so ugly pt code below
		{
			struct vsfile_t *file;

			VSFPOOL_INIT(&app.vfsfile_pool, struct vsfile_vfsfile_t, 2);
			vsfile_init((struct vsfile_memop_t *)&app_vsfile_memop);

			// create msc_root and httpd_root under root
			app.caller_pt.state = 0;
			vsfile_addfile(&app.caller_pt, 0, NULL, "msc_root", VSFILE_ATTR_DIRECTORY);

			// mount fakefat32 under /msc_root
			app.caller_pt.state = 0;
			vsfile_getfile(&app.caller_pt, 0, NULL, "/msc_root", &file);
			app.caller_pt.state = 0;
			app.caller_pt.user_data = &app.mal.fakefat32;
			vsfile_mount(&app.caller_pt, 0, (struct vsfile_fsop_t *)&fakefat32_fs_op, file);
		}

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
		vsftimer_create(sm, 200, 1, APP_EVT_USBPU_TO);
		break;
	case APP_EVT_USBPU_TO:
		if (app.usb_pullup.port != VSFHAL_DUMMY_PORT)
		{
			vsfhal_gpio_set(app.usb_pullup.port, 1 << app.usb_pullup.pin);
		}
		app.usbd.device.drv->connect();
		break;
	}
	return NULL;
}

static void app_on_pendsv(void *param)
{
	struct vsfsm_evtq_t *evtq_cur = param, *evtq_old = vsfsm_evtq_set(evtq_cur);

	while (vsfsm_get_event_pending())
	{
		vsfsm_poll();
	}
	vsfsm_evtq_set(evtq_old);
}

int main(void)
{
	vsf_enter_critical();
	vsfsm_evtq_init(&app.pendsvq);

	vsfsm_evtq_set(&app.pendsvq);
	vsfsm_init(&app.sm);

	vsfhal_core_pendsv_config(app_on_pendsv, &app.pendsvq);
	vsf_leave_critical();

	vsfsm_evtq_set(NULL);
	while (1)
	{
		// no thread runs in mainq, just sleep in main loop
		vsfhal_core_sleep(SLEEP_WFI);
	}
}
