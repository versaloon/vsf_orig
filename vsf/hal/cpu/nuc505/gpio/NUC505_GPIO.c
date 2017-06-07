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

#include "NUC505_GPIO.h"
#include "NUC505Series.h"

#define NUC505_GPIO_NUM					4

typedef struct
{
    __IO uint32_t MODE;
    __IO uint32_t PUEN;
    __IO uint32_t DOUT;
    __I  uint32_t PIN;
} GPIO_COMMON_T;

vsf_err_t nuc505_gpio_init(uint8_t index)
{
	return VSFERR_NONE;
}

vsf_err_t nuc505_gpio_fini(uint8_t index)
{
	return VSFERR_NONE;
}

vsf_err_t nuc505_gpio_config_pin(uint8_t index, uint8_t pin_idx, uint32_t mode)
{
	GPIO_COMMON_T *gpio;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));

	if (mode == nuc505_GPIO_INFLOAT)
		gpio->MODE &= ~(0x1ul << pin_idx);
	else
		gpio->MODE |= 0x1ul << pin_idx;

	return VSFERR_NONE;
}

vsf_err_t nuc505_gpio_config(uint8_t index, uint32_t pin_mask, uint32_t io,
							uint32_t pull_en_mask, uint32_t input_pull_mask)
{
	GPIO_COMMON_T *gpio;
	uint32_t i, mask;
	uint32_t mode;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));

	for (i = 0; i < 16; i++)
	{
		mask = 1UL << i;
		if (pin_mask & mask)
		{
			//mode = !(io & mask) ? nuc505_GPIO_INFLOAT :
			//	(pull_en_mask & input_pull_mask & mask) ? nuc505_GPIO_OUTOD :
			//	nuc505_GPIO_OUTPP;
			mode = (io & mask) ? nuc505_GPIO_OUTPP : nuc505_GPIO_INFLOAT;
			nuc505_gpio_config_pin(index, i, mode);
			if (pull_en_mask & mask)
			{
				if (input_pull_mask & mask) // pull down
				{
					gpio->PUEN = (gpio->PUEN & ~(0x3 << i)) | (0x2 << i);
				}
				else // pull up
				{
					gpio->PUEN = (gpio->PUEN & ~(0x3 << i)) | (0x1 << i);
				}
			}
			else
			{
				gpio->PUEN &= ~(0x3 << i);
			}
		}
	}
	return VSFERR_NONE;
}

vsf_err_t nuc505_gpio_set(uint8_t index, uint32_t pin_mask)
{
	GPIO_COMMON_T *gpio;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	gpio->DOUT |= pin_mask;

	return VSFERR_NONE;
}

vsf_err_t nuc505_gpio_clear(uint8_t index, uint32_t pin_mask)
{
	GPIO_COMMON_T *gpio;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	gpio->DOUT &= ~pin_mask;

	return VSFERR_NONE;
}

vsf_err_t nuc505_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value)
{
	GPIO_COMMON_T *gpio;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	gpio->DOUT |= pin_mask & value;
	gpio->DOUT &= ~(pin_mask & ~value);

	return VSFERR_NONE;
}

vsf_err_t nuc505_gpio_in(uint8_t index, uint32_t pin_mask, uint32_t *value)
{
	GPIO_COMMON_T *gpio;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	*value = gpio->PIN & pin_mask;
	return VSFERR_NONE;
}

uint32_t nuc505_gpio_get(uint8_t index, uint32_t pin_mask)
{
	GPIO_COMMON_T *gpio;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	return gpio->PIN & pin_mask;
}

#endif

