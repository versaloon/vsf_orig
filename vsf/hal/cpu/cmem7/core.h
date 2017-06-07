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

#ifndef __CMEM7_CORE_H_INCLUDED__
#define __CMEM7_CORE_H_INCLUDED__

struct cmem7_info_t
{
	uint32_t sys_freq_hz;
	uint8_t priority_group;
	uint32_t vector_table;
};

vsf_err_t cmem7_init(void *p);
vsf_err_t cmem7_fini(void *p);
vsf_err_t cmem7_reset(void *p);
uint32_t cmem7_get_stack(void);
vsf_err_t cmem7_set_stack(uint32_t sp);
void cmem7_sleep(uint32_t mode);
vsf_err_t cmem7_pendsv(void (*on_pendsv)(void *), void *param);

vsf_err_t cmem7_get_info(struct cmem7_info_t **info);

vsf_err_t cmem7_tickclk_init(void);
vsf_err_t cmem7_tickclk_fini(void);
vsf_err_t cmem7_tickclk_start(void);
vsf_err_t cmem7_tickclk_stop(void);
uint32_t cmem7_tickclk_get_count(void);
vsf_err_t cmem7_tickclk_set_callback(void (*callback)(void*), void *param);

#endif	// __CMEM7_CORE_H_INCLUDED__

