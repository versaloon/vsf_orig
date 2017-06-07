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

// TODO: remove MACROs below to stm32_reg.h
#define STM32_RCC_APB2RSTR_ADC1RST		((uint32_t)1 << 9)
#define STM32_RCC_APB2RSTR_ADC2RST		((uint32_t)1 << 10)
#define STM32_RCC_APB2RSTR_ADC3RST		((uint32_t)1 << 15)

#define STM32_RCC_APB2ENR_ADC1EN		((uint32_t)1 << 9)
#define STM32_RCC_APB2ENR_ADC2EN		((uint32_t)1 << 10)
#define STM32_RCC_APB2ENR_ADC3EN		((uint32_t)1 << 15)
#define STM32_RCC_APB2ENR_IOPAEN		((uint32_t)1 << 2)
#define STM32_RCC_APB2ENR_IOPBEN		((uint32_t)1 << 3)
#define STM32_RCC_APB2ENR_IOPCEN		((uint32_t)1 << 4)
#define STM32_RCC_APB2ENR_IOPDEN		((uint32_t)1 << 5)
#define STM32_RCC_APB2ENR_IOPFEN		((uint32_t)1 << 7)

#define STM32_RCC_CFGR_ADCPRE_SFT		14
#define STM32_RCC_CFGR_ADCPRE_MSK		((uint32_t)0x03 << \
											STM32_RCC_CFGR_ADCPRE_SFT)

#define STM32_ADC_SR_STRT				((uint32_t)1 << 4)
#define STM32_ADC_SR_EOC				((uint32_t)1 << 1)

#define STM32_ADC_CR1_EOCIE				((uint32_t)1 << 5)
#define STM32_ADC_CR1_DISCEN			((uint32_t)1 << 11)

#define STM32_ADC_CR2_ADON				((uint32_t)1 << 0)
#define STM32_ADC_CR2_CAL				((uint32_t)1 << 2)
#define STM32_ADC_CR2_RSTCAL			((uint32_t)1 << 3)
#define STM32_ADC_CR2_EXTSEL_SWSTART	((uint32_t)3 << 17)
#define STM32_ADC_CR2_SWSTART			((uint32_t)1 << 22)

#define STM32_ADC_SQR1_L_SFT			20
#define STM32_ADC_SQR1_L_MASK			((uint32_t)0x0F << STM32_ADC_SQR1_L_SFT)

#if VSFHAL_ADC_EN

#include "STM32_ADC.h"

#define STM32_ADC_NUM					3

static const ADC_TypeDef *stm32_adcs[STM32_ADC_NUM] = {ADC1, ADC2, ADC3};
static void (*stm32_adc_callback[STM32_ADC_NUM])(void *param, uint16_t value);
static void *stm32_adc_param[STM32_ADC_NUM];

