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

#if VSFHAL_ADC_EN

#include "GD32_ADC.h"

#define GD32_ADC_NUM					1

struct gd32_adc_t
{
	ADC_TypeDef *adc;
	IRQn_Type irqn;
}static const gd32_adcs[GD32_ADC_NUM] = {{ADC1, ADC1_CMP_IRQn}};
static void (*gd32_adc_callback[GD32_ADC_NUM])(void *param, uint16_t value);
static void *gd32_adc_param[GD32_ADC_NUM];

vsf_err_t gd32f1x0_adc_init(uint8_t index, int32_t int_priority)
{
	ADC_TypeDef *adc;
	IRQn_Type irqn;

	adc = gd32_adcs[index].adc;
	irqn = gd32_adcs[index].irqn;

	switch (index)
	{
	case 0:
		RCC->APB2RCR |= RCC_APB2RCR_ADC1RST;
		RCC->APB2RCR &= ~RCC_APB2RCR_ADC1RST;
		RCC->APB2CCR |= RCC_APB2CCR_ADC1EN;
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}

	adc->CTLR1 = 0;
	adc->CTLR2 = ADC_CTLR2_ADCON;
	adc->RSQ3 = 0;

	gd32_adc_callback[index] = NULL;
	if (int_priority >= 0)
	{
		NVIC_SetPriority(irqn, (uint32_t)int_priority);
		NVIC_EnableIRQ(irqn);
	}
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_adc_fini(uint8_t index)
{
	switch (index)
	{
	case 0:
		RCC->APB2CCR &= ~RCC_APB2CCR_ADC1EN;
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}
	return VSFERR_NONE;
}

ROOTFUNC void ADC1_CMP_IRQHandler(void);
void gd32f1x0_adc_poll(void)
{
	ADC1_CMP_IRQHandler();
}

vsf_err_t gd32f1x0_adc_config(uint8_t index, uint32_t clock_hz, uint32_t mode)
{
	ADC_TypeDef *adc = gd32_adcs[index].adc;

	RCC->GCFGR &= ~RCC_GCFGR_ADCPS;
	RCC->GCFGR |= RCC_GCFGR_ADCPS_DIV8;
	adc->CTLR2 |= mode;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_adc_config_channel(uint8_t index, uint8_t channel, 
								uint8_t cycles)
{
	ADC_TypeDef *adc = gd32_adcs[index].adc;

	// channel:
	// channel 0  -- 7  : PA0 -- PA7
	// channel 8  -- 9  : PB0 -- PB1
	// channel 10 -- 15 : PC0 -- PC5
	if (channel <= 7)
	{
		RCC->AHBCCR |= RCC_AHBCCR_PAEN;
		GPIOA->CTLR = (GPIOA->CTLR & ~(0x03 << ((channel - 0) * 4))) | 
						(uint32_t)gd32f1x0_GPIO_AN << ((channel - 0) * 4);
	}
	else if (channel >= 10)
	{
		RCC->AHBCCR |= RCC_AHBCCR_PCEN;
		GPIOC->CTLR = (GPIOC->CTLR & ~(0x03 << ((channel - 10) * 4))) | 
						(uint32_t)gd32f1x0_GPIO_AN << ((channel - 10) * 4);
	}
	else
	{
		RCC->AHBCCR |= RCC_AHBCCR_PBEN;
		GPIOB->CTLR = (GPIOB->CTLR & ~(0x03 << ((channel - 8) * 4))) | 
						(uint32_t)gd32f1x0_GPIO_AN << ((channel - 8) * 4);
	}

	if (channel > 9)
		adc->SPT1 |= 0x07 << (3 * (channel - 10));
	else
		adc->SPT2 |= 0x07 << (3 * (channel - 0));
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_adc_calibrate(uint8_t index, uint8_t channel)
{
	ADC_TypeDef *adc = gd32_adcs[index].adc;

	adc->RSQ3 = channel;
	adc->CTLR2 |= ADC_CTLR2_RSTCLB;
	while (adc->CTLR2 & ADC_CTLR2_RSTCLB);
	adc->CTLR2 |= ADC_CTLR2_CLB;
	while (adc->CTLR2 & ADC_CTLR2_CLB);
	return VSFERR_NONE;
}

uint32_t gd32f1x0_adc_get_max_value(uint8_t index)
{
	REFERENCE_PARAMETER(index);
	return (1 << 12);
}

vsf_err_t gd32f1x0_adc_start(uint8_t index, uint8_t channel,
							void (callback)(void *, uint16_t), void *param)
{
	ADC_TypeDef *adc = gd32_adcs[index].adc;

	gd32_adc_callback[index] = callback;
	gd32_adc_param[index] = param;

	adc->RSQ3 = channel;
	adc->STR &= ~ADC_STR_EORC;
	adc->CTLR1 |= ADC_CTLR1_EORCIE;
	adc->CTLR2 |= ADC_CTLR2_SWRCST;
	return VSFERR_NONE;
}

ROOTFUNC void ADC1_CMP_IRQHandler(void)
{
	if (ADC1->STR & ADC_STR_EORC)
	{
		ADC1->CTLR2 &= ~ADC_CTLR2_SWRCST;
		if (gd32_adc_callback[0] != NULL)
		{
			gd32_adc_callback[0](gd32_adc_param[0], ADC1->RDTR);
		}
	}
}

#endif
