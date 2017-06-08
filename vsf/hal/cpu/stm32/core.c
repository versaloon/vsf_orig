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

#include "vsf_err.h"

#include "app_cfg.h"
#include "app_type.h"

#include "vsfhal_cfg.h"
#include "vsfhal_const.h"
#include "vsfhal.h"
#include "core.h"

#define STM32_RCC_CR_HSEON				(1 << 16)
#define STM32_RCC_CR_HSERDY				(1 << 17)
#define STM32_RCC_CR_HSEBYP				(1 << 18)
#define STM32_RCC_CR_PLLON				(1 << 24)
#define STM32_RCC_CR_PLLRDY				(1 << 25)

#define STM32_RCC_CFGR_SW_MSK			0x00000003
#define STM32_RCC_CFGR_HPRE_SFT			4
#define STM32_RCC_CFGR_HPRE_MSK			(0x0F << STM32_RCC_CFGR_HPRE_SFT)
#define STM32_RCC_CFGR_PPRE1_SFT		8
#define STM32_RCC_CFGR_PPRE1_MSK		(0x07 << STM32_RCC_CFGR_PPRE1_SFT)
#define STM32_RCC_CFGR_PPRE2_SFT		11
#define STM32_RCC_CFGR_PPRE2_MSK		(0x07 << STM32_RCC_CFGR_PPRE2_SFT)
#define STM32_RCC_CFGR_PLLSRC			(1 << 16)
#define STM32_RCC_CFGR_PLLXTPRE			(1 << 17)
#define STM32_RCC_CFGR_PLLMUL_SFT		18
#define STM32_RCC_CFGR_PLLMUL_MSK		(0x0F << STM32_RCC_CFGR_PLLMUL_SFT)

#define STM32_FLASH_ACR_PRFTBE			(1 << 4)

#define STM32_RCC_APB2ENR_AFIO			(1 << 0)

#define STM32_AFIO_MAPR_SWJCFG_SFT		24

#define STM32_HSI_FREQ_HZ				(8 * 1000 * 1000)

#define STM32_UID_ADDR					0x1FFFF7E8
#define STM32_UID_SIZE					12

static struct stm32_info_t stm32_info = 
{
	0, CORE_VECTOR_TABLE, CORE_CLKSRC, CORE_PLLSRC, CORE_RTCSRC, CORE_HSE_TYPE,
	OSC0_FREQ_HZ, CORE_PLL_FREQ_HZ, CORE_AHB_FREQ_HZ, CORE_APB1_FREQ_HZ,
	CORE_APB2_FREQ_HZ, CORE_FLASH_LATENCY, CORE_DEBUG
};
uint32_t stm32_dma_dummy;

// Pendsv
struct stm32_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static stm32_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (stm32_pendsv.on_pendsv != NULL)
	{
		stm32_pendsv.on_pendsv(stm32_pendsv.param);
	}
}

