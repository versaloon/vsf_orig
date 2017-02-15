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

#ifndef __FAKEFAT32_H_INCLUDED__
#define __FAKEFAT32_H_INCLUDED__

struct fakefat32_file_t;
struct fakefat32_cb_t
{
	vsf_err_t (*read)(struct vsfsm_pt_t*, vsfsm_evt_t,
						struct fakefat32_file_t*, uint64_t, uint8_t*, uint32_t);
	vsf_err_t (*write)(struct vsfsm_pt_t*, vsfsm_evt_t,
						struct fakefat32_file_t*, uint64_t, uint8_t*, uint32_t);
};

struct fakefat32_file_t
{
	struct vsfile_memfile_t memfile;

	struct fakefat32_cb_t cb;
	uint32_t first_cluster;
	PACKED_HEAD struct PACKED_MID
	{
		uint8_t CrtTimeTenth;
		uint16_t CrtTime;
		uint16_t CrtData;
		uint16_t LstAccData;
		uint16_t FstClusHI;
		uint16_t WrtTime;
		uint16_t WrtData;
		uint16_t FstClusLO;
	} record; PACKED_TAIL
};

struct fakefat32_param_t
{
	uint16_t sector_size;
	uint32_t sector_number;
	uint8_t sectors_per_cluster;

	uint32_t volume_id;
	uint32_t disk_id;
	struct fakefat32_file_t root[2];
};

#ifdef VSFCFG_STANDALONE_MODULE
#define FAKEFAT32_MODNAME					"vsf.tool.fakefat32"

struct fakefat32_modifs_t
{
	struct vsfmal_drv_t mal_drv;
	struct vsfile_fsop_t fs_op;

	uint8_t mbr[512];
};

vsf_err_t fakefat32_modexit(struct vsf_module_t*);
vsf_err_t fakefat32_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define FAKEFAT32_MOD						\
	((struct fakefat32_modifs_t *)vsf_module_load(FAKEFAT32_MODNAME, true))
#define fakefat32_mal_drv					FAKEFAT32_MOD->mal_drv
#define fakefat32_fs_op						FAKEFAT32_MOD->fs_op
#define fakefat32_mbr						FAKEFAT32_MOD->mbr

#else
extern const struct vsfmal_drv_t fakefat32_mal_drv;
extern const struct vsfile_fsop_t fakefat32_fs_op;
#endif

#endif	// __FAKEFAT32_H_INCLUDED__
