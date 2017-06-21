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

#include "gd32f1x0.h"
#include "core.h"

#define gd32f1x0_SLEEP_WFI				0
#define gd32f1x0_SLEEP_PWRDOWN			1

#define gd32f1x0_GPIO_INFLOAT			0x00
#define gd32f1x0_GPIO_INPU				0x08
#define gd32f1x0_GPIO_INPD				0x10
#define gd32f1x0_GPIO_OUTPP				0x01
#define gd32f1x0_GPIO_OUTOD				0x05

#define gd32f1x0_GPIO_OD				0x04
#define gd32f1x0_GPIO_AF				0x02
#define gd32f1x0_GPIO_AN				0x03

#define gd32f1x0_ADC_ALIGNLEFT			0x08
#define gd32f1x0_ADC_ALIGNRIGHT			0x00

#define gd32f1x0_PWM_ENABLE				0x01
#define gd32f1x0_PWM_POLARITY_HIGH		0x02
#define gd32f1x0_PWM_POLARITY_LOW		0x00

#if 0
#define gd32f1x0_USART_MODE0			0x00
#define gd32f1x0_USART_MODE1			0x04
#define gd32f1x0_USART_MODE2			0x08
#define gd32f1x0_USART_MODE3			0x0C
#define gd32f1x0_USART_CLKEN			0x80
#define gd32f1x0_USART_STOPBITS_0P5		0x20
#define gd32f1x0_USART_STOPBITS_1		0x00
#define gd32f1x0_USART_STOPBITS_1P5		0x60
#define gd32f1x0_USART_STOPBITS_2		0x40
#define gd32f1x0_USART_PARITY_NONE		0x00
#define gd32f1x0_USART_PARITY_ODD		0x03
#define gd32f1x0_USART_PARITY_EVEN		0x02

#define gd32f1x0_SPI_MASTER				0x04
#define gd32f1x0_SPI_SLAVE				0x00
#define gd32f1x0_SPI_MODE0				0
#define gd32f1x0_SPI_MODE1				1
#define gd32f1x0_SPI_MODE2				2
#define gd32f1x0_SPI_MODE3				3
#define gd32f1x0_SPI_MSB_FIRST			0x00
#define gd32f1x0_SPI_LSB_FIRST			0x80
#endif

#define gd32f1x0_EINT_ONFALL			0x01
#define gd32f1x0_EINT_ONRISE			0x02
#define gd32f1x0_EINT_ONLEVEL			0x80
#define gd32f1x0_EINT_ONLOW				(gd32f1x0_EINT_ONLEVEL | 0x00)
#define gd32f1x0_EINT_ONHIGH			(gd32f1x0_EINT_ONLEVEL | 0x10)

#endif	// __VSFHAL_CONST_H_INCLUDED__
