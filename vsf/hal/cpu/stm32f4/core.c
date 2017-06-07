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

#include "app_type.h"
#include "compiler.h"
#include "vsfhal.h"

#include "stm32f4xx.h"
#include "core.h"

static struct stm32f4_info_t stm32f4_info =
{
	0, CORE_VECTOR_TABLE,
	CORE_CLKEN,
	CORE_HCLKSRC, CORE_PLLSRC,
	HSI_FREQ_HZ, HSE_FREQ_HZ,
	CORE_PLL_FREQ_HZ, CORE_HCLK_FREQ_HZ, CORE_PCLK1_FREQ_HZ, CORE_PCLK2_FREQ_HZ,
};

vsf_err_t stm32f4_init(void *p)
{
	if (p != NULL)
	{
		stm32f4_info = *(struct stm32f4_info_t *)p;
	}
	
	FLASH->ACR |= FLASH_ACR_ICEN;
	FLASH->ACR |= FLASH_ACR_DCEN;
	FLASH->ACR |= FLASH_ACR_PRFTEN;
	
	// enable hsi and select hclksrc to hsi
	RCC->CR |= RCC_CR_HSION;
	while (!(RCC->CR & RCC_CR_HSIRDY));
	RCC->CFGR &= ~RCC_CFGR_SW;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

    // enable clk
    if (stm32f4_info.clk_enable & STM32F4_CLK_HSE)
    {
		RCC->CR |= RCC_CR_HSEON;
		while (!(RCC->CR & RCC_CR_HSERDY));
    }
    else
	{
		RCC->CR &= ~RCC_CR_HSEON;
	}

	RCC->CR &= ~RCC_CR_PLLON;
    if (stm32f4_info.clk_enable & STM32F4_HCLKSRC_PLL)
	{
		uint32_t n, m, p, input, output, pllcfgr;
		pllcfgr = RCC->PLLCFGR & 0xf0000000;

		pllcfgr |= stm32f4_info.pllsrc == STM32F4_PLLSRC_HSI ? 0 :
				RCC_PLLCFGR_PLLSRC_HSE;
		input = stm32f4_info.pllsrc == STM32F4_PLLSRC_HSI ?
				stm32f4_info.hsi_freq_hz : stm32f4_info.hse_freq_hz;

		if (input % 2000000)
		{
			m = input / 1000000;
			input = 1000000;
		}
		else
		{
			m = input / 2000000;
			input = 2000000;
		}
		pllcfgr |= m;

		for (p = 2; p <= 8; p += 2)
		{
			if ((stm32f4_info.pll_freq_hz * p >= 192000000) &&
				(stm32f4_info.pll_freq_hz * p <= 432000000))
			{
				break;
			}
		}
		if (p > 8)
			return  VSFERR_FAIL;
		pllcfgr |= (p / 2 - 1) << 16;

		output = stm32f4_info.pll_freq_hz * p;
		n = output / input;
		pllcfgr |= n << 6;

		pllcfgr |= (output / 48000000) << 24;
		
		RCC->PLLCFGR = pllcfgr;
		RCC->CR |= RCC_CR_PLLON;
		while (!(RCC->CR & RCC_CR_PLLRDY));
	} 

	// set pclk and hclk
	RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
	
	if (stm32f4_info.pll_freq_hz > stm32f4_info.hclk_freq_hz * 4)
		RCC->CFGR |= 0x8 << 4 | 0x2 << 4;
	else if (stm32f4_info.pll_freq_hz > stm32f4_info.hclk_freq_hz * 2)
		RCC->CFGR |= 0x8 << 4 | 0x1 << 4;
	else if (stm32f4_info.pll_freq_hz > stm32f4_info.hclk_freq_hz * 1)
		RCC->CFGR |= 0x8 << 4 | 0x0 << 4;
	else
		RCC->CFGR |= 0;
	
	if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk1_freq_hz * 8)
		RCC->CFGR |= 0x4 << 10 | 0x3 << 10;
	else if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk1_freq_hz * 4)
		RCC->CFGR |= 0x4 << 10 | 0x2 << 10;
	else if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk1_freq_hz * 2)
		RCC->CFGR |= 0x4 << 10 | 0x1 << 10;
	else if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk1_freq_hz * 1)
		RCC->CFGR |= 0x4 << 10 | 0x0 << 10;
	else
		RCC->CFGR |= 0;
	
	if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk2_freq_hz * 8)
		RCC->CFGR |= 0x4 << 13 | 0x3 << 13;
	else if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk2_freq_hz * 4)
		RCC->CFGR |= 0x4 << 13 | 0x2 << 13;
	else if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk2_freq_hz * 2)
		RCC->CFGR |= 0x4 << 13 | 0x1 << 13;
	else if (stm32f4_info.hclk_freq_hz > stm32f4_info.pclk2_freq_hz * 1)
		RCC->CFGR |= 0x4 << 13 | 0x0 << 13;
	else
		RCC->CFGR |= 0;

	// select
	RCC->CFGR |= stm32f4_info.hclksrc;
	while (((RCC->CFGR & RCC_CFGR_SWS) >> 2) != stm32f4_info.hclksrc);

	SCB->VTOR = stm32f4_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | stm32f4_info.priority_group;
	return VSFERR_NONE;
}

