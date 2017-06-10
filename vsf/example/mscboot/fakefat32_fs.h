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
#ifndef __FAKEFAT32_FS_H_INCLUDED__
#define __FAKEFAT32_FS_H_INCLUDED__

#include "component/fakefat32/fakefat32.h"

// fakefat32
const char config[] =
"#!/usr/bin/python\n\
import os\n\
from Tkinter import *\n\
\n\
def get_u32(buff, i):\n\
	return buff[i] + (buff[i+1] << 8) + (buff[i+2] << 16) + (buff[i+3] << 24)\n\
\n\
def set_u32(buff, i, value):\n\
	buff[i] = (value >> 0) & 0xFF\n\
	buff[i + 1] = (value >> 8) & 0xFF\n\
	buff[i + 2] = (value >> 16) & 0xFF\n\
	buff[i + 3] = (value >> 24) & 0xFF\n\
\n\
def save_do(reset):\n\
	path = './config.bin'\n\
	file = open(path, 'rb+')\n\
	if reset:\n\
		config = bytearray(4)\n\
		set_u32(config, 0, 1)\n\
	else:\n\
		config = bytearray(8)\n\
		set_u32(config, 0, 0)\n\
		set_u32(config, 4, int(entryCali.get()))\n\
	file.write(config)\n\
	file.close()\n\
	if reset:\n\
		sys.exit()\n\
\n\
def reset():\n\
	save_do(True)\n\
\n\
def save():\n\
	save_do(False)\n\
\n\
path = './config.bin'\n\
file = open(path, 'rb')\n\
data = bytearray(file.read())\n\
file.close();\n\
\n\
root = Tk()\n\
root.title('VSF_CONFIG')\n\
Label(root, text='calibration:').grid(row=0, column=0)\n\
strvar = StringVar()\n\
strvar.set(str(get_u32(data, 4)))\n\
entryCali = Entry(root, textvariable=strvar)\n\
entryCali.grid(row=0, column=1)\n\
btnReset = Button(root, text='Reset', state='normal', command=reset)\n\
btnReset.grid(row=2, column=0)\n\
btnSave = Button(root, text='Save', state='normal', command=save)\n\
btnSave.grid(row=2, column=1)\n\
root.mainloop()\n";

// config size MUST be <= 512 bytes
struct msc_config_t
{
	bool reset;
	uint32_t calibration;
};
static uint32_t size;
static void write_firmware_on_finish(void *param, vsf_err_t result)
{
	vsfsm_post_evt_pending((struct vsfsm_t *)param, VSFSM_EVT_USER);
}
static void do_reset(void *p)
{
	vsfhal_core_reset(NULL);
}

static vsf_err_t read_config(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct fakefat32_file_t *file, uint64_t offset, uint8_t *buff,
		uint32_t pagesize)
{
	memset(buff, 0, pagesize);
	memcpy(buff, (uint8_t *)(APPCFG_FLASH_BASE + APPCFG_CFG_ADDR), sizeof(struct msc_config_t));
	return VSFERR_NONE;
}
static vsf_err_t write_config(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct fakefat32_file_t *file, uint64_t offset, uint8_t *buff,
		uint32_t pagesize)
{
	struct msc_config_t *config = (struct msc_config_t *)buff;
	uint32_t block_size;

	vsfsm_pt_begin(pt);
	if (config->reset)
	{
		vsftimer_create_cb(100, 1, do_reset, NULL);
	}
	else
	{
		vsfhal_flash_config_cb(0, 0xFF, pt->sm, write_firmware_on_finish);

		size = 0;
		while (size < sizeof(struct msc_config_t))
		{
			vsfhal_flash_erase(0, APPCFG_CFG_ADDR + size);
			vsfsm_pt_wfe(pt, VSFSM_EVT_USER);
			size += vsfhal_flash_blocksize(0, APPCFG_CFG_ADDR + size, 0, 0);
		}
		size = 0;
		while (size < sizeof(struct msc_config_t))
		{
			block_size = vsfhal_flash_blocksize(0, APPCFG_CFG_ADDR + size,
										sizeof(struct msc_config_t) - size, 2);
			vsfhal_flash_write(0, APPCFG_CFG_ADDR + size, buff + size);
			size += block_size;
			vsfsm_pt_wfe(pt, VSFSM_EVT_USER);
		}
	}
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t read_firmware(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct fakefat32_file_t *file, uint64_t offset, uint8_t *buff,
		uint32_t pagesize)
{
	memset(buff, 0, pagesize);
	return VSFERR_NONE;
}
static vsf_err_t write_firmware(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct fakefat32_file_t *file, uint64_t offset, uint8_t *buff,
		uint32_t pagesize)
{
	uint32_t block_size, erase_size;

	vsfsm_pt_begin(pt);
	vsfhal_flash_config_cb(0, 0xFF, pt->sm, write_firmware_on_finish);
	size = 0;
	while (size < pagesize)
	{
		erase_size = vsfhal_flash_blocksize(0, APPCFG_FW_ADDR + offset + size, 0, 0);
		if (!((APPCFG_FW_ADDR + offset + size) & (erase_size - 1)))
		{
			vsfhal_flash_erase(0, APPCFG_FW_ADDR + offset + size);
			vsfsm_pt_wfe(pt, VSFSM_EVT_USER);
		}
		block_size = vsfhal_flash_blocksize(0, APPCFG_FW_ADDR + offset + size, pagesize - size, 2);
		vsfhal_flash_write(0, APPCFG_FW_ADDR + offset + size, buff + size);
		size += block_size;
		vsfsm_pt_wfe(pt, VSFSM_EVT_USER);
	}
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}
static struct fakefat32_file_t fakefat32_root_dir[] =
{
	{
		.memfile.file.name = "VSF_UPDATE",
		.memfile.file.attr = VSFILE_ATTR_VOLUMID,
	},
	{
		.memfile.file.name = "fw.bin",
		.memfile.file.size = APPCFG_FW_SIZE,
		.memfile.file.attr = VSFILE_ATTR_ARCHIVE,
		.cb.read = read_firmware,
		.cb.write = write_firmware,
	},
	{
		.memfile.file.name = "config.bin",
		.memfile.file.size = sizeof(struct msc_config_t),
		.memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_HIDDEN | VSFILE_ATTR_SYSTEM,
		.cb.read = read_config,
		.cb.write = write_config,
	},
	{
		.memfile.file.name = "config.py",
		.memfile.file.size = sizeof(config) - 1,
		.memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_READONLY,
		.memfile.f.buff = (uint8_t *)config,
	},
	{
		.memfile.file.name = NULL,
	},
};

#endif		// __FAKEFAT32_FS_H_INCLUDED__
