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

#ifndef __VSFUSBH_HUB_H_INCLUDED__
#define __VSFUSBH_HUB_H_INCLUDED__

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFUSBH_HUB_MODNAME					"vsf.stack.usb.host.hub"

struct vsfusbh_hub_modifs_t
{
	struct vsfusbh_class_drv_t drv;
};

vsf_err_t vsfusbh_hub_modexit(struct vsf_module_t*);
vsf_err_t vsfusbh_hub_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFUSBH_HUBCMOD						\
	((struct vsfusbh_hub_modifs_t *)vsf_module_load(VSFUSBH_HUB_MODNAME, true))
#define vsfusbh_hub_drv						VSFUSBH_HUBCMOD->drv

#else
extern const struct vsfusbh_class_drv_t vsfusbh_hub_drv;
#endif

#endif
