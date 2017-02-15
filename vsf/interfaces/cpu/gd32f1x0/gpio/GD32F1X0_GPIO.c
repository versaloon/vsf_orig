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

