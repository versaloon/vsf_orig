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

#ifndef __VSFHAL_CONST_H_INCLUDED__
#define __VSFHAL_CONST_H_INCLUDED__

#include "stm32f10x.h"
#include "core.h"

#define stm32_SLEEP_WFI				0
#define stm32_SLEEP_PWRDOWN			1

#define stm32_USART_MODE0			0x00
#define stm32_USART_MODE1			0x04
#define stm32_USART_MODE2			0x08
#define stm32_USART_MODE3			0x0C
#define stm32_USART_CLKEN			0x80
#define stm32_USART_STOPBITS_0P5	0x20
#define stm32_USART_STOPBITS_1		0x00
#define stm32_USART_STOPBITS_1P5	0x60
#define stm32_USART_STOPBITS_2		0x40
#define stm32_USART_PARITY_NONE		0x00
#define stm32_USART_PARITY_ODD		0x03
#define stm32_USART_PARITY_EVEN		0x02

#define stm32_SPI_MASTER			0x04
#define stm32_SPI_SLAVE				0x00
#define stm32_SPI_MODE0				0
#define stm32_SPI_MODE1				1
#define stm32_SPI_MODE2				2
#define stm32_SPI_MODE3				3
#define stm32_SPI_MSB_FIRST			0x00
#define stm32_SPI_LSB_FIRST			0x80

#define stm32_ADC_ALIGNLEFT			0x08
#define stm32_ADC_ALIGNRIGHT		0x00

#define stm32_GPIO_INFLOAT			0x04
#define stm32_GPIO_INPU				0x88
#define stm32_GPIO_INPD				0x08
#define stm32_GPIO_INP				0x08
#define stm32_GPIO_OUTPP			0x01
#define stm32_GPIO_OUTOD			0x05
#define stm32_GPIO_ANALOG			0x00
#define stm32_GPIO_AFOD				0x0F
#define stm32_GPIO_AFPP				0x0B

#define stm32_EINT_ONFALL			0x01
#define stm32_EINT_ONRISE			0x02
#define stm32_EINT_ONLEVEL			0x80
#define stm32_EINT_ONLOW			(stm32_EINT_ONLEVEL | 0x00)
#define stm32_EINT_ONHIGH			(stm32_EINT_ONLEVEL | 0x10)

#define stm32_SDIO_RESP_NONE		0x00
#define stm32_SDIO_RESP_SHORT		0x40
#define stm32_SDIO_RESP_LONG		0xC0

#endif	// __VSFHAL_CONST_H_INCLUDED__
