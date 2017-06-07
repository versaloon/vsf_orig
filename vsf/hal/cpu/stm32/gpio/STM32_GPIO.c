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

#include "STM32_GPIO.h"

#define STM32_GPIO_NUM					7

#define RCC_APB2Periph_GPIOA			((uint32_t)0x00000004)

vsf_err_t stm32_gpio_init(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA << index;
	return VSFERR_NONE;
}

vsf_err_t stm32_gpio_fini(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	RCC->APB2ENR &= ~(RCC_APB2Periph_GPIOA << index);
	RCC->APB2RSTR &= ~(RCC_APB2Periph_GPIOA << index);
	return VSFERR_NONE;
}

vsf_err_t stm32_gpio_config_pin(uint8_t index, uint8_t pin_idx, uint32_t mode)
{
	GPIO_TypeDef *gpio;
	uint32_t tmpreg = mode & 0x0F;
	
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	if(pin_idx < 8)
	{
		gpio->CRL = (gpio->CRL & ~(((uint32_t)0x0F) << ((pin_idx - 0) * 4))) | 
						tmpreg << ((pin_idx - 0) * 4);
	}
	else
	{
		gpio->CRH = (gpio->CRH & ~(((uint32_t)0x0F) << ((pin_idx - 8) * 4))) | 
						tmpreg << ((pin_idx - 8) * 4);
	}
	
	if(0x08 == tmpreg)
	{
		if(mode & 0x80)
		{
			gpio->BSRR = (((uint32_t)0x01) << pin_idx);
		}
		else
		{
			gpio->BRR = (((uint32_t)0x01) << pin_idx);
		}
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_gpio_config(uint8_t index, uint32_t pin_mask, uint32_t io, 
							uint32_t pull_en_mask, uint32_t input_pull_mask)
{
	GPIO_TypeDef *gpio;
	uint32_t mask, tmpregl, tmpregh;
	uint8_t i;
	
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	tmpregl = gpio->CRL;
	tmpregh = gpio->CRH;
	for (i = 0; i < 8; i++)
	{
		mask = (1 << i);
		if (pin_mask & mask)
		{
			tmpregl &= ~(0x0F << (4 * i));
			if (io & mask)
			{
				tmpregl |= 0x03 << (4 * i);
				if (pull_en_mask & input_pull_mask & mask)
				{
					tmpregl |= 0x01 << (4 * i + 2);
				}
			}
			else
			{
				if (pull_en_mask & mask)
				{
					tmpregl |= 0x02 << (4 * i + 2);
				}
				else
				{
					tmpregl |= 0x01 << (4 * i + 2);
				}
			}
			if (input_pull_mask & mask)
			{
				gpio->BSRR = mask;
			}
			else
			{
				gpio->BRR = mask;
			}
		}
		mask = (1 << (i + 8));
		if (pin_mask & mask)
		{
			tmpregh &= ~(0x0F << (4 * i));
			if (io & mask)
			{
				tmpregh |= 0x03 << (4 * i);
				if (pull_en_mask & input_pull_mask & mask)
				{
					tmpregh |= 0x04 << (4 * i);
				}
			}
			else
			{
				if (pull_en_mask & mask)
				{
					tmpregh |= 0x08 << (4 * i);
				}
				else
				{
					tmpregh |= 0x04 << (4 * i);
				}
			}
			if (input_pull_mask & mask)
			{
				gpio->BSRR = mask;
			}
			else
			{
				gpio->BRR = mask;
			}
		}
	}
	gpio->CRL = tmpregl;
	gpio->CRH = tmpregh;
	return VSFERR_NONE;
}

vsf_err_t stm32_gpio_set(uint8_t index, uint32_t pin_mask)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	gpio->BSRR = pin_mask;
	return VSFERR_NONE;
}

vsf_err_t stm32_gpio_clear(uint8_t index, uint32_t pin_mask)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	gpio->BRR = pin_mask;
	return VSFERR_NONE;
}

vsf_err_t stm32_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	gpio->BSRR = pin_mask & value;
	gpio->BRR = pin_mask & ~value;
	return VSFERR_NONE;
}

vsf_err_t stm32_gpio_in(uint8_t index, uint32_t pin_mask, uint32_t *value)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	*value = gpio->IDR & pin_mask;
	return VSFERR_NONE;
}

uint32_t stm32_gpio_get(uint8_t index, uint32_t pin_mask)
{
	GPIO_TypeDef *gpio;
	
#if __VSF_DEBUG__
	if (index >= STM32_GPIO_NUM)
	{
		return 0;
	}
#endif
	
	gpio = (GPIO_TypeDef *)(GPIOA_BASE + ((uint32_t)index << 10));
	return gpio->IDR & pin_mask;
}

#endif
