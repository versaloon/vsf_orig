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

#define APPCFG_VSFTIMER_NUM				16
#define APPCFG_VSFSM_PENDSVQ_LEN		16
//#define APPCFG_VSFSM_MAINQ_LEN			16

// Note: shell also depend on APPCFG_BUFMGR_SIZE
//#define APPCFG_BUFMGR_SIZE				(4 * 1024)

#define APPCFG_VSFIP_BUFFER_NUM			8
#define APPCFG_VSFIP_SOCKET_NUM			8
#define APPCFG_VSFIP_TCPPCB_NUM			8

#include "fakefat32_fs.h"
#include "httpd_fs.h"
#include "usbd_desc.h"

static const struct vsfusbd_desc_filter_t USB_descriptors[] =
{
	VSFUSBD_DESC_DEVICE(0, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor)),
	VSFUSBD_DESC_CONFIG(0, 0, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor)),
	VSFUSBD_DESC_STRING(0, 0, USB_StringLangID, sizeof(USB_StringLangID)),
	VSFUSBD_DESC_STRING(0x0409, 1, USB_StringVendor, sizeof(USB_StringVendor)),
	VSFUSBD_DESC_STRING(0x0409, 2, USB_StringProduct, sizeof(USB_StringProduct)),
	VSFUSBD_DESC_STRING(0x0409, 3, USB_StringSerial, sizeof(USB_StringSerial)),
	VSFUSBD_DESC_STRING(0x0409, 4, RNDIS_StringFunc, sizeof(RNDIS_StringFunc)),
	VSFUSBD_DESC_STRING(0x0409, 5, CDC_StringFunc, sizeof(CDC_StringFunc)),
	VSFUSBD_DESC_STRING(0x0409, 6, MSC_StringFunc, sizeof(MSC_StringFunc)),
	VSFUSBD_DESC_NULL
};

// app state machine events
#define APP_EVT_USBPU_TO				VSFSM_EVT_USER_LOCAL_INSTANT + 0

static struct vsfsm_state_t* app_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt);
#if defined(APPCFG_VSFSM_PENDSVQ_LEN) && (APPCFG_VSFSM_PENDSVQ_LEN > 0)
static void app_pendsv_activate(struct vsfsm_evtq_t *q);
#endif

void app_rndis_on_connect(void *param);

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

		struct vsf_scsistream_t scsistream;
		struct vsf_mal2scsi_t mal2scsi;
		struct vsfmal_t mal;
		struct fakefat32_param_t fakefat32;
		uint8_t *pbuffer[2];
		uint8_t buffer[2][512];
	} mal;

	// usb & shell
	struct
	{
		struct
		{
			struct vsfusbd_RNDIS_param_t param;
			struct vsfip_dhcpd_t dhcpd;
		} rndis;
		struct
		{
			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[65];
			uint8_t rxbuff[65];
		} cdc;
		struct
		{
			struct vsfusbd_MSCBOT_param_t param;
		} msc;
		struct vsfusbd_iface_t ifaces[5];
		struct vsfusbd_config_t config[1];
		struct vsfusbd_device_t device;
	} usbd;
#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	struct vsfshell_t shell;
#endif

	struct
	{
		VSFPOOL_DEFINE(buffer_pool, struct vsfip_buffer_t, APPCFG_VSFIP_BUFFER_NUM);
		VSFPOOL_DEFINE(socket_pool, struct vsfip_socket_t, APPCFG_VSFIP_SOCKET_NUM);
		VSFPOOL_DEFINE(tcppcb_pool, struct vsfip_tcppcb_t, APPCFG_VSFIP_TCPPCB_NUM);
		uint8_t buffer_mem[APPCFG_VSFIP_BUFFER_NUM][VSFIP_BUFFER_SIZE];

		struct
		{
			struct vsfip_telnetd_t telnetd;
			struct vsfip_telnetd_session_t sessions[1];

			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[65];
			uint8_t rxbuff[65];
		} telnetd;
	} vsfip;

	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t caller_pt;
	struct vsfsm_t sm;

	// private
	// buffer mamager
