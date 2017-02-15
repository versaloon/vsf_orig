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

#ifndef __EMBFLASH_H_INCLUDED__
#define __EMBFLASH_H_INCLUDED__

struct embflash_mal_t
{
	struct vsfmal_t mal;
	uint8_t index;
	uint32_t int_priority;

	// private
	vsf_err_t err;
	uint32_t cursize;
	uint32_t pagesize;
	struct vsfsm_t *notifier;
	struct vsfsm_pt_t pt;
};

#ifdef VSFCFG_STANDALONE_MODULE
#define EMBFLASH_MODNAME					"vsf.tool.embflash"

struct embflash_modifs_t
{
	struct
	{
		struct vsfmal_drv_t drv;
	} mal;
};

vsf_err_t embflash_modexit(struct vsf_module_t*);
vsf_err_t embflash_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define EMBFLASH_MOD						\
	((struct embflash_modifs_t *)vsf_module_load(EMBFLASH_MODNAME, true))
#define embflash_mal_drv					EMBFLASH_MOD->mal.drv

#else
extern struct vsfmal_drv_t embflash_mal_drv;
#endif

#endif	// __EMBFLASH_H_INCLUDED__
