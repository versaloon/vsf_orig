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

#ifndef __VSFOS_H_INCLUDED__
#define __VSFOS_H_INCLUDED__

#define VSFOS_MODNAME							"vsf.os"

// config
#define VSFOSCFG_VSFSM_PENDSVQ_LEN				16
#define VSFOSCFG_VSFTIMER_NUM					16

#define VSFOSCFG_VFS_NO							4

#define VSFOSCFG_VSFIP_BUFFER_NUM				16
#define VSFOSCFG_VSFIP_SOCKET_NUM				8
#define VSFOSCFG_VSFIP_TCPPCB_NUM				8

struct vsfos_modifs_t
{
	struct app_hwcfg_t const *hwcfg;

	struct
	{
		struct fakefat32_file_t Windows_dir[5];
		struct fakefat32_file_t Driver_dir[4];
		struct fakefat32_file_t LOSTDIR_dir[3];
		struct fakefat32_file_t RECYCLEBIN_dir[4];
		struct fakefat32_file_t SVI_dir[5];
		struct fakefat32_file_t root_dir[6];
	} fakefat32_fs;

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

	struct
	{
		struct vsfile_memop_t memop;
	} fs;

	struct
	{
		struct vsfos_rndis_t
		{
			struct vsfusbd_RNDIS_param_t param;
			struct vsfip_dhcpc_t dhcpc;
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
		struct vsfusbd_desc_filter_t desc[10];
	} usbd;

	struct
	{
		VSFPOOL_DEFINE(buffer_pool, struct vsfip_buffer_t, VSFOSCFG_VSFIP_BUFFER_NUM);
		VSFPOOL_DEFINE(socket_pool, struct vsfip_socket_t, VSFOSCFG_VSFIP_SOCKET_NUM);
		VSFPOOL_DEFINE(tcppcb_pool, struct vsfip_tcppcb_t, VSFOSCFG_VSFIP_TCPPCB_NUM);
		uint8_t buffer_mem[VSFOSCFG_VSFIP_BUFFER_NUM][VSFIP_BUFFER_SIZE];
		struct vsfip_mem_op_t mem_op;

		struct
		{
			struct vsfip_telnetd_t telnetd;

			struct
			{
				struct vsfip_telnetd_session_t session;
				struct vsf_fifostream_t stream_tx;
				struct vsf_fifostream_t stream_rx;
				uint8_t txbuff[65];
				uint8_t rxbuff[65];
			} sessions[1];
		} telnetd;
	} tcpip;

	struct vsfshell_t shell;

	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;

	VSFPOOL_DEFINE(vfsfile_pool, struct vsfile_vfsfile_t, VSFOSCFG_VFS_NO);

	struct vsftimer_mem_op_t vsftimer_memop;
	VSFPOOL_DEFINE(vsftimer_pool, struct vsftimer_t, VSFOSCFG_VSFTIMER_NUM);
	struct vsfsm_evtq_t pendsvq;
	struct vsfsm_evtq_element_t pendsvq_ele[VSFOSCFG_VSFSM_PENDSVQ_LEN];

	struct vsfile_t *file;
	struct vsfsm_pt_t caller_pt;
};

vsf_err_t vsfos_modexit(struct vsf_module_t*);
vsf_err_t vsfos_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFOS_MOD								\
	((struct vsfos_modifs_t *)vsf_module_load(VSFOS_MODNAME, true))
#define vsfos									VSFOS_MOD

#endif	// __VSFOS_H_INCLUDED__