#if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
	VSFPOOL_DEFINE(vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
#endif

	VSFPOOL_DEFINE(vfsfile_pool, struct vsfile_vfsfile_t, 2);

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

	.usbd.rndis.param.CDCACM.CDC.ep_notify	= 1,
	.usbd.rndis.param.CDCACM.CDC.ep_out		= 2,
	.usbd.rndis.param.CDCACM.CDC.ep_in		= 2,
	.usbd.rndis.param.mac.size				= 6,
	.usbd.rndis.param.mac.addr.s_addr64		= 0x0605040302E0,
	.usbd.rndis.param.cb.param				= &app,
	.usbd.rndis.param.cb.on_connect			= app_rndis_on_connect,
	.usbd.cdc.param.CDC.ep_notify			= 3,
	.usbd.cdc.param.CDC.ep_out				= 4,
	.usbd.cdc.param.CDC.ep_in				= 4,
	.usbd.cdc.param.CDC.stream_tx			= (struct vsf_stream_t *)&app.usbd.cdc.stream_tx,
	.usbd.cdc.param.CDC.stream_rx			= (struct vsf_stream_t *)&app.usbd.cdc.stream_rx,
	.usbd.cdc.param.line_coding.bitrate		= 115200,
	.usbd.cdc.param.line_coding.stopbittype	= 0,
	.usbd.cdc.param.line_coding.paritytype	= 0,
	.usbd.cdc.param.line_coding.datatype	= 8,
	.usbd.cdc.stream_tx.stream.op			= &fifostream_op,
	.usbd.cdc.stream_tx.mem.buffer.buffer	= (uint8_t *)&app.usbd.cdc.txbuff,
	.usbd.cdc.stream_tx.mem.buffer.size		= sizeof(app.usbd.cdc.txbuff),
	.usbd.cdc.stream_rx.stream.op			= &fifostream_op,
	.usbd.cdc.stream_rx.mem.buffer.buffer	= (uint8_t *)&app.usbd.cdc.rxbuff,
	.usbd.cdc.stream_rx.mem.buffer.size		= sizeof(app.usbd.cdc.rxbuff),
	.usbd.msc.param.ep_in					= 5,
	.usbd.msc.param.ep_out					= 5,
	.usbd.msc.param.scsi_dev				= &app.mal.scsi_dev,
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_RNDISControl_class,
	.usbd.ifaces[0].protocol_param			= &app.usbd.rndis.param,
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_RNDISData_class,
	.usbd.ifaces[1].protocol_param			= &app.usbd.rndis.param,
	.usbd.ifaces[2].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[2].protocol_param			= &app.usbd.cdc.param,
	.usbd.ifaces[3].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[3].protocol_param			= &app.usbd.cdc.param,
	.usbd.ifaces[4].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_MSCBOT_class,
	.usbd.ifaces[4].protocol_param			= &app.usbd.msc.param,
	.usbd.config[0].num_of_ifaces			= dimof(app.usbd.ifaces),
	.usbd.config[0].iface					= app.usbd.ifaces,
	.usbd.device.num_of_configuration		= dimof(app.usbd.config),
	.usbd.device.config						= app.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.device.drv						= (struct interface_usbd_t *)&core_interfaces.usbd,
	.usbd.device.int_priority				= 0,

	.usbd.rndis.param.netif.macaddr.size			= 6,
	.usbd.rndis.param.netif.macaddr.addr.s_addr64	= 0x0E0D0C0B0AE0,
	.usbd.rndis.param.netif.ipaddr.size				= 4,
	.usbd.rndis.param.netif.ipaddr.addr.s_addr		= 0x01202020,
	.usbd.rndis.param.netif.netmask.size			= 4,
	.usbd.rndis.param.netif.netmask.addr.s_addr		= 0x00FFFFFF,
	.usbd.rndis.param.netif.gateway.size			= 4,
	.usbd.rndis.param.netif.gateway.addr.s_addr		= 0x01202020,

	.vsfip.telnetd.telnetd.port						= 23,
	.vsfip.telnetd.telnetd.session_num				= dimof(app.vsfip.telnetd.sessions),
	.vsfip.telnetd.sessions[0].stream_tx			= (struct vsf_stream_t *)&app.vsfip.telnetd.stream_tx,
	.vsfip.telnetd.sessions[0].stream_rx			= (struct vsf_stream_t *)&app.vsfip.telnetd.stream_rx,
	.vsfip.telnetd.stream_tx.stream.op				= &fifostream_op,
	.vsfip.telnetd.stream_tx.mem.buffer.buffer		= (uint8_t *)&app.vsfip.telnetd.txbuff,
	.vsfip.telnetd.stream_tx.mem.buffer.size		= sizeof(app.vsfip.telnetd.txbuff),
	.vsfip.telnetd.stream_rx.stream.op				= &fifostream_op,
	.vsfip.telnetd.stream_rx.mem.buffer.buffer		= (uint8_t *)&app.vsfip.telnetd.rxbuff,
	.vsfip.telnetd.stream_rx.mem.buffer.size		= sizeof(app.vsfip.telnetd.rxbuff),

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
//	.shell.echo								= true,
//	.shell.stream_tx						= (struct vsf_stream_t *)&app.usbd.cdc.stream_tx,
//	.shell.stream_rx						= (struct vsf_stream_t *)&app.usbd.cdc.stream_rx,
	.shell.echo								= false,
	.shell.stream_tx						= (struct vsf_stream_t *)&app.vsfip.telnetd.stream_tx,
	.shell.stream_rx						= (struct vsf_stream_t *)&app.vsfip.telnetd.stream_rx,
#endif

	.sm.init_state.evt_handler				= app_evt_handler,

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

// vsfip buffer manager
static struct vsfip_buffer_t* app_vsfip_get_buffer(uint32_t size)
{
	return VSFPOOL_ALLOC(&app.vsfip.buffer_pool, struct vsfip_buffer_t);
}

static void app_vsfip_release_buffer(struct vsfip_buffer_t *buffer)
{
	VSFPOOL_FREE(&app.vsfip.buffer_pool, buffer);
}

static struct vsfip_socket_t* app_vsfip_get_socket(void)
{
	return VSFPOOL_ALLOC(&app.vsfip.socket_pool, struct vsfip_socket_t);
}

static void app_vsfip_release_socket(struct vsfip_socket_t *socket)
{
	VSFPOOL_FREE(&app.vsfip.socket_pool, socket);
}

static struct vsfip_tcppcb_t* app_vsfip_get_tcppcb(void)
{
	return VSFPOOL_ALLOC(&app.vsfip.tcppcb_pool, struct vsfip_tcppcb_t);
}

static void app_vsfip_release_tcppcb(struct vsfip_tcppcb_t *tcppcb)
{
	VSFPOOL_FREE(&app.vsfip.tcppcb_pool, tcppcb);
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

// rndis on_connect
void app_rndis_on_connect(void *param)
{
	struct vsfapp_t *app = (struct vsfapp_t *)param;
	vsfip_dhcpd_start(&app->usbd.rndis.param.netif, &app->usbd.rndis.dhcpd);
}

static struct vsfsm_state_t *
app_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	switch (evt)
	{
	case VSFSM_EVT_INIT:
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

			// tester
			app.caller_pt.state = 0;
			vsfile_getfile(&app.caller_pt, 0, NULL, "/msc_root/Driver/Windows/VSFRNDIS.inf", &file);
		}

		// vsfip init
		{
			struct vsfip_buffer_t *buffer;
			int i;

			buffer = &app.vsfip.buffer_pool.buffer[0];
			for (i = 0; i < APPCFG_VSFIP_BUFFER_NUM; i++)
			{
				buffer->buffer = app.vsfip.buffer_mem[i];
				buffer++;
			}
		}
		VSFPOOL_INIT(&app.vsfip.buffer_pool, struct vsfip_buffer_t, APPCFG_VSFIP_BUFFER_NUM);
		VSFPOOL_INIT(&app.vsfip.socket_pool, struct vsfip_socket_t, APPCFG_VSFIP_SOCKET_NUM);
		VSFPOOL_INIT(&app.vsfip.tcppcb_pool, struct vsfip_tcppcb_t, APPCFG_VSFIP_TCPPCB_NUM);
		vsfip_init((struct vsfip_mem_op_t *)&app_vsfip_mem_op);

		// telnet stream innit
		STREAM_INIT(&app.vsfip.telnetd.stream_rx);
		STREAM_INIT(&app.vsfip.telnetd.stream_tx);
		vsfip_telnetd_start(&app.vsfip.telnetd.telnetd);
	
		// usbd cdc init
		STREAM_INIT(&app.usbd.cdc.stream_rx);
		STREAM_INIT(&app.usbd.cdc.stream_tx);

		vsfscsi_init(&app.mal.scsi_dev);
		vsfusbd_device_init(&app.usbd.device);

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
		vsfshell_init(&app.shell);
#endif

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
			vsfhal_gpio_set(app.usb_pullup.port,
										1 << app.usb_pullup.pin);
		}
		app.usbd.device.drv->connect();
		break;
	}
	return NULL;
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
	vsfsm_init(&app.sm);

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
