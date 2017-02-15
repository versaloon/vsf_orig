#include "vsf.h"
#include "app_hw_cfg.h"
#include "usrapp.h"

static const struct app_hwcfg_t app_hwcfg =
{
	.usbd.pullup.port = USB_PULLUP_PORT,
	.usbd.pullup.pin = USB_PULLUP_PIN,
};

struct usrapp_param_t
{
	struct
	{
		uint8_t CDC_DeviceDescriptor[18];
		uint8_t CDC_ConfigDescriptor[32];
		uint8_t CDC_StringLangID[4];
		uint8_t CDC_StringVendor[20];
		uint8_t CDC_StringSerial[50];
		uint8_t CDC_StringProduct[22];
		struct vsfusbd_desc_filter_t CDC_StdDesc[7];
	} usbd;
} static const usrapp_param =
{
	.usbd.CDC_DeviceDescriptor =
	{
		USB_DT_DEVICE_SIZE,
		USB_DT_DEVICE,
		0x10, 0x01,	// bcdUSB
		0xFF,		// device class
		0x00,		// device sub class
		0x00,		// device protocol
		0x08,		// max packet size
		0xA1, 0xA2,	// vendor
		0x08, 0x07,	// product
		0x00, 0x01,	// bcdDevice
		1,			// manu facturer
		2,			// product
		3,			// serial number
		0x01		// number of configuration 
	},
	.usbd.CDC_ConfigDescriptor =
	{
		USB_DT_CONFIG_SIZE,
		USB_DT_CONFIG,
		32, 0,		// wTotalLength
		0x01,		// bNumInterfaces: 2 interface
		0x01,		// bConfigurationValue: Configuration value
		0x00,		// iConfiguration: Index of string descriptor describing the configuration
		0x80,		// bmAttributes: bus powered
		0x32,		// MaxPower

		USB_DT_INTERFACE_SIZE,
		USB_DT_INTERFACE,
		0x00,		// bInterfaceNumber: Number of Interface
		0x00,		// bAlternateSetting: Alternate setting
		0x02,		// bNumEndpoints
		0x00,		// bInterfaceClass
		0x00,		// bInterfaceSubClass
		0x00,		// nInterfaceProtocol
		0x00,		// iInterface:

		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		0x81,		// bEndpointAddress: (IN1)
		0x03,		// bmAttributes: Interrupt
		8,			// wMaxPacketSize:
		0x00,
		10,			// bInterval:

		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		0x01,		// bEndpointAddress: (IN1)
		0x03,		// bmAttributes: Interrupt
		8,			// wMaxPacketSize:
		0x00,
		10,			// bInterval:
	},
	.usbd.CDC_StringLangID =
	{
		4,
		USB_DT_STRING,
		0x09,
		0x04,
	},
	.usbd.CDC_StringVendor =
	{
		20,
		USB_DT_STRING,
		'S', 0, 'i', 0, 'm', 0, 'o', 0, 'n', 0, 'Q', 0, 'i', 0, 'a', 0,
		'n', 0,
	},
	.usbd.CDC_StringSerial =
	{
		50,
		USB_DT_STRING,
		'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0,
		'8', 0, '9', 0, 'A', 0, 'B', 0, 'C', 0, 'D', 0, 'E', 0, 'F', 0,
		'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
	},
	.usbd.CDC_StringProduct =
	{
		22,
		USB_DT_STRING,
		'U', 0, 'S', 0, 'B', 0, ' ', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0,
		'a', 0, 'l', 0,
	},
	.usbd.CDC_StdDesc =
	{
		VSFUSBD_DESC_DEVICE(0, usrapp_param.usbd.CDC_DeviceDescriptor, sizeof(usrapp_param.usbd.CDC_DeviceDescriptor)),
		VSFUSBD_DESC_CONFIG(0, 0, usrapp_param.usbd.CDC_ConfigDescriptor, sizeof(usrapp_param.usbd.CDC_ConfigDescriptor)),
		VSFUSBD_DESC_STRING(0, 0, usrapp_param.usbd.CDC_StringLangID, sizeof(usrapp_param.usbd.CDC_StringLangID)),
		VSFUSBD_DESC_STRING(0x0409, 1, usrapp_param.usbd.CDC_StringVendor, sizeof(usrapp_param.usbd.CDC_StringVendor)),
		VSFUSBD_DESC_STRING(0x0409, 2, usrapp_param.usbd.CDC_StringProduct, sizeof(usrapp_param.usbd.CDC_StringProduct)),
		VSFUSBD_DESC_STRING(0x0409, 3, usrapp_param.usbd.CDC_StringSerial, sizeof(usrapp_param.usbd.CDC_StringSerial)),
		VSFUSBD_DESC_NULL,
	},
};

