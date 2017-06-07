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
#define STM32_AFIO_MAPR_SPI1			((uint32_t)1 << 0)
#define STM32_AFIO_MAPR_SWJCFG			((uint32_t)7 << 24)

#define STM32_RCC_APB1ENR_SPI2EN		((uint32_t)1 << 14)
#define STM32_RCC_APB1ENR_SPI3EN		((uint32_t)1 << 15)
#define STM32_RCC_APB2ENR_SPI1EN		((uint32_t)1 << 12)
#define STM32_RCC_APB2ENR_IOPAEN		((uint32_t)1 << 2)
#define STM32_RCC_APB2ENR_IOPBEN		((uint32_t)1 << 3)
#define STM32_RCC_APB2ENR_IOPCEN		((uint32_t)1 << 4)
#define STM32_RCC_APB2ENR_IOPDEN		((uint32_t)1 << 5)

#define STM32_RCC_AHBENR_DMA1			((uint32_t)1 << 0)
#define STM32_RCC_AHBENR_DMA2			((uint32_t)1 << 1)

#define STM32_SPI_CR1_SSM				((uint32_t)1 << 9)
#define STM32_SPI_CR1_SSI				((uint32_t)1 << 8)
#define STM32_SPI_CR1_SPE				((uint32_t)1 << 6)

#define STM32_SPI_SR_RXNE				((uint32_t)1 << 0)
#define STM32_SPI_SR_TXE				((uint32_t)1 << 1)

#define STM32_SPI_I2SCFGR				((uint32_t)1 << 11)

#define STM32_DMA_IFCR_TC1				((uint32_t)1 << 1)
#define STM32_DMA_IFCR_TC2				((uint32_t)1 << 5)
#define STM32_DMA_IFCR_TC3				((uint32_t)1 << 9)
#define STM32_DMA_IFCR_TC4				((uint32_t)1 << 13)
#define STM32_DMA_IFCR_TC5				((uint32_t)1 << 17)

#define STM32_DMA_CCR_EN				((uint32_t)1 << 0)

#if VSFHAL_SPI_EN

#include "STM32_SPI.h"

struct
{
	void (*onready)(void *);
	void *param;
	uint32_t len;
} static stm32_spi_param[SPI_NUM];
struct
{
	SPI_TypeDef *spi;
	uint8_t irqntx;
	DMA_Channel_TypeDef *dmatx;
	DMA_Channel_TypeDef *dmarx;
} static const stm32_spi[SPI_NUM] =
{
#if SPI_NUM >= 1
	{
		SPI1,
		DMA1_Channel3_IRQn,
		DMA1_Channel3,
		DMA1_Channel2,
	},
#endif
#if SPI_NUM >= 2
	{
		SPI2,
		DMA1_Channel5_IRQn,
		DMA1_Channel5,
		DMA1_Channel4,
	},
#endif
#if SPI_NUM >= 3
	{
		SPI3,
		DMA2_Channel2_IRQn,
		DMA2_Channel2,
		DMA2_Channel1,
	},
#endif
};

static uint8_t stm32_spi_get_sck_div(uint32_t module_khz, uint32_t khz)
{
	// Set Speed
	if(khz >= module_khz / 2)
	{
		khz = 0;
	}
	else if(khz >= module_khz / 4)
	{
		khz = 1;
	}
	else if(khz >= module_khz / 8)
	{
		khz = 2;
	}
	else if(khz >= module_khz / 16)
	{
		khz = 3;
	}
	else if(khz >= module_khz / 32)
	{
		khz = 4;
	}
	else if(khz > module_khz / 64)
	{
		khz = 5;
	}
	else if(khz > module_khz / 128)
	{
		khz = 6;
	}
	else
	{
		khz = 7;
	}

	return (uint8_t)(khz << 3);
}

