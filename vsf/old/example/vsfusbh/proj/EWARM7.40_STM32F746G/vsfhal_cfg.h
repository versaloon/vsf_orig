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
#	define CORE_CLKEN						(STM32F7_CLK_HSI | STM32F7_CLK_HSE | STM32F7_CLK_PLL)
#endif
#ifndef CORE_HCLKSRC
#	define CORE_HCLKSRC						STM32F7_HCLKSRC_PLL
#endif
#ifndef CORE_PLLSRC
#	define CORE_PLLSRC						STM32F7_PLLSRC_HSE
#endif
#ifndef HSI_FREQ_HZ
#	define HSI_FREQ_HZ						(16 * 1000 * 1000)
#endif
#ifndef HSE_FREQ_HZ
#	define HSE_FREQ_HZ						(25 * 1000 * 1000)
#endif
#ifndef CORE_PLL_FREQ_HZ
#	define CORE_PLL_FREQ_HZ					(192 * 1000 * 1000)
#endif
#ifndef CORE_HCLK_FREQ_HZ
#	define CORE_HCLK_FREQ_HZ				(192 * 1000 * 1000)
#endif
#ifndef CORE_PCLK1_FREQ_HZ
#	define CORE_PCLK1_FREQ_HZ				(24 * 1000 * 1000)
#endif
#ifndef CORE_PCLK2_FREQ_HZ
#	define CORE_PCLK2_FREQ_HZ				(24 * 1000 * 1000)
#endif
#ifndef CORE_VECTOR_TABLE
#	define CORE_VECTOR_TABLE				(0x08000000)
#endif

#define VSFHAL_FLASH_EN						0
#define VSFHAL_USART_EN						0
#define VSFHAL_SPI_EN						0
#define VSFHAL_ADC_EN						0
#define VSFHAL_GPIO_EN						0
#define VSFHAL_I2C_EN						0
#define VSFHAL_PWM_EN						0
#define VSFHAL_MICROWIRE_EN					0
#define VSFHAL_TIMER_EN						0
#define VSFHAL_EINT_EN						0
#define VSFHAL_EBI_EN						0
#define VSFHAL_SDIO_EN						0
#define VSFHAL_USBD_EN						0
#define VSFHAL_HCD_EN						1

