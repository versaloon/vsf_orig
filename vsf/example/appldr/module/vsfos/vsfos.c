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
#include "vsfos.h"
#include "fakefat32_fs.h"
#include "usbd_desc.h"
#include "vsfos_busybox.h"

// event queue
static void vsfos_pendsv_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_core_pendsv_trigger();
}

static void vsfos_on_pendsv(void *param)
{
	struct vsfsm_evtq_t *evtq_cur = param, *evtq_old = vsfsm_evtq_set(evtq_cur);

	while (vsfsm_get_event_pending())
	{
		vsfsm_poll();
	}
	vsfsm_evtq_set(evtq_old);
}

// vsftimer
static void vsfos_tickclk_callback_int(void *param)
{
	vsftimer_callback_int();
}

static struct vsftimer_t* vsftimer_memop_alloc(void)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	return VSFPOOL_ALLOC(&ifs->vsftimer_pool, struct vsftimer_t);
}

static void vsftimer_memop_free(struct vsftimer_t *timer)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	VSFPOOL_FREE(&ifs->vsftimer_pool, timer);
}

// vsfip buffer manager
static struct vsfip_buffer_t* vsfos_vsfip_get_buffer(uint32_t size)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	return VSFPOOL_ALLOC(&ifs->tcpip.buffer_pool, struct vsfip_buffer_t);
}

static void vsfos_vsfip_release_buffer(struct vsfip_buffer_t *buffer)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	VSFPOOL_FREE(&ifs->tcpip.buffer_pool, buffer);
}

static struct vsfip_socket_t* vsfos_vsfip_get_socket(void)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	return VSFPOOL_ALLOC(&ifs->tcpip.socket_pool, struct vsfip_socket_t);
}

static void vsfos_vsfip_release_socket(struct vsfip_socket_t *socket)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	VSFPOOL_FREE(&ifs->tcpip.socket_pool, socket);
}

static struct vsfip_tcppcb_t* vsfos_vsfip_get_tcppcb(void)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	return VSFPOOL_ALLOC(&ifs->tcpip.tcppcb_pool, struct vsfip_tcppcb_t);
}

static void vsfos_vsfip_release_tcppcb(struct vsfip_tcppcb_t *tcppcb)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	VSFPOOL_FREE(&ifs->tcpip.tcppcb_pool, tcppcb);
}

// vsfile_memop
struct vsfile_vfsfile_t* vsfos_vsfile_alloc_vfs(void)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	return VSFPOOL_ALLOC(&ifs->vfsfile_pool, struct vsfile_vfsfile_t);
}

static void vsfos_vsfile_free_vfs(struct vsfile_vfsfile_t *vfsfile)
{
	struct vsf_module_t *module = vsf_module_get("vsf.os");
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)module->ifs;
	VSFPOOL_FREE(&ifs->vfsfile_pool, vfsfile);
}

// rndis
void vsfos_rndis_on_connect(void *param)
{
	struct vsfos_rndis_t *rndis = (struct vsfos_rndis_t *)param;

	vsfsm_sem_init(&rndis->dhcpc.update_sem, 0, VSFSM_EVT_USER);
	vsfip_dhcpc_start(&rndis->param.netif, &rndis->dhcpc);
}

