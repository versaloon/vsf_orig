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

#ifndef __NUC400_CORE_H_INCLUDED__
#define __NUC400_CORE_H_INCLUDED__

#define NUC400_CLK_HXT				(1UL << 0)
#define NUC400_CLK_LXT				(1UL << 1)
#define NUC400_CLK_HIRC				(1UL << 2)
#define NUC400_CLK_LIRC				(1UL << 3)

enum nuc400_hclksrc_t
{
	NUC400_HCLKSRC_HIRC = 7,
	NUC400_HCLKSRC_PLL2FOUT = 4,
	NUC400_HCLKSRC_LIRC = 3,
	NUC400_HCLKSRC_PLLFOUT = 2,
	NUC400_HCLKSRC_LXT = 1,
	NUC400_HCLKSRC_HXT = 0,
};
enum nuc400_pclksrc_t
{
	NUC400_PCLKSRC_HCLK = 0,
	NUC400_PCLKSRC_HCLKd2 = 1,
};
enum nuc400_pllsrc_t
{
	NUC400_PLLSRC_HXT = 0,
	NUC400_PLLSRC_HIRC = 1,
	NUC400_PLLSRC_NONE = -1,
};
struct nuc400_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;
	
	uint32_t clk_enable;
	
	enum nuc400_hclksrc_t hclksrc;
	enum nuc400_pclksrc_t pclksrc;
	enum nuc400_pllsrc_t pllsrc;
	
	uint32_t osc_freq_hz;
	uint32_t osc32k_freq_hz;
	uint32_t hirc_freq_hz;
	uint32_t lirc_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t cpu_freq_hz;
	uint32_t hclk_freq_hz;
	uint32_t pclk_freq_hz;
};

vsf_err_t nuc400_init(void *p);
vsf_err_t nuc400_fini(void *p);
vsf_err_t nuc400_reset(void *p);
uint32_t nuc400_get_stack(void);
vsf_err_t nuc400_set_stack(uint32_t sp);
void nuc400_sleep(uint32_t mode);
vsf_err_t nuc400_get_info(struct nuc400_info_t **info);

uint32_t nuc400_uid_get(uint8_t *buffer, uint32_t size);

vsf_err_t nuc400_delay_delayms(uint16_t ms);
vsf_err_t nuc400_delay_delayus(uint16_t us);

vsf_err_t nuc400_tickclk_init(void);
vsf_err_t nuc400_tickclk_fini(void);
vsf_err_t nuc400_tickclk_start(void);
vsf_err_t nuc400_tickclk_stop(void);
uint32_t nuc400_tickclk_get_count(void);

#endif	// __NUC400_CORE_H_INCLUDED__
