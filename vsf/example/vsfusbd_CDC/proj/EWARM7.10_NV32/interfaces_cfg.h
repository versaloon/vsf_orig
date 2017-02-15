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
#	define CORE_CLKSRC						NV32_CLKSRC_OSC
#endif
#ifndef CORE_FLLSRC
#	define CORE_FLLSRC						NV32_FLLSRC_IRC
#endif
#ifndef IRC_FREQ_HZ
#	define IRC_FREQ_HZ						37500
#endif
#ifndef CORE_FLL_FREQ_HZ
#	define CORE_FLL_FREQ_HZ					48000000
#endif
#ifndef CORE_SYS_FREQ_HZ
#	define CORE_SYS_FREQ_HZ					24000000
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


#define IFS_FLASH_EN						0
#define IFS_USART_EN						0
#define IFS_SPI_EN							0
#define IFS_ADC_EN							0
#define IFS_GPIO_EN							1
#define IFS_I2C_EN							0
#define IFS_PWM_EN							0
#define IFS_MICROWIRE_EN					0
#define IFS_TIMER_EN						0
#define IFS_EINT_EN							0
#define IFS_EBI_EN							0
#define IFS_SDIO_EN							0
#define IFS_USBD_EN							0
#define IFS_USBDIO_EN						1
#define IFS_CRC_EN							1

#if IFS_USBDIO_EN
#	define IFS_USBDIO_PORT					1
#	define IFS_USBDIO_DP					0
#	define IFS_USBDIO_DM					1
#endif

#define IFS_TICKCLK_NOINT
