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

#ifndef __VSF_MALFS_H_INCLUDED__
#define __VSF_MALFS_H_INCLUDED__

#define VSF_MALFS_EVT_CRIT				(VSFSM_EVT_USER + 0)
#define VSF_MALFS_EVT_IODONE			(VSFSM_EVT_USER + 1)
#define VSF_MALFS_EVT_IOFAIL			(VSFSM_EVT_USER + 2)

struct vsf_malfs_t
{
	struct vsf_malstream_t malstream;

	// protected
	char *volume_name;
	struct vsfsm_crit_t crit;
	uint8_t *sector_buffer;
	struct vsfsm_t *notifier_sm;

	// private
	struct vsf_mbufstream_t mbufstream;
	uint8_t *mbufstream_buffer[1];
};

#ifdef VSFCFG_STANDALONE_MODULE
#define VSF_MALFS_MODNAME					"vsf.component.file.fs.malfs"

struct vsf_malfs_modifs_t
{
	vsf_err_t (*init)(struct vsf_malfs_t*);
	void (*fini)(struct vsf_malfs_t*);
	vsf_err_t (*read)(struct vsf_malfs_t*, uint32_t, uint8_t*, uint32_t);
	vsf_err_t (*write)(struct vsf_malfs_t*, uint32_t, uint8_t*, uint32_t);
};

vsf_err_t vsf_malfs_modexit(struct vsf_module_t*);
vsf_err_t vsf_malfs_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSF_MALFS_MOD						\
	((struct vsf_malfs_modifs_t *)vsf_module_load(VSF_MALFS_MODNAME, true))
#define vsf_malfs_init						VSF_MALFS_MOD->init
#define vsf_malfs_fini						VSF_MALFS_MOD->fini
#define vsf_malfs_read						VSF_MALFS_MOD->read
#define vsf_malfs_write						VSF_MALFS_MOD->write

#else
vsf_err_t vsf_malfs_init(struct vsf_malfs_t *malfs);
void vsf_malfs_fini(struct vsf_malfs_t *malfs);

vsf_err_t vsf_malfs_read(struct vsf_malfs_t *malfs, uint32_t sector,
							uint8_t *buff, uint32_t num);
vsf_err_t vsf_malfs_write(struct vsf_malfs_t *malfs, uint32_t sector,
							uint8_t *buff, uint32_t num);
#endif

#endif	// __VSF_MALFS_H_INCLUDED__
