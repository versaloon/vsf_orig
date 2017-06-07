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
#ifndef __VSFHAL_CFG_H__
#define __VSFHAL_CFG_H__

#ifndef CORE_CLKEN
#	define CORE_CLKEN						(GD32F1X0_CLK_HSE | GD32F1X0_CLK_PLL)
#endif
#ifndef CORE_CLKSRC
#	define CORE_CLKSRC						GD32F1X0_CLKSRC_PLL
#endif
#ifndef HSE_FREQ_HZ
#	define HSE_FREQ_HZ						OSC_HZ
#endif
#ifndef HSI_FREQ_HZ
#	define HSI_FREQ_HZ						8000000
#endif
#ifndef CORE_PLL_FREQ_HZ
#	define CORE_PLL_FREQ_HZ					120000000
#endif
#ifndef CORE_MCLK_FREQ_HZ
#	define CORE_MCLK_FREQ_HZ				60000000
#endif
#ifndef CORE_APB_FREQ_HZ
#	define CORE_APB_FREQ_HZ					60000000
#endif
#ifndef CORE_DEBUG
#	define CORE_DEBUG						
#endif
#ifndef CORE_VECTOR_TABLE
#	define CORE_VECTOR_TABLE				FLASH_LOAD_OFFSET
#endif

// SPI config
#define SPI_NUM								2

// USART config
#define USART_NUM							2

#define VSFHAL_FLASH_EN						0
#define VSFHAL_USART_EN						0
#define VSFHAL_SPI_EN						0
#define VSFHAL_ADC_EN						0
#define VSFHAL_GPIO_EN						1
#define VSFHAL_I2C_EN						0
#define VSFHAL_PWM_EN						0
#define VSFHAL_MICROWIRE_EN					0
#define VSFHAL_TIMER_EN						0
#define VSFHAL_EINT_EN						0
#define VSFHAL_EBI_EN						0
#define VSFHAL_SDIO_EN						0
#define VSFHAL_USBD_EN						1

#endif // __VSFHAL_CFG_H__

