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

#ifndef __STM32_CORE_H_INCLUDED__
#define __STM32_CORE_H_INCLUDED__

enum stm32_clksrc_t
{
	STM32_CLKSRC_HSE = 1,
	STM32_CLKSRC_HSI = 0,
	STM32_CLKSRC_PLL = 2
};
enum stm32_pllsrc_t
{
	STM32_PLLSRC_HSE,
	STM32_PLLSRC_HSEd2,
	STM32_PLLSRC_HSId2
};
enum stm32_rtcsrc_t
{
	STM32_RTCSRC_HSEd128,
	STM32_RTCSRC_LSE,
	STM32_RTCSRC_LSI
};
enum stm32_hse_type_t
{
	STM32_HSE_TYPE_CLOCK,
	STM32_HSE_TYPE_CRYSTAL
};
enum debug_setting_t
{
	STM32_DBG_JTAG_SWD = 0,
	STM32_DBG_SWD = 2,
	STM32_DBG_NONE = 4
};
struct stm32_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;
	
	enum stm32_clksrc_t clksrc;
	enum stm32_pllsrc_t pllsrc;
	enum stm32_rtcsrc_t rtcsrc;
	enum stm32_hse_type_t hse_type;
	
	uint32_t osc_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t ahb_freq_hz;
	uint32_t apb1_freq_hz;
	uint32_t apb2_freq_hz;
	
	uint8_t flash_latency;
	enum debug_setting_t debug_setting;
	
	// calculated internally
	uint32_t sys_freq_hz;
};

vsf_err_t stm32_init(void *p);
vsf_err_t stm32_fini(void *p);
vsf_err_t stm32_reset(void *p);
uint32_t stm32_get_stack(void);
vsf_err_t stm32_set_stack(uint32_t sp);
void stm32_sleep(uint32_t mode);
vsf_err_t stm32_get_info(struct stm32_info_t **info);
vsf_err_t stm32_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t stm32_pendsv_trigger(void);

uint32_t stm32_uid_get(uint8_t *buffer, uint32_t size);

vsf_err_t stm32_delay_delayms(uint16_t ms);
vsf_err_t stm32_delay_delayus(uint16_t us);

vsf_err_t stm32_tickclk_init(int32_t int_priority);
vsf_err_t stm32_tickclk_fini(void);
void stm32_tickclk_poll(void);
vsf_err_t stm32_tickclk_start(void);
vsf_err_t stm32_tickclk_stop(void);
uint32_t stm32_tickclk_get_count(void);
vsf_err_t stm32_tickclk_config_cb(void (*callback)(void*), void *param, bool interrupt);

#endif	// __STM32_CORE_H_INCLUDED__
