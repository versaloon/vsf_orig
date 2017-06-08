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

#ifndef __NUC505_CORE_H_INCLUDED__
#define __NUC505_CORE_H_INCLUDED__

#define NUC505_CLK_HXT				(1UL << 0)
#define NUC505_CLK_LXT				(1UL << 1)
#define NUC505_CLK_LIRC				(1UL << 2)
#define NUC505_CLK_PLL				(1UL << 3)
#define NUC505_CLK_APLL				(1UL << 4)

enum nuc505_hclksrc_t
{
	NUC505_HCLKSRC_PLLFOUT = 1,
	NUC505_HCLKSRC_HXT = 0,
};

struct nuc505_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;

	uint32_t clk_enable;

	enum nuc505_hclksrc_t hclksrc;

	uint32_t osc_freq_hz;
	uint32_t osc32k_freq_hz;
	uint32_t lirc_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t apll_freq_hz;
	uint32_t cpu_freq_hz;
	uint32_t hclk_freq_hz;
	uint32_t pclk_freq_hz;
};

vsf_err_t nuc505_init(void *p);
vsf_err_t nuc505_fini(void *p);
vsf_err_t nuc505_reset(void *p);
uint32_t nuc505_get_stack(void);
vsf_err_t nuc505_set_stack(uint32_t sp);
void nuc505_sleep(uint32_t mode);
vsf_err_t nuc505_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t nuc505_pendsv_trigger(void);
vsf_err_t nuc505_get_info(struct nuc505_info_t **info);

vsf_err_t nuc505_tickclk_init(int32_t int_priority);
vsf_err_t nuc505_tickclk_fini(void);
void nuc505_tickclk_poll(void);
vsf_err_t nuc505_tickclk_set_interval(uint16_t ms);
vsf_err_t nuc505_tickclk_start(void);
vsf_err_t nuc505_tickclk_stop(void);
uint32_t nuc505_tickclk_get_count(void);
vsf_err_t nuc505_tickclk_config_cb(void (*callback)(void*), void *param, bool interrupt);

// special
int32_t nuc505_is_running_on_ram(void);
vsf_err_t nuc505_code_map(uint8_t en, uint8_t rst, uint8_t len_kb, uint32_t addr);


#endif	// __NUC505_CORE_H_INCLUDED__