vsf_err_t stm32f4_fini(void *p)
{
	return VSFERR_NONE;
}
vsf_err_t stm32f4_reset(void *p)
{
	return VSFERR_NONE;
}
uint32_t stm32f4_get_stack(void)
{
	return __get_MSP();
}
vsf_err_t stm32f4_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}
void stm32f4_sleep(uint32_t mode)
{
	// TODO
	return;
}

// Pendsv
struct stm32f4_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static stm32f4_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (stm32f4_pendsv.on_pendsv != NULL)
	{
		stm32f4_pendsv.on_pendsv(stm32f4_pendsv.param);
	}
}

vsf_err_t stm32f4_pendsv_config(void (*on_pendsv)(void *),
		void *param)
{
	stm32f4_pendsv.on_pendsv = on_pendsv;
	stm32f4_pendsv.param = param;

	if (stm32f4_pendsv.on_pendsv != NULL)
	{
		SCB->SHP[10] = 0xFF;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32f4_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

static void (*stm32f4_tickclk_callback)(void *param) = NULL;
static void *stm32f4_tickclk_param = NULL;
static volatile uint32_t stm32f4_tickcnt = 0;

vsf_err_t stm32f4_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t stm32f4_tickclk_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t stm32f4_tickclk_init(void)
{
	stm32f4_tickcnt = 0;
	SysTick->LOAD = stm32f4_info.hclk_freq_hz / 1000;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
	NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
	return VSFERR_NONE;
}

vsf_err_t stm32f4_tickclk_fini(void)
{
	return stm32f4_tickclk_stop();
}

static uint32_t stm32f4_tickclk_get_count_local(void)
{
	return stm32f4_tickcnt;
}

uint32_t stm32f4_tickclk_get_count(void)
{
	uint32_t count1, count2;

	do {
		count1 = stm32f4_tickclk_get_count_local();
		count2 = stm32f4_tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

ROOTFUNC void SysTick_Handler(void)
{
	stm32f4_tickcnt++;
	if (stm32f4_tickclk_callback != NULL)
	{
		stm32f4_tickclk_callback(stm32f4_tickclk_param);
	}
}

vsf_err_t stm32f4_tickclk_config_cb(void (*callback)(void *), void *param)
{
	uint32_t tmp = SysTick->CTRL;

	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
	stm32f4_tickclk_callback = callback;
	stm32f4_tickclk_param = param;
	SysTick->CTRL = tmp;
	return VSFERR_NONE;
}

