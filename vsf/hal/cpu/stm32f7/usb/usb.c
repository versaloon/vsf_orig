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

#include "app_cfg.h"
#include "app_type.h"
#include "vsfhal.h"

#include "stm32f7xx.h"

#if VSFHAL_HCD_EN

#define STM32F7_USB_NUM			2

static vsf_err_t (*irq0)(void*);
static void *irq0_param;
static vsf_err_t (*irq1)(void*);
static void *irq1_param;

ROOTFUNC void OTG_HS_IRQHandler(void)
{
	if (irq0 != NULL)
		irq0(irq0_param);
}

ROOTFUNC void OTG_FS_IRQHandler(void)
{
	if(irq1 != NULL)
		irq1(irq1_param);
}

static void ulpi_io_config(GPIO_TypeDef *gpiox, uint8_t io)
{
	gpiox->MODER &= ~(GPIO_MODER_MODER0 << (io * 2));
	gpiox->MODER |= GPIO_MODER_MODER0_1 << (io * 2);
	gpiox->OTYPER &= ~(GPIO_OTYPER_OT_0 << (io * 2));
	gpiox->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0 << (io * 2);
	gpiox->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << (io * 2));
	if (io < 8)
	{
		gpiox->AFR[0] &= ~(0xful << (io * 4));
		gpiox->AFR[0] |= 0xaul << (io * 4);
	}
	else
	{
		gpiox->AFR[1] &= ~(0xful << ((io - 8) * 4));
		gpiox->AFR[1] |= 0xaul << ((io - 8) * 4);
	}
}

vsf_err_t stm32f7_hcd_init(uint32_t index, vsf_err_t (*irq)(void *), void *param)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= STM32F7_USB_NUM)
		return VSFERR_NOT_SUPPORT;

	// enable 48M clock
	RCC->PLLSAICFGR = (96 << 16) | (0 << 16) | (2 << 24) | (2 << 28);
	RCC->CR |= RCC_CR_PLLSAION;
	while (!(RCC->CR & RCC_CR_PLLSAIRDY));	
	RCC->DCKCFGR2 |= RCC_DCKCFGR2_CK48MSEL;

	if (usb_id == 0) // HS
	{
		irq0 = irq;
		irq0_param = param;
		
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN |
				RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOHEN;
		
		// CLK
		ulpi_io_config(GPIOA, 5);

		// D0 - D7
		ulpi_io_config(GPIOA, 3);
		ulpi_io_config(GPIOB, 0);
		ulpi_io_config(GPIOB, 1);
		ulpi_io_config(GPIOB, 5);
		ulpi_io_config(GPIOB, 10);
		ulpi_io_config(GPIOB, 11);
		ulpi_io_config(GPIOB, 12);
		ulpi_io_config(GPIOB, 13);

		// STP
		ulpi_io_config(GPIOC, 0);

		// NXT
		ulpi_io_config(GPIOH, 4);

		// DIR
		ulpi_io_config(GPIOC, 2);

		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_OTGHSULPIEN);
		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_OTGHSEN);
		NVIC_EnableIRQ(OTG_HS_IRQn);
	}
	else if (usb_id == 1) // FS
	{
		irq1 = irq;
		irq1_param = param;

		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

		// TODO GPIO init

		SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);

		NVIC_EnableIRQ(OTG_FS_IRQn);
	}

	return VSFERR_NONE;
}

vsf_err_t stm32f7_hcd_fini(uint32_t index)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= STM32F7_USB_NUM)
		return VSFERR_NOT_SUPPORT;

	if (usb_id == 0) // HS
	{
		RCC->AHB1ENR &= ~(RCC_AHB1ENR_OTGHSULPIEN);
		RCC->AHB1ENR &= ~(RCC_AHB1ENR_OTGHSEN);

		NVIC_DisableIRQ(OTG_HS_IRQn);
	}
	else if (usb_id == 1) // FS
	{
		RCC->AHB2ENR &= ~(RCC_AHB2ENR_OTGFSEN);
		// RCC->APB2ENR &= ~(RCC_APB2ENR_SYSCFGEN);

		NVIC_DisableIRQ(OTG_FS_IRQn);
	}

	return VSFERR_NONE;
}


void* stm32f7_hcd_regbase(uint32_t index)
{
	switch (index >> 16)
	{
	case 0:
		return (void*)USB_OTG_HS;
	case 1:
		return (void*)USB_OTG_FS;
	default:
		return NULL;
	}
}

#endif // VSFHAL_HCD_EN


#if VSFHAL_USB_DCD_EN

#endif // VSFHAL_USB_DCD_EN