// vsfos
static vsf_err_t vsfos_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfos_modifs_t *ifs = (struct vsfos_modifs_t *)pt->user_data;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	vsfhal_core_init(NULL);
	vsfhal_tickclk_init();
	vsfhal_tickclk_start();

	// timer
	VSFPOOL_INIT(&ifs->vsftimer_pool, struct vsftimer_t, VSFOSCFG_VSFSM_PENDSVQ_LEN);
	vsftimer_init(&ifs->vsftimer_memop);
	vsfhal_tickclk_config_cb(vsfos_tickclk_callback_int, NULL);

	// file
	VSFPOOL_INIT(&ifs->vfsfile_pool, struct vsfile_vfsfile_t, VSFOSCFG_VFS_NO);
	vsfile_init(&ifs->fs.memop);

	ifs->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_addfile(&ifs->caller_pt, evt, NULL, "msc_root", VSFILE_ATTR_DIRECTORY);
	if (err != 0) return err;

	ifs->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_getfile(&ifs->caller_pt, evt, NULL, "/msc_root", &ifs->file);
	if (err != 0) return err;

	ifs->caller_pt.state = 0;
	ifs->caller_pt.user_data = &ifs->mal.fakefat32;
	err = vsfile_mount(&ifs->caller_pt, evt, (struct vsfile_fsop_t *)&fakefat32_fs_op, ifs->file);
	if (err != 0) return err;

	// vsfip
	{
		struct vsfip_buffer_t *buffer;
		int i;

		buffer = &ifs->tcpip.buffer_pool.buffer[0];
		for (i = 0; i < VSFOSCFG_VSFIP_BUFFER_NUM; i++)
		{
			buffer->buffer = ifs->tcpip.buffer_mem[i];
			buffer++;
		}
	}
	VSFPOOL_INIT(&ifs->tcpip.buffer_pool, struct vsfip_buffer_t, VSFOSCFG_VSFIP_BUFFER_NUM);
	VSFPOOL_INIT(&ifs->tcpip.socket_pool, struct vsfip_socket_t, VSFOSCFG_VSFIP_SOCKET_NUM);
	VSFPOOL_INIT(&ifs->tcpip.tcppcb_pool, struct vsfip_tcppcb_t, VSFOSCFG_VSFIP_TCPPCB_NUM);
	vsfip_init((struct vsfip_mem_op_t *)&ifs->tcpip.mem_op);

	STREAM_INIT(&ifs->usbd.cdc.stream_rx);
	STREAM_INIT(&ifs->usbd.cdc.stream_tx);

	vsfscsi_init(&ifs->mal.scsi_dev);
	vsfusbd_device_init(&ifs->usbd.device);
	vsfshell_init(&ifs->shell);
	vsfos_busybox_init(&ifs->shell);

	if ((ifs->hwcfg->usbd.pullup.port != IFS_DUMMY_PORT) && (vsfhal_gpio_if != NULL))
	{
		uint8_t port = ifs->hwcfg->usbd.pullup.port;
		uint8_t pin = ifs->hwcfg->usbd.pullup.pin;
		vsfhal_gpio_init(port);
		vsfhal_gpio_clear(port, 1 << pin);
		vsfhal_gpio_config_pin(port, pin, GPIO_OUTPP);
	}
	ifs->usbd.device.drv->disconnect();

	vsfsm_pt_delay(pt, 200);

	if ((ifs->hwcfg->usbd.pullup.port != IFS_DUMMY_PORT) && (vsfhal_gpio_if != NULL))
	{
		uint8_t port = ifs->hwcfg->usbd.pullup.port;
		uint8_t pin = ifs->hwcfg->usbd.pullup.pin;
		vsfhal_gpio_set(port, 1 << pin);
	}
	ifs->usbd.device.drv->connect();

	while (1)
	{
		vsfsm_pt_delay(pt, 1000);
		asm("nop");
		vsfsm_pt_delay(pt, 1000);
		asm("nop");
		vsfsm_pt_delay(pt, 1000);
		asm("nop");
		vsfsm_pt_delay(pt, 1000);
		asm("nop");
		vsfsm_pt_delay(pt, 1000);
		asm("nop");
		vsfsm_pt_delay(pt, 1000);
		asm("nop");
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsfos_modexit(struct vsf_module_t *module)
{
	vsf_bufmgr_free(module->ifs);
	module->ifs = NULL;
	return VSFERR_NONE;
}

vsf_err_t vsfos_modinit(struct vsf_module_t *module,
								struct app_hwcfg_t const *cfg)
{
	struct vsfos_modifs_t *ifs;
	ifs = vsf_bufmgr_malloc(sizeof(struct vsfos_modifs_t));
	if (!ifs) return VSFERR_FAIL;
	memset(ifs, 0, sizeof(*ifs));

	ifs->hwcfg = cfg;

	ifs->pendsvq.size = dimof(ifs->pendsvq_ele);
	ifs->pendsvq.queue = ifs->pendsvq_ele;
	ifs->pendsvq.activate = vsfos_pendsv_activate;

	// init vsftimer
	ifs->vsftimer_memop.alloc = vsftimer_memop_alloc;
	ifs->vsftimer_memop.free = vsftimer_memop_free;

	// init fakefat32_fs
	ifs->fakefat32_fs.Windows_dir[0].memfile.file.name = ".";
	ifs->fakefat32_fs.Windows_dir[0].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.Windows_dir[1].memfile.file.name = "..";
	ifs->fakefat32_fs.Windows_dir[1].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.Windows_dir[2].memfile.file.name = "VSFCDC.inf";
	ifs->fakefat32_fs.Windows_dir[2].memfile.file.size = sizeof(vsfcdc_inf) - 1;
	ifs->fakefat32_fs.Windows_dir[2].memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_READONLY;
	ifs->fakefat32_fs.Windows_dir[2].memfile.f.buff = (uint8_t *)vsfcdc_inf;
	ifs->fakefat32_fs.Windows_dir[3].memfile.file.name = "VSFRNDIS.inf";
	ifs->fakefat32_fs.Windows_dir[3].memfile.file.size = sizeof(vsfrndis_inf) - 1;
	ifs->fakefat32_fs.Windows_dir[3].memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_READONLY;
	ifs->fakefat32_fs.Windows_dir[3].memfile.f.buff = (uint8_t *)vsfrndis_inf;

	ifs->fakefat32_fs.Driver_dir[0].memfile.file.name = ".";
	ifs->fakefat32_fs.Driver_dir[0].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.Driver_dir[1].memfile.file.name = "..";
	ifs->fakefat32_fs.Driver_dir[1].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.Driver_dir[2].memfile.file.name = "Windows";
	ifs->fakefat32_fs.Driver_dir[2].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.Driver_dir[2].memfile.d.child = (struct vsfile_memfile_t *)ifs->fakefat32_fs.Windows_dir;

	ifs->fakefat32_fs.LOSTDIR_dir[0].memfile.file.name = ".";
	ifs->fakefat32_fs.LOSTDIR_dir[0].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.LOSTDIR_dir[1].memfile.file.name = "..";
	ifs->fakefat32_fs.LOSTDIR_dir[1].memfile.file.attr = VSFILE_ATTR_DIRECTORY;

	ifs->fakefat32_fs.RECYCLEBIN_dir[0].memfile.file.name = ".";
	ifs->fakefat32_fs.RECYCLEBIN_dir[0].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.RECYCLEBIN_dir[1].memfile.file.name = "..";
	ifs->fakefat32_fs.RECYCLEBIN_dir[1].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.RECYCLEBIN_dir[2].memfile.file.name = "DESKTOP.INI";
	ifs->fakefat32_fs.RECYCLEBIN_dir[2].memfile.file.size = sizeof(Win_recycle_DESKTOP_INI) - 1;
	ifs->fakefat32_fs.RECYCLEBIN_dir[2].memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_READONLY;
	ifs->fakefat32_fs.RECYCLEBIN_dir[2].memfile.f.buff = (uint8_t *)Win_recycle_DESKTOP_INI;

	ifs->fakefat32_fs.SVI_dir[0].memfile.file.name = ".";
	ifs->fakefat32_fs.SVI_dir[0].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.SVI_dir[1].memfile.file.name = "..";
	ifs->fakefat32_fs.SVI_dir[1].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.SVI_dir[2].memfile.file.name = "IndexerVolumeGuid";
	ifs->fakefat32_fs.SVI_dir[2].memfile.file.size = sizeof(Win10_IndexerVolumeGuid);
	ifs->fakefat32_fs.SVI_dir[2].memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_READONLY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN,
	ifs->fakefat32_fs.SVI_dir[2].memfile.f.buff = (uint8_t *)Win10_IndexerVolumeGuid;
	ifs->fakefat32_fs.SVI_dir[3].memfile.file.name = "WPSettings.dat";
	ifs->fakefat32_fs.SVI_dir[3].memfile.file.size = sizeof(Win10_WPSettings_dat);
	ifs->fakefat32_fs.SVI_dir[3].memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_READONLY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN,
	ifs->fakefat32_fs.SVI_dir[3].memfile.f.buff = (uint8_t *)Win10_WPSettings_dat;

	ifs->fakefat32_fs.root_dir[0].memfile.file.name = "VSFDriver";
	ifs->fakefat32_fs.root_dir[0].memfile.file.attr = VSFILE_ATTR_VOLUMID;
	ifs->fakefat32_fs.root_dir[1].memfile.file.name = "LOST.DIR";
	ifs->fakefat32_fs.root_dir[1].memfile.file.attr = VSFILE_ATTR_DIRECTORY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN;
	ifs->fakefat32_fs.root_dir[1].memfile.d.child = (struct vsfile_memfile_t *)ifs->fakefat32_fs.LOSTDIR_dir;
	ifs->fakefat32_fs.root_dir[2].memfile.file.name = "$RECYCLE.BIN";
	ifs->fakefat32_fs.root_dir[2].memfile.file.attr = VSFILE_ATTR_DIRECTORY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN;
	ifs->fakefat32_fs.root_dir[2].memfile.d.child = (struct vsfile_memfile_t *)ifs->fakefat32_fs.RECYCLEBIN_dir;
	ifs->fakefat32_fs.root_dir[3].memfile.file.name = "System Volume Information";
	ifs->fakefat32_fs.root_dir[3].memfile.file.attr = VSFILE_ATTR_DIRECTORY | VSFILE_ATTR_SYSTEM | VSFILE_ATTR_HIDDEN;
	ifs->fakefat32_fs.root_dir[3].memfile.d.child = (struct vsfile_memfile_t *)ifs->fakefat32_fs.SVI_dir;
	ifs->fakefat32_fs.root_dir[4].memfile.file.name = "Driver";
	ifs->fakefat32_fs.root_dir[4].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
	ifs->fakefat32_fs.root_dir[4].memfile.d.child = (struct vsfile_memfile_t *)ifs->fakefat32_fs.Driver_dir;

	// init fakefat32
	ifs->mal.fakefat32.sector_size = 512;
	ifs->mal.fakefat32.sector_number = 0x1000;
	ifs->mal.fakefat32.sectors_per_cluster = 8;
	ifs->mal.fakefat32.volume_id = 0xDEADBEEF;
	ifs->mal.fakefat32.disk_id = 0xBABECAFE;
	ifs->mal.fakefat32.root[0].memfile.file.name = "ROOT";
	ifs->mal.fakefat32.root[0].memfile.d.child = (struct vsfile_memfile_t *)ifs->fakefat32_fs.root_dir,

	// init mal
	ifs->mal.mal.drv = &fakefat32_mal_drv;
	ifs->mal.mal.param = &ifs->mal.fakefat32;
	ifs->mal.pbuffer[0] = ifs->mal.buffer[0];
	ifs->mal.pbuffer[1] = ifs->mal.buffer[1];

	// init file
	ifs->fs.memop.alloc_vfs = vsfos_vsfile_alloc_vfs;
	ifs->fs.memop.free_vfs = vsfos_vsfile_free_vfs;

	// init mal2scsi
	ifs->mal.scsistream.mbuf.count = dimof(ifs->mal.buffer);
	ifs->mal.scsistream.mbuf.size = sizeof(ifs->mal.buffer[0]);
	ifs->mal.scsistream.mbuf.buffer_list = ifs->mal.pbuffer;
	ifs->mal.mal2scsi.malstream.mal = &ifs->mal.mal;
	ifs->mal.mal2scsi.cparam.block_size = 512;
	ifs->mal.mal2scsi.cparam.removable = false;
	memcpy(ifs->mal.mal2scsi.cparam.vendor, "Simon   ", 8);
	memcpy(ifs->mal.mal2scsi.cparam.product, "VSFDriver       ", 16);
	memcpy(ifs->mal.mal2scsi.cparam.revision, "1.00", 4);
	ifs->mal.mal2scsi.cparam.type = SCSI_PDT_DIRECT_ACCESS_BLOCK;

	// init scsi device
	ifs->mal.lun[0].op = &vsf_mal2scsi_op;
	ifs->mal.lun[0].stream = (struct vsf_stream_t *)&ifs->mal.scsistream;
	ifs->mal.lun[0].param = &ifs->mal.mal2scsi;
	ifs->mal.scsi_dev.max_lun = 0;
	ifs->mal.scsi_dev.lun = ifs->mal.lun;

	// init rndis
	ifs->usbd.rndis.param.CDCACM.CDC.ep_notify = 1;
	ifs->usbd.rndis.param.CDCACM.CDC.ep_out = 2;
	ifs->usbd.rndis.param.CDCACM.CDC.ep_in = 2;
	ifs->usbd.rndis.param.mac.size = 6;
	ifs->usbd.rndis.param.mac.addr.s_addr64 = 0x0605040302E0;
	ifs->usbd.rndis.param.netif.macaddr.size = 6;
	ifs->usbd.rndis.param.netif.macaddr.addr.s_addr64 = 0x0E0D0C0B0AE0;
	ifs->usbd.rndis.param.cb.param = &ifs->usbd.rndis;
	ifs->usbd.rndis.param.cb.on_connect = vsfos_rndis_on_connect;
	if (0)
	{
		ifs->usbd.rndis.param.netif.ipaddr.size = 4;
		ifs->usbd.rndis.param.netif.ipaddr.addr.s_addr = 0x01202020;
		ifs->usbd.rndis.param.netif.netmask.size = 4;
		ifs->usbd.rndis.param.netif.netmask.addr.s_addr = 0x00FFFFFF;
		ifs->usbd.rndis.param.netif.gateway.size = 4;
		ifs->usbd.rndis.param.netif.gateway.addr.s_addr = 0x01202020;
	}

	// init cdc
	ifs->usbd.cdc.stream_tx.stream.op = &fifostream_op;
	ifs->usbd.cdc.stream_tx.mem.buffer.buffer = ifs->usbd.cdc.txbuff;
	ifs->usbd.cdc.stream_tx.mem.buffer.size = sizeof(ifs->usbd.cdc.txbuff);
	ifs->usbd.cdc.stream_rx.stream.op = &fifostream_op;
	ifs->usbd.cdc.stream_rx.mem.buffer.buffer = ifs->usbd.cdc.rxbuff;
	ifs->usbd.cdc.stream_rx.mem.buffer.size = sizeof(ifs->usbd.cdc.rxbuff);
	ifs->usbd.cdc.param.CDC.ep_notify = 3;
	ifs->usbd.cdc.param.CDC.ep_out = 4;
	ifs->usbd.cdc.param.CDC.ep_in = 4;
	ifs->usbd.cdc.param.CDC.stream_tx = (struct vsf_stream_t *)&ifs->usbd.cdc.stream_tx;
	ifs->usbd.cdc.param.CDC.stream_rx = (struct vsf_stream_t *)&ifs->usbd.cdc.stream_rx;
	ifs->usbd.cdc.param.line_coding.bitrate = 115200;
	ifs->usbd.cdc.param.line_coding.stopbittype = 0;
	ifs->usbd.cdc.param.line_coding.paritytype = 0;
	ifs->usbd.cdc.param.line_coding.datatype = 8;

	// init msc
	ifs->usbd.msc.param.ep_in = 5;
	ifs->usbd.msc.param.ep_out = 5;
	ifs->usbd.msc.param.scsi_dev = &ifs->mal.scsi_dev;

	// init usbd
	ifs->usbd.desc[0] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_DEVICE(0, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor));
	ifs->usbd.desc[1] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_CONFIG(0, 0, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor));
	ifs->usbd.desc[2] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_STRING(0, 0, USB_StringLangID, sizeof(USB_StringLangID));
	ifs->usbd.desc[3] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_STRING(0x0409, 1, USB_StringVendor, sizeof(USB_StringVendor));
	ifs->usbd.desc[4] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_STRING(0x0409, 2, USB_StringProduct, sizeof(USB_StringProduct));
	ifs->usbd.desc[5] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_STRING(0x0409, 3, USB_StringSerial, sizeof(USB_StringSerial));
	ifs->usbd.desc[6] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_STRING(0x0409, 4, RNDIS_StringFunc, sizeof(RNDIS_StringFunc));
	ifs->usbd.desc[7] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_STRING(0x0409, 5, CDC_StringFunc, sizeof(CDC_StringFunc));
	ifs->usbd.desc[8] = (struct vsfusbd_desc_filter_t)VSFUSBD_DESC_STRING(0x0409, 6, MSC_StringFunc, sizeof(MSC_StringFunc));
	ifs->usbd.ifaces[0].class_protocol = (struct vsfusbd_class_protocol_t *)&vsfusbd_RNDISControl_class;
	ifs->usbd.ifaces[0].protocol_param = &ifs->usbd.rndis.param;
	ifs->usbd.ifaces[1].class_protocol = (struct vsfusbd_class_protocol_t *)&vsfusbd_RNDISData_class;
	ifs->usbd.ifaces[1].protocol_param = &ifs->usbd.rndis.param;
	ifs->usbd.ifaces[2].class_protocol = (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class;
	ifs->usbd.ifaces[2].protocol_param = &ifs->usbd.cdc.param;
	ifs->usbd.ifaces[3].class_protocol = (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class;
	ifs->usbd.ifaces[3].protocol_param = &ifs->usbd.cdc.param;
	ifs->usbd.ifaces[4].class_protocol = (struct vsfusbd_class_protocol_t *)&vsfusbd_MSCBOT_class;
	ifs->usbd.ifaces[4].protocol_param = &ifs->usbd.msc.param;
	ifs->usbd.config[0].num_of_ifaces = dimof(ifs->usbd.ifaces);
	ifs->usbd.config[0].iface = ifs->usbd.ifaces;
	ifs->usbd.device.num_of_configuration = dimof(ifs->usbd.config);
	ifs->usbd.device.config = ifs->usbd.config;
	ifs->usbd.device.desc_filter = ifs->usbd.desc;
	ifs->usbd.device.device_class_iface = 0;
	ifs->usbd.device.drv = vsfhal_usbd_if;
	ifs->usbd.device.int_priority = 0x01;

	// init telnetd
	ifs->tcpip.telnetd.telnetd.port = 23;
	ifs->tcpip.telnetd.telnetd.session_num = dimof(ifs->tcpip.telnetd.sessions);
	ifs->tcpip.telnetd.sessions[0].session.stream_tx = (struct vsf_stream_t *)&ifs->tcpip.telnetd.sessions[0].stream_tx;
	ifs->tcpip.telnetd.sessions[0].session.stream_rx = (struct vsf_stream_t *)&ifs->tcpip.telnetd.sessions[0].stream_rx;
	ifs->tcpip.telnetd.sessions[0].stream_tx.stream.op = &fifostream_op;
	ifs->tcpip.telnetd.sessions[0].stream_tx.mem.buffer.buffer = (uint8_t *)ifs->tcpip.telnetd.sessions[0].txbuff;
	ifs->tcpip.telnetd.sessions[0].stream_tx.mem.buffer.size = sizeof(ifs->tcpip.telnetd.sessions[0].txbuff);
	ifs->tcpip.telnetd.sessions[0].stream_rx.stream.op = &fifostream_op;
	ifs->tcpip.telnetd.sessions[0].stream_rx.mem.buffer.buffer = (uint8_t *)ifs->tcpip.telnetd.sessions[0].rxbuff;
	ifs->tcpip.telnetd.sessions[0].stream_rx.mem.buffer.size = sizeof(ifs->tcpip.telnetd.sessions[0].rxbuff);

	// init tcpip
	ifs->tcpip.mem_op.get_buffer = vsfos_vsfip_get_buffer;
	ifs->tcpip.mem_op.release_buffer = vsfos_vsfip_release_buffer;
	ifs->tcpip.mem_op.get_socket = vsfos_vsfip_get_socket;
	ifs->tcpip.mem_op.release_socket = vsfos_vsfip_release_socket;
	ifs->tcpip.mem_op.get_tcppcb = vsfos_vsfip_get_tcppcb;
	ifs->tcpip.mem_op.release_tcppcb = vsfos_vsfip_release_tcppcb;

	// init shell
	ifs->shell.echo = true;
	ifs->shell.stream_tx = (struct vsf_stream_t *)&ifs->usbd.cdc.stream_tx;
	ifs->shell.stream_rx = (struct vsf_stream_t *)&ifs->usbd.cdc.stream_rx;

	ifs->pt.user_data = ifs;
	ifs->pt.thread = vsfos_thread;
	module->ifs = ifs;

	vsfsm_evtq_init(&ifs->pendsvq);
	vsfsm_evtq_set(&ifs->pendsvq);
	vsfsm_pt_init(&ifs->sm, &ifs->pt);

	vsfhal_core_pendsv_config(vsfos_on_pendsv, &ifs->pendsvq);
	vsf_leave_critical();

	vsfsm_evtq_set(NULL);
	while (1)
	{
		// no thread runs in mainq, just sleep in main loop
		vsfhal_core_sleep(0);
	}
}
