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
#include "vsfhal.h"

#if VSFHAL_TIMER_EN

#include "STM32_TIM.h"

#define STM32_TIM_NUM					17
#define STM32_TIM_CHANNEL_NUM			4

static TIM_TypeDef * stm32_timers[STM32_TIM_NUM] = 
{
	TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8, TIM9, TIM10, TIM11, TIM12, 
	TIM13, TIM14, TIM15, TIM16, TIM17
};

vsf_err_t stm32_timer_init(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	switch (index)
	{
	case 0:
		RCC->APB2RSTR |= RCC_APB2Periph_TIM1;
		break;
	case 1: case 2: case 3: case 4: case 5: case 6:
		RCC->APB1RSTR |= RCC_APB1Periph_TIM2 << (index - 1);
		break;
	case 7:
		RCC->APB2RSTR |= RCC_APB2Periph_TIM8;
		break;
	case 8: case 9: case 10:
		RCC->APB2RSTR |= RCC_APB2Periph_TIM9 << (index - 8);
		break;
	case 11: case 12: case 13:
		RCC->APB1RSTR |= RCC_APB1Periph_TIM12 << (index - 11);
		break;
	case 14: case 15: case 16:
		RCC->APB2RSTR |= RCC_APB2Periph_TIM15 << (index - 14);
		break;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_fini(uint8_t index)
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	TIM_DeInit(timer);
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_config(uint8_t index, uint32_t kHz, uint32_t mode, 
							void (*overflow)(void))
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_start(uint8_t index)
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	timer->CR1 |= TIM_CR1_CEN;
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_stop(uint8_t index)
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	timer->CR1 &= ~TIM_CR1_CEN;
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_get_count(uint8_t index, uint32_t *count)
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	*count = (uint32_t)timer->CNT;
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_set_count(uint8_t index, uint32_t count)
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	timer->CNT = (uint16_t)count;
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_config_channel(uint8_t index, uint8_t channel, 
									uint32_t mode, void (*callback)(void))
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_get_channel(uint8_t index, uint8_t channel,
									uint32_t *count)
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	*count = (uint32_t)*(uint16_t *)(&timer->CCR1 + channel * 4);
	return VSFERR_NONE;
}

vsf_err_t stm32_timer_set_channel(uint8_t index, uint8_t channel,
									uint32_t count)
{
	TIM_TypeDef * timer;
	
#if __VSF_DEBUG__
	if (index >= STM32_TIM_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	timer = stm32_timers[index];
	
	*(uint16_t *)(&timer->CCR1 + channel * 4) = (uint16_t)count;
	return VSFERR_NONE;
}

#endif
