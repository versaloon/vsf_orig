/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces_const.h                                        *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    consts of interface module                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2011-04-08:     created(by SimonQian)                             *
 **************************************************************************/
#ifndef __GD32F1X0_INTERFACE_CONST_H_INCLUDED__
#define __GD32F1_INTERFACE_CONST_H_INCLUDED__

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

#if 0
#define gd32f1x0_USART_MODE0			0x00
#define gd32f1x0_USART_MODE1			0x04
#define gd32f1x0_USART_MODE2			0x08
#define gd32f1x0_USART_MODE3			0x0C
#define gd32f1x0_USART_CLKEN			0x80
#define gd32f1x0_USART_STOPBITS_0P5	0x20
#define gd32f1x0_USART_STOPBITS_1		0x00
#define gd32f1x0_USART_STOPBITS_1P5	0x60
#define gd32f1x0_USART_STOPBITS_2		0x40
#define gd32f1x0_USART_PARITY_NONE		0x00
#define gd32f1x0_USART_PARITY_ODD		0x03
#define gd32f1x0_USART_PARITY_EVEN		0x02

#define gd32f1x0_SPI_MASTER			0x04
#define gd32f1x0_SPI_SLAVE				0x00
#define gd32f1x0_SPI_MODE0				0
#define gd32f1x0_SPI_MODE1				1
#define gd32f1x0_SPI_MODE2				2
#define gd32f1x0_SPI_MODE3				3
#define gd32f1x0_SPI_MSB_FIRST			0x00
#define gd32f1x0_SPI_LSB_FIRST			0x80

#define gd32f1x0_ADC_ALIGNLEFT			0x08
#define gd32f1x0_ADC_ALIGNRIGHT		0x00
#endif

#define gd32f1x0_EINT_ONFALL			0x01
#define gd32f1x0_EINT_ONRISE			0x02
#define gd32f1x0_EINT_ONLEVEL			0x80
#define gd32f1x0_EINT_ONLOW			(gd32f1x0_EINT_ONLEVEL | 0x00)
#define gd32f1x0_EINT_ONHIGH			(gd32f1x0_EINT_ONLEVEL | 0x10)

#define gd32f1x0_SDIO_RESP_NONE		0x00
#define gd32f1x0_SDIO_RESP_SHORT		0x40
#define gd32f1x0_SDIO_RESP_LONG		0xC0

#endif	// __GD32F1X0_INTERFACE_CONST_H_INCLUDED__
