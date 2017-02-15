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

#ifndef __VSFUSBD_RNDIS_H_INCLUDED__
#define __VSFUSBD_RNDIS_H_INCLUDED__

#ifndef VSFUSBD_CDCCFG_TRANSACT
#error "vsfusbd_RNDIS need vsfusbd_CDC run in TRANSACT mode"
#endif

#if VSFIP_CFG_NETIF_HEADLEN < 64
#error "RNDIS require minimum 64 bytes netif header size"
#endif

#include "../../common/CDC/vsfusb_RNDIS.h"

#ifndef VSFUSBD_RNDIS_CFG_OIDNUM
#define VSFUSBD_RNDIS_CFG_OIDNUM				25
#endif

struct vsfusbd_RNDIS_param_t
{
	struct vsfusbd_CDCACM_param_t CDCACM;
	struct vsfip_macaddr_t mac;
	struct
	{
		void *param;
		void (*on_connect)(void *param);
	} cb;

	// private
	uint8_t encapsulated_buf[4 * VSFUSBD_RNDIS_CFG_OIDNUM + 32];
	struct vsfip_netif_t netif;
	struct vsfip_netdrv_t netdrv;
	bool netif_inited;
	struct vsf_bufstream_t tx_bufstream;
	struct vsf_bufstream_t rx_bufstream;
	struct vsfip_buffer_t *tx_buffer;
	struct vsfip_buffer_t *rx_buffer;

	// thread to receive netif.output_sem
	struct vsfsm_t sm;

	struct
	{
		uint32_t txok;
		uint32_t rxok;
		uint32_t txbad;
		uint32_t rxbad;
		uint32_t rx_nobuf;
	} statistics;
	uint32_t oid_packet_filter;
};

#define vsfusbd_RNDISControl_class			vsfusbd_CDCACMControl_class

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFUSBD_RNDIS_MODNAME				"vsf.stack.usb.device.classes.rndis"

struct vsfusbd_RNDIS_modifs_t
{
	struct vsfusbd_class_protocol_t data_protocol;
	struct vsfip_netdrv_op_t netdrv_op;
};

vsf_err_t vsfusbd_RNDIS_modexit(struct vsf_module_t*);
vsf_err_t vsfusbd_RNDIS_modinit(struct vsf_module_t*,
										struct app_hwcfg_t const*);

#define VSFUSBD_RNDISMOD					\
	((struct vsfusbd_RNDIS_modifs_t *)vsf_module_load(VSFUSBD_RNDIS_MODNAME, true))
#define vsfusbd_RNDISData_class				VSFUSBD_RNDISMOD->data_protocol
#define vsfusbd_RNDIS_netdrv_op				VSFUSBD_RNDISMOD->netdrv_op

#else
extern const struct vsfusbd_class_protocol_t vsfusbd_RNDISData_class;
#endif

#endif	// __VSFUSBD_RNDIS_H_INCLUDED__