vsf_err_t stm32_pendsv_config(void (*on_pendsv)(void *), void *param)
{
	stm32_pendsv.on_pendsv = on_pendsv;
	stm32_pendsv.param = param;

	if (stm32_pendsv.on_pendsv != NULL)
	{
		SCB->SHP[10] = 0xFF;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

vsf_err_t stm32_get_info(struct stm32_info_t **info)
{
	*info = &stm32_info;
	return VSFERR_NONE;
}

vsf_err_t stm32_fini(void *p)
{
	return VSFERR_NONE;
}

vsf_err_t stm32_reset(void *p)
{
	NVIC_SystemReset();
	return VSFERR_NONE;
}

uint32_t stm32_get_stack(void)
{
	return __get_MSP();
}

vsf_err_t stm32_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void stm32_sleep(uint32_t mode)
{
	vsf_leave_critical();
	__WFI();
}

static uint32_t __log2__(uint32_t n)
{
	uint32_t i, value = 1;
	
	for (i = 0; i < 31; i++)
	{
		if (value == n)
		{
			return i;
		}
		value <<= 1;
	}
	return 0;
}

vsf_err_t stm32_init(void *p)
{
	uint32_t tmp32;
	
	if (p != NULL)
	{
		stm32_info = *(struct stm32_info_t *)p;
	}
	
	switch (stm32_info.clksrc)
	{
	case STM32_CLKSRC_HSI:
		stm32_info.sys_freq_hz = STM32_HSI_FREQ_HZ;
		break;
	case STM32_CLKSRC_HSE:
		stm32_info.sys_freq_hz = OSC0_FREQ_HZ;
		break;
	case STM32_CLKSRC_PLL:
		stm32_info.sys_freq_hz = CORE_PLL_FREQ_HZ;
		break;
	}
	
	// RCC Reset
	RCC->CR |= (uint32_t)0x00000001;
#ifndef STM32F10X_CL
	RCC->CFGR &= (uint32_t)0xF8FF0000;
#else
	RCC->CFGR &= (uint32_t)0xF0FF0000;
#endif
	RCC->CR &= (uint32_t)0xFEF6FFFF;
	RCC->CR &= (uint32_t)0xFFFBFFFF;
	RCC->CFGR &= (uint32_t)0xFF80FFFF;
#ifdef STM32F10X_CL
	RCC->CR &= (uint32_t)0xEBFFFFFF;
	RCC->CIR = 0x00FF0000;
	RCC->CFGR2 = 0x00000000;
#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) 
	RCC->CIR = 0x009F0000;
	RCC->CFGR2 = 0x00000000;      
#else
	RCC->CIR = 0x009F0000;
#endif
	
	if ((STM32_CLKSRC_HSE == stm32_info.clksrc) || 
		(STM32_PLLSRC_HSE == stm32_info.pllsrc) || 
		(STM32_PLLSRC_HSEd2 == stm32_info.pllsrc) || 
		(STM32_RTCSRC_HSEd128 == stm32_info.rtcsrc))
	{
		RCC->CR |= STM32_RCC_CR_HSEON;
		
		if (STM32_HSE_TYPE_CLOCK == stm32_info.hse_type)
		{
			RCC->CR |= STM32_RCC_CR_HSEBYP;
		}
		else
		{
			RCC->CR &= ~STM32_RCC_CR_HSEBYP;
		}
		
		while (!(RCC->CR & STM32_RCC_CR_HSERDY));
	}
	else
	{
		RCC->CR &= ~STM32_RCC_CR_HSEON;
	}
	
	FLASH->ACR = STM32_FLASH_ACR_PRFTBE | CORE_FLASH_LATENCY;
	RCC->CFGR &= ~(STM32_RCC_CFGR_HPRE_MSK | STM32_RCC_CFGR_PPRE1_MSK | 
					STM32_RCC_CFGR_PPRE2_MSK);
	
	tmp32 = __log2__(stm32_info.sys_freq_hz / stm32_info.ahb_freq_hz);
	if (tmp32)
	{
		RCC->CFGR |= (0x08 | (tmp32 - 1)) << STM32_RCC_CFGR_HPRE_SFT;
	}
	tmp32 = __log2__(stm32_info.sys_freq_hz / stm32_info.apb1_freq_hz);
	if (tmp32)
	{
		RCC->CFGR |= (0x04 | (tmp32 - 1)) << STM32_RCC_CFGR_PPRE1_SFT;
	}
	tmp32 = __log2__(stm32_info.sys_freq_hz / stm32_info.apb2_freq_hz);
	if (tmp32)
	{
		RCC->CFGR |= (0x04 | (tmp32 - 1)) << STM32_RCC_CFGR_PPRE2_SFT;
	}
	
	if (stm32_info.pll_freq_hz)
	{
		RCC->CFGR &= ~(STM32_RCC_CFGR_PLLMUL_MSK | STM32_RCC_CFGR_PLLSRC | 
						STM32_RCC_CFGR_PLLXTPRE);
		switch (stm32_info.pllsrc)
		{
		case STM32_PLLSRC_HSE:
			tmp32 = stm32_info.osc_freq_hz;
			RCC->CFGR |= STM32_RCC_CFGR_PLLSRC;
			break;
		case STM32_PLLSRC_HSEd2:
			tmp32 = stm32_info.osc_freq_hz / 2;
			RCC->CFGR |= STM32_RCC_CFGR_PLLSRC | STM32_RCC_CFGR_PLLXTPRE;
			break;
		case STM32_PLLSRC_HSId2:
			tmp32 = STM32_HSI_FREQ_HZ / 2;
			break;
		}
		tmp32 = stm32_info.pll_freq_hz / tmp32;
#if __VSF_DEBUG__
		if ((tmp32 < 2) || (tmp32 > 16))
		{
			return VSFERR_INVALID_PARAMETER;
		}
#endif
		RCC->CFGR |= ((tmp32 - 2) << STM32_RCC_CFGR_PLLMUL_SFT);
		
		RCC->CR |= STM32_RCC_CR_PLLON;
		while (!(RCC->CR & STM32_RCC_CR_PLLRDY));
	}
	
	RCC->CFGR &= ~STM32_RCC_CFGR_SW_MSK;
	RCC->CFGR |= CORE_CLKSRC;
	while (((RCC->CFGR >> 2) & STM32_RCC_CFGR_SW_MSK) != CORE_CLKSRC);
	
	RCC->APB2ENR |= STM32_RCC_APB2ENR_AFIO;
	AFIO->MAPR |= stm32_info.debug_setting << STM32_AFIO_MAPR_SWJCFG_SFT;
	
	SCB->VTOR = stm32_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | stm32_info.priority_group;
	return VSFERR_NONE;
}

uint32_t stm32_uid_get(uint8_t *buffer, uint32_t size)
{
	if (NULL == buffer)
	{
		return 0;
	}
	
	if (size > STM32_UID_SIZE)
	{
		size = STM32_UID_SIZE;
	}
	
	memcpy(buffer, (uint8_t *)STM32_UID_ADDR, size);
	return size;
}

// tickclk
static void (*stm32_tickclk_cb)(void *param) = NULL;
static void *stm32_tickclk_param = NULL;
static uint32_t stm32_tickcnt = 0;
vsf_err_t stm32_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t stm32_tickclk_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

static uint32_t stm32_tickclk_get_count_local(void)
{
	return stm32_tickcnt;
}

uint32_t stm32_tickclk_get_count(void)
{
	uint32_t count1, count2;
	
	do {
		count1 = stm32_tickclk_get_count_local();
		count2 = stm32_tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

ROOTFUNC void SysTick_Handler(void)
{
	stm32_tickcnt++;
	if (stm32_tickclk_cb != NULL)
	{
		stm32_tickclk_cb(stm32_tickclk_param);
	}
}

void stm32_tickclk_poll(void)
{
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
		SysTick_Handler();
}

vsf_err_t stm32_tickclk_config_cb(void (*callback)(void*), void *param)
{
	uint32_t tmp = SysTick->CTRL;
	
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
	stm32_tickclk_cb = callback;
	stm32_tickclk_param = param;
	SysTick->CTRL = tmp;
	return VSFERR_NONE;
}

vsf_err_t stm32_tickclk_init(int32_t int_priority)
{
	stm32_tickcnt = 0;
	SysTick->LOAD = stm32_info.sys_freq_hz / 1000;
	if (int_priority >= 0)
	{
		SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
		NVIC_SetPriority(SysTick_IRQn, (uint32_t)int_priority);
	}
	else
	{
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_tickclk_fini(void)
{
	return stm32_tickclk_stop();
}
