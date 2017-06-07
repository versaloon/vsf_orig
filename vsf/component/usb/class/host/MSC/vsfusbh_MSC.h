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

#ifndef __VSFUSBH_MSC_H_INCLUDED__
#define __VSFUSBH_MSC_H_INCLUDED__

#include "../../common/MSC/vsfusb_MSC.h"

struct vsfusbh_msc_t
{
	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;

	struct vsfusbh_t *usbh;
	struct vsfusbh_device_t *dev;
	struct usb_interface_t *interface;

	struct vsfusbh_urb_t *urb;
	struct vsfscsi_device_t *scsi_dev;
	uint8_t max_lun;
	uint8_t ep_out;
	uint8_t ep_in;
	uint8_t iface;

	uint8_t executing;
	uint32_t cur_size;
	uint8_t *cur_ptr;
	struct vsfsm_t *inout_notifier;
	struct USBMSC_CBW_t CBW;
	struct USBMSC_CSW_t CSW;
};

struct vsfusbh_msc_global_t
{
	struct vsfscsi_device_t* (*on_new)(uint8_t maxlun,
				struct vsfscsi_lun_op_t *op, void *param);
	void (*on_delete)(struct vsfscsi_device_t *dev);
};

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFUSBH_MSC_MODNAME					"vsf.stack.usb.host.msc"

struct vsfusbh_msc_modifs_t
{
	struct vsfusbh_class_drv_t drv;
	struct vsfscsi_lun_op_t scsi_op;
	struct vsfusbh_msc_global_t global;
};

vsf_err_t vsfusbh_msc_modexit(struct vsf_module_t*);
vsf_err_t vsfusbh_msc_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFUSBH_MSCMOD						\
	((struct vsfusbh_msc_modifs_t *)vsf_module_load(VSFUSBH_MSC_MODNAME, true))
#define vsfusbh_msc_drv						VSFUSBH_MSCMOD->drv
#define vsfscsi_lun_op_t					VSFUSBH_MSCMOD->scsi_op
#define vsfusbh_msc							VSFUSBH_MSCMOD->global

#else
extern const struct vsfusbh_class_drv_t vsfusbh_msc_drv;
extern const struct vsfscsi_lun_op_t vsfusbh_msc_scsi_op;
extern struct vsfusbh_msc_global_t vsfusbh_msc;
#endif

#endif