vsf_err_t stm32_spi_init(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;
	uint8_t remap_idx = (index >> 4) & 0x0F;
	
#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	switch (spi_idx)
	{
	#if SPI00_ENABLE || SPI10_ENABLE
	case 0:
		RCC->APB2ENR |= STM32_RCC_APB2ENR_SPI1EN;
		RCC->APB2RSTR |= STM32_RCC_APB2ENR_SPI1EN;
		RCC->APB2RSTR &= ~STM32_RCC_APB2ENR_SPI1EN;
		RCC->AHBENR |= STM32_RCC_AHBENR_DMA1;
		switch (remap_idx)
		{
		#if SPI00_ENABLE
		case 0:
			AFIO->MAPR = (AFIO->MAPR & ~STM32_AFIO_MAPR_SPI1)
							| STM32_AFIO_MAPR_SWJCFG;
			RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPAEN;
			break;
		#endif
		#if SPI10_ENABLE
		case 1:
			AFIO->MAPR |= STM32_AFIO_MAPR_SPI1 | STM32_AFIO_MAPR_SWJCFG;
			RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPBEN
			#if SPI10_NSS_ENABLE
							| STM32_RCC_APB2ENR_IOPAEN
			#endif
							;
			break;
		#endif
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI01_ENABLE
	case 1:
		RCC->APB1ENR |= STM32_RCC_APB1ENR_SPI2EN;
		RCC->APB1RSTR |= STM32_RCC_APB1ENR_SPI2EN;
		RCC->APB1RSTR &= ~STM32_RCC_APB1ENR_SPI2EN;
		RCC->AHBENR |= STM32_RCC_AHBENR_DMA1;
		switch (remap_idx)
		{
		case 0:
			RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPBEN;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI02_ENABLE
	case 2:
		RCC->APB1ENR |= STM32_RCC_APB1ENR_SPI3EN;
		RCC->APB1RSTR |= STM32_RCC_APB1ENR_SPI3EN;
		RCC->APB1RSTR &= ~STM32_RCC_APB1ENR_SPI3EN;
		RCC->AHBENR |= STM32_RCC_AHBENR_DMA2;
		switch (remap_idx)
		{
		case 0:
			RCC->APB2ENR |= STM32_RCC_APB2ENR_IOPBEN
			#if SPI02_NSS_ENABLE
							| STM32_RCC_APB2ENR_IOPAEN
			#endif
							;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_fini(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;
	uint8_t remap_idx = (index >> 4) & 0x0F;
	
#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	switch (spi_idx)
	{
	#if SPI00_ENABLE || SPI10_ENABLE
	case 0:
		RCC->APB2ENR &= ~STM32_RCC_APB2ENR_SPI1EN;
		switch (remap_idx)
		{
		#if SPI00_ENABLE
		case 0:
			GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (5 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (5 * 4);
			#if SPI00_MISO_ENABLE
			GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (6 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (6 * 4);
			#endif
			#if SPI00_MOSI_ENABLE
			GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (7 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (7 * 4);
			#endif
			#if SPI00_NSS_ENABLE
			GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (4 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (4 * 4);
			#endif
			break;
		#endif
		#if SPI10_ENABLE
		case 1:
			GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (3 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (3 * 4);
			#if SPI10_MISO_ENABLE
			GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (4 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (4 * 4);
			#endif
			#if SPI10_MOSI_ENABLE
			GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (5 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (5 * 4);
			#endif
			#if SPI10_NSS_ENABLE
			GPIOA->CRH = (GPIOA->CRH & ~(0x0F << ((15 - 8) * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << ((15 - 8) * 4);
			#endif
			break;
		#endif
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI01_ENABLE
	case 1:
		RCC->APB1ENR &= ~STM32_RCC_APB1ENR_SPI2EN;
		switch (remap_idx)
		{
		case 0:
			GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((13 - 8) * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << ((13 - 8) * 4);
			#if SPI01_MISO_ENABLE
			GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((14 - 8) * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << ((14 - 8) * 4);
			#endif
			#if SPI01_MOSI_ENABLE
			GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((15 - 8) * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << ((15 - 8) * 4);
			#endif
			#if SPI01_NSS_ENABLE
			GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((12 - 8) * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << ((12 - 8) * 4);
			#endif
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI02_ENABLE
	case 2:
		RCC->APB1ENR &= ~STM32_RCC_APB1ENR_SPI3EN;
		switch (remap_idx)
		{
		case 0:
			GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (3 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (3 * 4);
			#if SPI02_MISO_ENABLE
			GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (4 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (4 * 4);
			#endif
			#if SPI01_MOSI_ENABLE
			GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (5 * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << (5 * 4);
			#endif
			#if SPI02_NSS_ENABLE
			GPIOA->CRH = (GPIOA->CRH & ~(0x0F << ((15 - 8) * 4))) | 
							(uint32_t)stm32_GPIO_INFLOAT << ((15 - 8) * 4);
			#endif
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_enable(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;
	SPI_TypeDef *spi;
	
#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	spi = stm32_spi[spi_idx].spi;
	spi->CR1 |= STM32_SPI_CR1_SPE;
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_disable(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;
	SPI_TypeDef *spi;
	
#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	spi = stm32_spi[spi_idx].spi;
	spi->CR1 &= ~STM32_SPI_CR1_SPE;
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_get_ability(uint8_t index, struct spi_ability_t *ability)
{
	uint8_t spi_idx = index & 0x0F;
	struct stm32_info_t *info;
	
#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	if (stm32_get_info(&info))
	{
		return VSFERR_FAIL;
	}
	
	switch (spi_idx)
	{
	case 0:
		ability->max_freq_hz = ability->min_freq_hz = info->apb2_freq_hz;
		break;
	case 1:
	case 2:
		ability->max_freq_hz = ability->min_freq_hz = info->apb1_freq_hz;
		break;
	}
	ability->max_freq_hz /= 2;
	ability->min_freq_hz /= 256;
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_config(uint8_t index, uint32_t kHz, uint32_t mode)
{
	uint8_t spi_idx = index & 0x0F;
	uint8_t remap_idx = (index >> 4) & 0x0F;
	SPI_TypeDef *spi;
	uint32_t module_khz;
	struct stm32_info_t *info;
	uint8_t master = mode & stm32_SPI_MASTER;
	
#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	spi = stm32_spi[spi_idx].spi;
	
	if (stm32_get_info(&info))
	{
		return VSFERR_FAIL;
	}
	
	switch (spi_idx)
	{
	#if SPI00_ENABLE || SPI10_ENABLE
	case 0:
		module_khz = info->apb2_freq_hz / 1000;
		switch (remap_idx)
		{
		#if SPI00_ENABLE
		case 0:
			if (master)
			{
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (5 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (5 * 4);
				#if SPI00_MISO_ENABLE
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (6 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (6 * 4);
				#endif
				#if SPI00_MOSI_ENABLE
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (7 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (7 * 4);
				#endif
				#if SPI00_NSS_ENABLE
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (4 * 4))) | 
								(uint32_t)stm32_GPIO_OUTOD << (4 * 4);
				GPIOA->BSRR = 1UL << 4;
				#endif
			}
			else
			{
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (5 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (5 * 4);
				#if SPI00_MISO_ENABLE
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (6 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (6 * 4);
				#endif
				#if SPI00_MOSI_ENABLE
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (7 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (7 * 4);
				#endif
				#if SPI00_NSS_ENABLE
				GPIOA->CRL = (GPIOA->CRL & ~(0x0F << (4 * 4))) | 
								(uint32_t)stm32_GPIO_INP << (4 * 4);
				GPIOA->BSRR = (uint32_t)1 << 4;
				#endif
			}
			break;
		#endif
		#if SPI10_ENABLE
		case 1:
			if (master)
			{
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (3 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (3 * 4);
				#if SPI10_MISO_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (4 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (4 * 4);
				#endif
				#if SPI10_MOSI_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (5 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (5 * 4);
				#endif
				#if SPI10_NSS_ENABLE
				GPIOA->CRH = (GPIOA->CRH & ~(0x0F << ((15 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_OUTOD << ((15 - 8) * 4);
				GPIOA->BSRR = 1UL << 15;
				#endif
			}
			else
			{
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (3 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (3 * 4);
				#if SPI10_MISO_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (4 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (4 * 4);
				#endif
				#if SPI10_MOSI_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (5 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (5 * 4);
				#endif
				#if SPI10_NSS_ENABLE
				GPIOA->CRH = (GPIOA->CRH & ~(0x0F << ((15 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_INP << ((15 - 8) * 4);
				GPIOA->BSRR = (uint32_t)1 << 15;
				#endif
			}
			break;
		#endif
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI01_ENABLE
	case 1:
		module_khz = info->apb1_freq_hz / 1000;
		switch (remap_idx)
		{
		case 0:
			if (master)
			{
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((13 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << ((13 - 8) * 4);
				#if SPI01_MISO_ENABLE
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((14 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << ((14 - 8) * 4);
				#endif
				#if SPI01_MOSI_ENABLE
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((15 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << ((15 - 8) * 4);
				#endif
				#if SPI01_NSS_ENABLE
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((12 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_OUTOD << ((12 - 8) * 4);
				GPIOB->BSRR = 1UL << 12;
				#endif
			}
			else
			{
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((13 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << ((13 - 8) * 4);
				#if SPI01_MISO_ENABLE
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((14 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << ((14 - 8) * 4);
				#endif
				#if SPI01_MOSI_ENABLE
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((15 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << ((15 - 8) * 4);
				#endif
				#if SPI01_NSS_ENABLE
				GPIOB->CRH = (GPIOB->CRH & ~(0x0F << ((12 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_INP << ((12 - 8) * 4);
				GPIOB->BSRR = (uint32_t)1 << 12;
				#endif
			}
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI02_ENABLE
	case 2:
		module_khz = info->apb1_freq_hz / 1000;
		switch (remap_idx)
		{
		case 0:
			if (master)
			{
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (3 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (3 * 4);
				#if SPI02_MISO_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (4 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (4 * 4);
				#endif
				#if SPI02_MOSI_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (5 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (5 * 4);
				#endif
				#if SPI02_NSS_ENABLE
				GPIOA->CRH = (GPIOA->CRH & ~(0x0F << ((15 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_OUTOD << ((15 - 8) * 4);
				GPIOA->BSRR = 1UL << 15;
				#endif
			}
			else
			{
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (3 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (3 * 4);
				#if SPI02_MISO_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (4 * 4))) | 
								(uint32_t)stm32_GPIO_AFPP << (4 * 4);
				#endif
				#if SPI02_MOSI_ENABLE
				GPIOB->CRL = (GPIOB->CRL & ~(0x0F << (5 * 4))) | 
								(uint32_t)stm32_GPIO_INFLOAT << (5 * 4);
				#endif
				#if SPI02_NSS_ENABLE
				GPIOA->CRH = (GPIOA->CRH & ~(0x0F << ((15 - 8) * 4))) | 
								(uint32_t)stm32_GPIO_INP << ((15 - 8) * 4);
				GPIOA->BSRR = (uint32_t)1 << 15;
				#endif
			}
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	default:
		return VSFERR_NOT_SUPPORT;
	}
	
	spi->CR1 &= ~STM32_SPI_CR1_SPE;
	spi->CR1 = mode | STM32_SPI_CR1_SSM | 
				stm32_spi_get_sck_div(module_khz, kHz);
	if (master)
	{
		spi->CR1 |= STM32_SPI_CR1_SSI;
	}
	spi->I2SCFGR &= ~STM32_SPI_I2SCFGR;
	spi->CR1 |= STM32_SPI_CR1_SPE;
	spi->CR2 |= 3ul;
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_config_cb(uint8_t index, uint32_t int_priority,
									void *p, void (*onready)(void *))
{
	uint8_t spi_idx = index & 0x0F;
	uint8_t irqn;
	
#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	stm32_spi_param[spi_idx].onready = onready;
	stm32_spi_param[spi_idx].param = p;
	
	if (onready != NULL)
	{
		irqn = stm32_spi[spi_idx].irqntx;
		NVIC->IP[irqn] = int_priority;
		NVIC->ISER[irqn >> 0x05] = 1UL << (irqn & 0x1F);
	}
	else
	{
		irqn = stm32_spi[spi_idx].irqntx;
		NVIC->ICER[irqn >> 0x05] = 1UL << (irqn & 0x1F);
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_select(uint8_t index, uint8_t cs)
{
	uint8_t spi_idx = index & 0x0F;
	uint8_t remap_idx = (index >> 4) & 0x0F;
	
#if __VSF_DEBUG__
	if ((spi_idx >= SPI_NUM) || (cs > 0))
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	switch (spi_idx)
	{
	#if (SPI00_ENABLE && SPI00_NSS_ENABLE) || (SPI10_ENABLE && SPI10_NSS_ENABLE)
	case 0:
		switch (remap_idx)
		{
		#if SPI00_ENABLE && SPI00_NSS_ENABLE
		case 0:
			GPIOA->BRR = 1UL << 4;
			break;
		#endif
		#if SPI10_ENABLE && SPI10_NSS_ENABLE
		case 1:
			GPIOA->BRR = 1UL << 15;
			break;
		#endif
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI01_ENABLE && SPI01_NSS_ENABLE
	case 1:
		switch (remap_idx)
		{
		case 0:
			GPIOB->BRR = 1UL << 12;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI02_ENABLE && SPI02_NSS_ENABLE
	case 2:
		switch (remap_idx)
		{
		case 0:
			GPIOA->BRR = 1UL << 15;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	default:
		return VSFERR_NOT_SUPPORT;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_spi_deselect(uint8_t index, uint8_t cs)
{
	uint8_t spi_idx = index & 0x0F;
	uint8_t remap_idx = (index >> 4) & 0x0F;
	
#if __VSF_DEBUG__
	if ((spi_idx >= SPI_NUM) || (cs > 0))
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	switch (spi_idx)
	{
	#if (SPI00_ENABLE && SPI00_NSS_ENABLE) || (SPI10_ENABLE && SPI10_NSS_ENABLE)
	case 0:
		switch (remap_idx)
		{
		#if SPI00_ENABLE && SPI00_NSS_ENABLE
		case 0:
			GPIOA->BSRR = 1UL << 4;
			break;
		#endif
		#if SPI10_ENABLE && SPI10_NSS_ENABLE
		case 1:
			GPIOA->BSRR = 1UL << 15;
			break;
		#endif
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI01_ENABLE && SPI01_NSS_ENABLE
	case 1:
		switch (remap_idx)
		{
		case 0:
			GPIOB->BSRR = 1UL << 12;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	#if SPI02_ENABLE && SPI02_NSS_ENABLE
	case 2:
		switch (remap_idx)
		{
		case 0:
			GPIOA->BSRR = 1UL << 15;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		break;
	#endif
	default:
		return VSFERR_NOT_SUPPORT;
	}
	return VSFERR_NONE;
}

extern uint32_t stm32_dma_dummy;
vsf_err_t stm32_spi_start(uint8_t index, uint8_t *out, uint8_t *in,
							uint32_t len)
{
	uint8_t spi_idx = index & 0x0F;
	SPI_TypeDef *spi;
	DMA_Channel_TypeDef *dmatx, *dmarx;
	
#if __VSF_DEBUG__
	if ((spi_idx >= SPI_NUM) || (len > 0xFFFF))
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	spi = stm32_spi[spi_idx].spi;
	dmatx = stm32_spi[spi_idx].dmatx;
	dmarx = stm32_spi[spi_idx].dmarx;
	
	switch (spi_idx)
	{
	#if SPI00_ENABLE || SPI10_ENABLE
	case 0:
		DMA1->IFCR = STM32_DMA_IFCR_TC2 | STM32_DMA_IFCR_TC3;
		break;
	#endif
	#if SPI01_ENABLE
	case 1:
		DMA1->IFCR = STM32_DMA_IFCR_TC5 | STM32_DMA_IFCR_TC4;
		break;
	#endif
	#if SPI01_ENABLE
	case 2:
		DMA2->IFCR = STM32_DMA_IFCR_TC2 | STM32_DMA_IFCR_TC1;
		break;
	#endif
	}
	
	// for stm32, dmatx and dmarx can not enable at the same time
	stm32_spi_param[spi_idx].len = len;
	if (in != NULL)
	{
		dmarx->CCR = 0x00000080;
		dmarx->CNDTR = len;
		dmarx->CPAR = (uint32_t)&spi->DR;
		dmarx->CMAR = (uint32_t)in;
		stm32_dma_dummy = spi->DR;
		dmarx->CCR |= STM32_DMA_CCR_EN;
	}
	if (out != NULL)
	{
		dmatx->CCR = 0x00000092;
		dmatx->CNDTR = len;
		dmatx->CPAR = (uint32_t)&spi->DR;
		dmatx->CMAR = (uint32_t)out;
		dmatx->CCR |= STM32_DMA_CCR_EN;
	}
	else if (len > 0)
	{
		dmatx->CCR = 0x00000012;
		dmatx->CNDTR = len;
		dmatx->CPAR = (uint32_t)&spi->DR;
		dmatx->CMAR = (uint32_t)&stm32_dma_dummy;
		dmatx->CCR |= STM32_DMA_CCR_EN;
	}
	
	return VSFERR_NONE;
}

uint32_t stm32_spi_stop(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;
	DMA_Channel_TypeDef *dmatx, *dmarx;
	
#if __VSF_DEBUG__
	if ((spi_idx >= SPI_NUM) || (len > 0xFFFF))
	{
		return 0;
	}
#endif
	
	dmatx = stm32_spi[spi_idx].dmatx;
	dmarx = stm32_spi[spi_idx].dmarx;
	
	dmatx->CCR &= ~1;
	dmarx->CCR &= ~1;
	
	return stm32_spi_param[spi_idx].len - dmatx->CNDTR;
}

#if SPI_NUM >= 1
ROOTFUNC void DMA1_Channel3_IRQHandler(void)
{
	if (DMA1->ISR & (1 << (1 + (4 * (3 - 1)))))
	{
		if (stm32_spi_param[0].onready != NULL)
		{
			stm32_spi_param[0].onready(stm32_spi_param[0].param);
		}
	}
	DMA1->IFCR = 0x0000000F << (4 * (3 - 1));
}
#endif

#if SPI_NUM >= 2
ROOTFUNC void DMA1_Channel5_IRQHandler(void)
{
	if (DMA1->ISR & (1 << (1 + (4 * (5 - 1)))))
	{
		if (stm32_spi_param[1].onready != NULL)
		{
			stm32_spi_param[1].onready(stm32_spi_param[1].param);
		}
	}
	DMA1->IFCR = 0x00000001 << (4 * (5 - 1));
}
#endif

#if SPI_NUM >= 3
ROOTFUNC void DMA2_Channel2_IRQHandler(void)
{
	if (DMA2->ISR & (1 << (1 + (4 * (2 - 1)))))
	{
		if (stm32_spi_param[2].onready != NULL)
		{
			stm32_spi_param[2].onready(stm32_spi_param[0].param);
		}
	}
	DMA2->IFCR = 0x00000001 << (4 * (2 - 1));
}
#endif

#endif
