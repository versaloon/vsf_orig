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

#if VSFHAL_GPIO_EN

#define GD32F1X0_GPIO_NUM					6

#define RCC_APB2Periph_GPIOA			((uint32_t)0x00000004)

vsf_err_t gd32f1x0_gpio_init(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	RCC->AHBCCR |= RCC_AHBCCR_PAEN << index;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_gpio_fini(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	RCC->AHBRCR |= RCC_AHBRCR_PARST << index;
	RCC->AHBCCR &= ~(RCC_AHBCCR_PAEN << index);
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_gpio_config_pin(uint8_t index, uint8_t pin_idx, uint32_t mode)
{
	GPIO_TypeDef *gpio;
	uint8_t offset = pin_idx << 1;
	
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	
	gpio->CTLR = (gpio->CTLR & ~(0x3 << offset)) | ((mode & 0x3) << offset);
	
	gpio->OMODE &= ~(GPIO_OMODE_OM0 << pin_idx);
	gpio->OMODE |= ((mode >> 2) & 0x1) << offset;
	
	gpio->PUPD &= ~(GPIO_PUPD_PUPD0 << offset);
	gpio->PUPD |= ((mode >> 3) & 0x3) << offset;
	
	gpio->OSPD &= ~(GPIO_OSPD_OSPD0 << offset);
	gpio->OSPD |= GPIO_OSPD_OSPD0 << offset;
//	gpio->OSPD |= ((mode >> 5) & 0x3) << offset;
	
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_gpio_config(uint8_t index, uint32_t pin_mask, uint32_t io, 
							uint32_t pull_en_mask, uint32_t input_pull_mask)
{
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_gpio_set(uint8_t index, uint32_t pin_mask)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	gpio->BOR = pin_mask & 0xffff;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_gpio_clear(uint8_t index, uint32_t pin_mask)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	gpio->BCR = pin_mask;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	gpio->BOR = ((pin_mask & ~value) << 16) | (pin_mask & value);
	return VSFERR_NONE;
}

uint32_t gd32f1x0_gpio_get(uint8_t index, uint32_t pin_mask)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return 0;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	return gpio->DIR & pin_mask;
}

vsf_err_t gd32f1x0_gpio_in(uint8_t index, uint32_t pin_mask, uint32_t *value)
{
#if __VSF_DEBUG__
	if (index >= GD32F1X0_GPIO_NUM)
	{
		return 0;
	}
#endif
	
	*value = gd32f1x0_gpio_get(index, pin_mask);
	return VSFERR_NONE;
}

#endif

