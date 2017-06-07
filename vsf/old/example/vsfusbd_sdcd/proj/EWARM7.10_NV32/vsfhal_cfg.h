/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces.h                                              *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    interfaces header file                                    *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

#ifndef CORE_CLKSRC
#	define CORE_CLKSRC						NV32_CLKSRC_FLL
#endif
#ifndef CORE_FLLSRC
#	define CORE_FLLSRC						NV32_FLLSRC_OSC
#endif
#ifndef IRC_FREQ_HZ
#	define IRC_FREQ_HZ						37500
#endif
#ifndef CORE_FLL_FREQ_HZ
#	define CORE_FLL_FREQ_HZ					48000000
#endif
#ifndef CORE_SYS_FREQ_HZ
#	define CORE_SYS_FREQ_HZ					48000000
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
#define VSFHAL_USBD_EN						0
#define VSFHAL_USBDIO_EN					1
#define VSFHAL_CRC_EN						1

#if VSFHAL_USBDIO_EN
#	define VSFHAL_USBDIO_PORT				1
#	define VSFHAL_USBDIO_DP					0
#	define VSFHAL_USBDIO_DM					1
#endif

