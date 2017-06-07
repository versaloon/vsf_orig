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
#	define CORE_PLL_FREQ_HZ					48000000
#endif
#ifndef CORE_MCLK_FREQ_HZ
#	define CORE_MCLK_FREQ_HZ				48000000
#endif
#ifndef CORE_APB_FREQ_HZ
#	define CORE_APB_FREQ_HZ					48000000
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

#if VSFHAL_USBDIO_EN
#	define VSFHAL_USBDIO_PORT				0
#	define VSFHAL_USBDIO_DP					11
#	define VSFHAL_USBDIO_DM					12
#endif

