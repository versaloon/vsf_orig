/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       GPIO.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    GPIO interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

#include "app_type.h"
#include "interfaces.h"

#if IFS_GPIO_EN

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