vsf_err_t stm32_adc_init(uint8_t index)
{
	ADC_TypeDef *adc;
	uint8_t irqn;
	
#if __VSF_DEBUG__
	if (index >= STM32_ADC_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	adc = (ADC_TypeDef *)stm32_adcs[index];
	
	switch (index)
	{
	case 0:
		RCC->APB2RSTR |= STM32_RCC_APB2RSTR_ADC1RST;
		RCC->APB2RSTR &= ~STM32_RCC_APB2RSTR_ADC1RST;
		RCC->APB2ENR |= STM32_RCC_APB2ENR_ADC1EN;
		break;
	case 1:
		RCC->APB2RSTR |= STM32_RCC_APB2RSTR_ADC2RST;
		RCC->APB2RSTR &= ~STM32_RCC_APB2RSTR_ADC2RST;
		RCC->APB2ENR |= STM32_RCC_APB2ENR_ADC2EN;
		break;
	case 2:
		RCC->APB2RSTR |= STM32_RCC_APB2RSTR_ADC3RST;
		RCC->APB2RSTR &= ~STM32_RCC_APB2RSTR_ADC3RST;
		RCC->APB2ENR |= STM32_RCC_APB2ENR_ADC3EN;
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}
	
	adc->CR1 = STM32_ADC_CR1_DISCEN;
	adc->CR2 = 0;
	adc->SQR1 = 0;
	
	stm32_adc_callback[index] = NULL;
	if (index < 2)
	{
		irqn = ADC1_2_IRQn;
	}
	else
	{
		irqn = ADC3_IRQn;
	}
	NVIC->IP[irqn] = 0xFF;
	NVIC->ISER[irqn >> 0x05] = 1UL << (irqn & 0x1F);
	
	return VSFERR_NONE;
}

vsf_err_t stm32_adc_fini(uint8_t index)
{
	switch (index)
	{
	case 0:
		RCC->APB2ENR &= ~STM32_RCC_APB2ENR_ADC1EN;
		break;
	case 1:
		RCC->APB2ENR &= ~STM32_RCC_APB2ENR_ADC2EN;
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_adc_config(uint8_t index, uint32_t clock_hz, uint32_t mode)
{
	ADC_TypeDef *adc;
	struct stm32_info_t *info;
	
	if (stm32_get_info(&info))
	{
		return VSFERR_FAIL;
	}
	
#if __VSF_DEBUG__
	if ((index >= STM32_ADC_NUM) || 
		(	((info->apb2_freq_hz / clock_hz) != 2) && 
		 	((info->apb2_freq_hz / clock_hz) != 4) && 
			((info->apb2_freq_hz / clock_hz) != 6) && 
			((info->apb2_freq_hz / clock_hz) != 8)))
	{
		return VSFERR_INVALID_PARAMETER;
	}
#endif
	adc = (ADC_TypeDef *)stm32_adcs[index];
	
	RCC->CFGR &= ~STM32_RCC_CFGR_ADCPRE_MSK;
	RCC->CFGR |= (((info->apb2_freq_hz / clock_hz) / 2) - 1) << 
					STM32_RCC_CFGR_ADCPRE_SFT;
	
	adc->CR2 = ((uint32_t)mode << 8) | STM32_ADC_CR2_EXTSEL_SWSTART;
	return VSFERR_NONE;
}

vsf_err_t stm32_adc_config_channel(uint8_t index, uint8_t channel, 
								uint8_t cycles)
{
	ADC_TypeDef *adc;
	uint32_t tmpreg;
	
#if __VSF_DEBUG__
	if (index >= STM32_ADC_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	adc = (ADC_TypeDef *)stm32_adcs[index];
	
	// common channel for all adc:
	// channel 0  -- 3  : PA0 -- PA3
	// channel 10 -- 13 : PC0 -- PC3
	// common channel for adc1 and adc2
	// channel 4  -- 7  : PA4 -- PA7
	// channel 8  -- 9  : PB0 -- PB1
	// channel 14 -- 15 : PC4 -- PC5
	// channel for adc3
	// channel 4  -- 8  : PF6 -- PF10
	if ((channel <= 3) || ((channel <= 7) && (index <= 1)))
	{
		RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPAEN;
		GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (channel * 4))) | 
						(uint32_t)stm32_GPIO_ANALOG << (channel * 4);
	}
	else if (((channel >= 10) && (channel <= 13)) || 
			 (((channel >= 14) && (channel <= 15)) && (index <= 1)))
	{
		RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPCEN;
		GPIOC->CRL = (GPIOC->CRL & ~(0x0F << ((channel - 10) * 4))) | 
						(uint32_t)stm32_GPIO_ANALOG << ((channel - 10) * 4);
	}
	else if ((channel >= 8) && (channel <= 9) && (index <= 1))
	{
		RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPBEN;
		GPIOB->CRL = (GPIOB->CRL & ~(0x0F << ((channel - 8) * 4))) | 
						(uint32_t)stm32_GPIO_ANALOG << ((channel - 8) * 4);
	}
	else if ((channel >= 4) && (channel <= 8) && (2 == index))
	{
		RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPFEN;
		if (channel <= 5)
		{
			GPIOF->CRL = (GPIOF->CRL & ~(0x0F << ((channel + 2) * 4))) | 
							(uint32_t)stm32_GPIO_ANALOG << ((channel + 2) * 4);
		}
		else
		{
			GPIOF->CRH = (GPIOF->CRH & ~(0x0F << ((channel - 6) * 4))) | 
							(uint32_t)stm32_GPIO_ANALOG << ((channel - 6) * 4);
		}
	}
	
	if (channel > 9)
	{
		tmpreg = adc->SMPR1;
		tmpreg = (tmpreg & ~(((uint32_t) 0x07) << (3 * (channel - 10)))) | 
					((cycles >> 5) << (3 * (channel - 10)));
		adc->SMPR1 = tmpreg;
	}
	else
	{
		tmpreg = adc->SMPR2;
		tmpreg = (tmpreg & ~(((uint32_t) 0x07) << (3 * channel))) | 
					((cycles >> 5) << (3 * channel));
		adc->SMPR2 = tmpreg;
	}
	if (!(adc->CR2 & STM32_ADC_CR2_ADON))
	{
		adc->CR2 |= STM32_ADC_CR2_ADON;
	}
	
	return VSFERR_NONE;
}

vsf_err_t stm32_adc_calibrate(uint8_t index, uint8_t channel)
{
	ADC_TypeDef *adc;
	
#if __VSF_DEBUG__
	if (index >= STM32_ADC_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	adc = (ADC_TypeDef *)stm32_adcs[index];
	
	adc->SQR3 = channel;
	
	adc->CR2 |= STM32_ADC_CR2_ADON;
	adc->CR2 |= STM32_ADC_CR2_RSTCAL;
	while (adc->CR2 & STM32_ADC_CR2_RSTCAL);
	adc->CR2 |= STM32_ADC_CR2_CAL;
	while (adc->CR2 & STM32_ADC_CR2_CAL);
	
	return VSFERR_NONE;
}

uint32_t stm32_adc_get_max_value(uint8_t index)
{
	REFERENCE_PARAMETER(index);
	return (1 << 12);
}

vsf_err_t stm32_adc_start(uint8_t index, uint8_t channel,
							void (callback)(void *, uint16_t), void *param)
{
	ADC_TypeDef *adc;
	
#if __VSF_DEBUG__
	if (index >= STM32_ADC_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	adc = (ADC_TypeDef *)stm32_adcs[index];
	
	stm32_adc_callback[index] = callback;
	stm32_adc_param[index] = param;
	
	adc->SQR3 = channel;
	adc->SR &= ~STM32_ADC_SR_EOC;
	adc->CR1 |= STM32_ADC_CR1_EOCIE;
	adc->CR2 |= STM32_ADC_CR2_ADON;
	return VSFERR_NONE;
}

ROOTFUNC void ADC1_2_IRQHandler(void)
{
	if (ADC1->SR & STM32_ADC_SR_EOC)
	{
		if (stm32_adc_callback[0] != NULL)
		{
			stm32_adc_callback[0](stm32_adc_param[0], ADC1->DR);
		}
	}
	if (ADC2->SR & STM32_ADC_SR_EOC)
	{
		if (stm32_adc_callback[1] != NULL)
		{
			stm32_adc_callback[1](stm32_adc_param[1], ADC2->DR);
		}
	}
}

ROOTFUNC void ADC3_IRQHandler(void)
{
	if (ADC3->SR & STM32_ADC_SR_EOC)
	{
		if (stm32_adc_callback[2] != NULL)
		{
			stm32_adc_callback[2](stm32_adc_param[2], ADC3->DR);
		}
	}
}

#endif
