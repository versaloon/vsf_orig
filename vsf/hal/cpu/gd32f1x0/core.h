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

#ifndef __GD32F1X0_CORE_H_INCLUDED__
#define __GD32F1X0_CORE_H_INCLUDED__

#define GD32F1X0_CLK_HSI				(1UL << 0)
#define GD32F1X0_CLK_HSE				(1UL << 1)
#define GD32F1X0_CLK_PLL				(1UL << 2)

struct gd32f1x0_afio_pin_t
{
	uint8_t port;
	uint8_t pin;
	int8_t af;
};
vsf_err_t gd32f1x0_afio_config(const struct gd32f1x0_afio_pin_t *pin, uint32_t mode);

enum gd32f1x0_clksrc_t
{
	GD32F1X0_CLKSRC_HSI = 0,
	GD32F1X0_CLKSRC_HSE = 1,
	GD32F1X0_CLKSRC_PLL = 2
};
enum gd32f1x0_pllsrc_t
{
	GD32F1X0_PLLSRC_HSID2,
	GD32F1X0_PLLSRC_HSE,
	GD32F1X0_PLLSRC_HSED2,
};
struct gd32f1x0_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;
	
	uint32_t clk_enable;
	
	enum gd32f1x0_clksrc_t clksrc;
	enum gd32f1x0_pllsrc_t pllsrc;
	
	uint32_t hse_freq_hz;
	uint32_t hsi_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t ahb_freq_hz;
	uint32_t apb_freq_hz;
	
	uint32_t sys_freq_hz;
};

vsf_err_t gd32f1x0_init(void *p);
vsf_err_t gd32f1x0_fini(void *p);
vsf_err_t gd32f1x0_reset(void *p);
uint32_t gd32f1x0_get_stack(void);
vsf_err_t gd32f1x0_set_stack(uint32_t sp);
void gd32f1x0_sleep(uint32_t mode);
vsf_err_t gd32f1x0_get_info(struct gd32f1x0_info_t **info);
vsf_err_t gd32f1x0_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t gd32f1x0_pendsv_trigger(void);

uint32_t gd32f1x0_uid_get(uint8_t *buffer, uint32_t size);

vsf_err_t gd32f1x0_tickclk_init(void);
vsf_err_t gd32f1x0_tickclk_fini(void);
void gd32f1x0_tickclk_poll(void);
vsf_err_t gd32f1x0_tickclk_start(void);
vsf_err_t gd32f1x0_tickclk_stop(void);
uint32_t gd32f1x0_tickclk_get_count(void);
vsf_err_t gd32f1x0_tickclk_config_cb(void (*callback)(void*), void *param);

#endif	// __GD32F1X0_CORE_H_INCLUDED__
