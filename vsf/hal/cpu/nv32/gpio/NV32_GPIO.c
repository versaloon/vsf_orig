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

#define NV32_GPIO_NUM					2

vsf_err_t nv32_gpio_init(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	return VSFERR_NONE;
}

vsf_err_t nv32_gpio_fini(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	return VSFERR_NONE;
}

vsf_err_t nv32_gpio_config_pin(uint8_t index, uint8_t pin_idx, uint32_t mode)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + ((uint32_t)index << 6));
	uint32_t *pu_reg = (uint32_t *)&PORT->PUEL + index;
	uint32_t mask = 1 << pin_idx;
	
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	if (mode & 1)
	{
		// output
		gpio->PIDR |= mask;
		gpio->PDDR |= mask;
	}
	else
	{
		// input
		gpio->PIDR &= ~mask;
		gpio->PDDR &= ~mask;
	}
	if (mode & 2)
		*pu_reg |= mask;
	else
		*pu_reg &= ~mask;
	
	return VSFERR_NONE;
}

vsf_err_t nv32_gpio_config(uint8_t index, uint32_t pin_mask, uint32_t io, 
							uint32_t pull_en_mask, uint32_t input_pull_mask)
{
	return VSFERR_NONE;
}

vsf_err_t nv32_gpio_set(uint8_t index, uint32_t pin_mask)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + ((uint32_t)index << 6));
	
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio->PSOR = pin_mask;
	return VSFERR_NONE;
}

vsf_err_t nv32_gpio_clear(uint8_t index, uint32_t pin_mask)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + ((uint32_t)index << 6));
	
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio->PCOR = pin_mask;
	return VSFERR_NONE;
}

vsf_err_t nv32_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + ((uint32_t)index << 6));
	
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	gpio->PDOR = ((pin_mask & ~value) << 16) | (pin_mask & value);
	return VSFERR_NONE;
}

uint32_t nv32_gpio_get(uint8_t index, uint32_t pin_mask)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + ((uint32_t)index << 6));
	
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return 0;
	}
#endif
	
	return gpio->PDIR & pin_mask;
}

vsf_err_t nv32_gpio_in(uint8_t index, uint32_t pin_mask, uint32_t *value)
{
#if __VSF_DEBUG__
	if (index >= NV32_GPIO_NUM)
	{
		return 0;
	}
#endif
	
	*value = nv32_gpio_get(index, pin_mask);
	return VSFERR_NONE;
}

#endif