static void usrapp_stream_in(void *p)
{
	struct usbd_cdc_t *cdc = (struct usbd_cdc_t *)p;
	uint8_t ch[8];
	struct vsf_buffer_t buffer = {.buffer = ch};

	do {
		buffer.size = sizeof(ch);
		buffer.size = STREAM_READ(&cdc->stream_rx, &buffer);
		if (buffer.size)
			STREAM_WRITE(&cdc->stream_tx, &buffer);
	} while (buffer.size);
}

struct usrapp_t usrapp =
{
	.hwcfg											= &app_hwcfg,

	.usbd.cdc.param.CDC.ep_notify					= 1,
	.usbd.cdc.param.CDC.ep_out						= 2,
	.usbd.cdc.param.CDC.ep_in						= 2,
	.usbd.cdc.param.CDC.stream_tx					= (struct vsf_stream_t *)&usrapp.usbd.cdc.stream_tx,
	.usbd.cdc.param.CDC.stream_rx					= (struct vsf_stream_t *)&usrapp.usbd.cdc.stream_rx,
	.usbd.cdc.param.line_coding.bitrate				= 115200,
	.usbd.cdc.param.line_coding.stopbittype			= 0,
	.usbd.cdc.param.line_coding.paritytype			= 0,
	.usbd.cdc.param.line_coding.datatype			= 8,

	.usbd.cdc.stream_tx.stream.op					= &fifostream_op,
	.usbd.cdc.stream_tx.mem.buffer.buffer			= (uint8_t *)&usrapp.usbd.cdc.txbuff,
	.usbd.cdc.stream_tx.mem.buffer.size				= sizeof(usrapp.usbd.cdc.txbuff),

	.usbd.cdc.stream_rx.stream.op					= &fifostream_op,
	.usbd.cdc.stream_rx.mem.buffer.buffer			= (uint8_t *)&usrapp.usbd.cdc.rxbuff,
	.usbd.cdc.stream_rx.mem.buffer.size				= sizeof(usrapp.usbd.cdc.rxbuff),
	.usbd.cdc.stream_rx.stream.callback_rx.param	= &usrapp.usbd.cdc,
	.usbd.cdc.stream_rx.stream.callback_rx.on_inout	= usrapp_stream_in,

	.usbd.ifaces[0].class_protocol					= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[0].protocol_param					= &usrapp.usbd.cdc.param,
	.usbd.ifaces[1].class_protocol					= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[1].protocol_param					= &usrapp.usbd.cdc.param,
	.usbd.config[0].num_of_ifaces					= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface							= usrapp.usbd.ifaces,
	.usbd.device.num_of_configuration				= dimof(usrapp.usbd.config),
	.usbd.device.config								= usrapp.usbd.config,
	.usbd.device.desc_filter						= (struct vsfusbd_desc_filter_t *)usrapp_param.usbd.CDC_StdDesc,
	.usbd.device.device_class_iface					= 0,
	.usbd.device.drv								= (struct interface_usbd_t *)&vsfsdcd_usbd,
	.usbd.device.int_priority						= -1,
};

static void usrapp_usbd_conn(void *p)
{
	struct usrapp_t *app = (struct usrapp_t *)p;

	STREAM_CONNECT_RX(&usrapp.usbd.cdc.stream_rx);
	STREAM_CONNECT_TX(&usrapp.usbd.cdc.stream_tx);

	vsfusbd_device_init(&app->usbd.device);
	app->usbd.device.drv->connect();
	if (app->hwcfg->usbd.pullup.port != IFS_DUMMY_PORT)
		vsfhal_gpio_set(app->hwcfg->usbd.pullup.port, 1 << app->hwcfg->usbd.pullup.pin);
}

void usrapp_poll(struct usrapp_t *app)
{
	vsfhal_tickclk_poll();
	app->usbd.device.drv->poll();
}

void usrapp_init(struct usrapp_t *app)
{
	if (app->hwcfg->usbd.pullup.port != IFS_DUMMY_PORT)
	{
		vsfhal_gpio_init(app->hwcfg->usbd.pullup.port);
		vsfhal_gpio_clear(app->hwcfg->usbd.pullup.port, 1 << app->hwcfg->usbd.pullup.pin);
		vsfhal_gpio_config_pin(app->hwcfg->usbd.pullup.port, app->hwcfg->usbd.pullup.pin, GPIO_OUTPP);
	}
	app->usbd.device.drv->disconnect();

	vsftimer_create_cb(200, 1, usrapp_usbd_conn, app);
}
