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

#include "app_cfg.h"
#include "app_type.h"
#include "vsfhal.h"

#include "cmem7.h"

#if VSFHAL_HCD_EN

#define CMEM7_USB_NUM			1

static vsf_err_t (*irq0)(void*);
static void *irq0_param;
static vsf_err_t (*irq1)(void*);
static void *irq1_param;

ROOTFUNC void OTG_HS_IRQHandler(void)
{
	if (irq0 != NULL)
		irq0(irq0_param);
}

ROOTFUNC void OTG_FS_IRQHandler(void)
{
	if(irq1 != NULL)
		irq1(irq1_param);
}

vsf_err_t cmem7_hcd_init(uint32_t index, vsf_err_t (*irq)(void *), void *param)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= CMEM7_USB_NUM)
		return VSFERR_NOT_SUPPORT;


	return VSFERR_NONE;
}

vsf_err_t cmem7_hcd_fini(uint32_t index)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= CMEM7_USB_NUM)
		return VSFERR_NOT_SUPPORT;


	return VSFERR_NONE;
}


void* cmem7_hcd_regbase(uint32_t index)
{
	switch (index >> 16)
	{
	default:
		return NULL;
	}
}

#endif // VSFHAL_HCD_EN


#if VSFHAL_USB_DCD_EN

#endif // VSFHAL_HCD_DCD_EN

