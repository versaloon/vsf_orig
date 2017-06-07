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

#ifndef __STM32F4_CORE_H_INCLUDED__
#define __STM32F4_CORE_H_INCLUDED__

#define STM32F4_CLK_HSI				(1UL << 0)
#define STM32F4_CLK_HSE				(1UL << 1)
#define STM32F4_CLK_PLL				(1UL << 2)

enum stm32f4_hclksrc_t
{
	STM32F4_HCLKSRC_HSI = 0,
	STM32F4_HCLKSRC_HSE = 1,
	STM32F4_HCLKSRC_PLL = 2
};

enum stm32f4_pllsrc_t
{
	STM32F4_PLLSRC_HSI = 0,
	STM32F4_PLLSRC_HSE = 1,
};

struct stm32f4_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;

	uint32_t clk_enable;

	enum stm32f4_hclksrc_t hclksrc;
	enum stm32f4_pllsrc_t pllsrc;

	uint32_t hsi_freq_hz;
	uint32_t hse_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t hclk_freq_hz;
	uint32_t pclk1_freq_hz;
	uint32_t pclk2_freq_hz;
};

vsf_err_t stm32f4_init(void *p);
vsf_err_t stm32f4_fini(void *p);
vsf_err_t stm32f4_reset(void *p);
uint32_t stm32f4_get_stack(void);
vsf_err_t stm32f4_set_stack(uint32_t sp);
void stm32f4_sleep(uint32_t mode);
vsf_err_t stm32f4_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t stm32f4_pendsv_trigger(void);

vsf_err_t stm32f4_tickclk_init(void);
vsf_err_t stm32f4_tickclk_fini(void);
vsf_err_t stm32f4_tickclk_start(void);
vsf_err_t stm32f4_tickclk_stop(void);
uint32_t stm32f4_tickclk_get_count(void);
vsf_err_t stm32f4_tickclk_config_cb(void (*callback)(void *), void *param);

#endif	// __stm32f4_CORE_H_INCLUDED__
