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

#include "NUC472_442.h"

#define VSF_BASE_ADDR				0x00000200

#define nuc400_SLEEP_WFI			0
#define nuc400_SLEEP_PWRDOWN		1

#define nuc400_USART_MODE0			
#define nuc400_USART_MODE1			
#define nuc400_USART_MODE2			
#define nuc400_USART_MODE3			
#define nuc400_USART_CLKEN			
#define nuc400_USART_STOPBITS_0P5	
#define nuc400_USART_STOPBITS_1		
#define nuc400_USART_STOPBITS_1P5	
#define nuc400_USART_STOPBITS_2		
#define nuc400_USART_PARITY_NONE	0x0
#define nuc400_USART_PARITY_ODD		0x2
#define nuc400_USART_PARITY_EVEN	0x3

#define nuc400_SPI_MASTER			
#define nuc400_SPI_SLAVE			
#define nuc400_SPI_MODE0			
#define nuc400_SPI_MODE1			
#define nuc400_SPI_MODE2			
#define nuc400_SPI_MODE3			
#define nuc400_SPI_MSB_FIRST		
#define nuc400_SPI_LSB_FIRST		

#define nuc400_ADC_ALIGNLEFT		
#define nuc400_ADC_ALIGNRIGHT		

#define nuc400_GPIO_INFLOAT			0x00
#define nuc400_GPIO_INPU			0x00
#define nuc400_GPIO_INPD			0x00
#define nuc400_GPIO_OUTPP			0x01
#define nuc400_GPIO_OUTOD			0x02

#define nuc400_EINT_ONFALL			
#define nuc400_EINT_ONRISE			
#define nuc400_EINT_INT				
#define nuc400_EINT_EVT				

#define nuc400_SDIO_RESP_NONE		
#define nuc400_SDIO_RESP_SHORT		
#define nuc400_SDIO_RESP_LONG		

#endif	// __VSFHAL_CONST_H_INCLUDED__
