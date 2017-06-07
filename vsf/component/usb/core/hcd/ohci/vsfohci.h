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
#ifndef __VSFOHCI_H___
#define __VSFOHCI_H___

struct vsfohci_hcd_param_t
{
	uint32_t index;
};

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFOHCI_MODNAME						"vsf.stack.usb.host.hcd.ohci"

struct vsfohci_modifs_t
{
	struct vsfusbh_hcddrv_t drv;
};

vsf_err_t vsfohci_modexit(struct vsf_module_t*);
vsf_err_t vsfohci_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFOHCICMOD							\
	((struct vsfohci_modifs_t *)vsf_module_load(VSFOHCI_MODNAME, true))
#define vsfohci_drv							VSFOHCICMOD->drv

#else
extern const struct vsfusbh_hcddrv_t vsfohci_drv;
#endif

#endif // __VSFOHCI_H___
