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

// Core config for clocks
#ifndef CORE_CLKEN
#	define CORE_CLKEN						(NUC505_CLK_HXT | NUC505_CLK_LIRC | NUC505_CLK_PLL)
#endif
#ifndef CORE_HCLKSRC
#	define CORE_HCLKSRC						NUC505_CLK_PLL
#endif
#ifndef OSC_FREQ_HZ
#	define OSC_FREQ_HZ						(12 * 1000 * 1000)
#endif
#ifndef OSC32_FREQ_HZ
#	define OSC32_FREQ_HZ					(32768)
#endif
#ifndef LIRC_FREQ_HZ
#	define LIRC_FREQ_HZ						(32768)
#endif
#ifndef CORE_PLL_FREQ_HZ
#	define CORE_PLL_FREQ_HZ					(480 * 1000 * 1000)
#endif
#ifndef CORE_APLL_FREQ_HZ
#	define CORE_APLL_FREQ_HZ				(0)
#endif
#ifndef CPU_FREQ_HZ
#	define CPU_FREQ_HZ						(96 * 1000 * 1000)
#endif
#ifndef HCLK_FREQ_HZ
#	define HCLK_FREQ_HZ						CPU_FREQ_HZ
#endif
#ifndef PCLK_FREQ_HZ
#	define PCLK_FREQ_HZ						(96 * 1000 * 1000)
#endif
#ifndef CORE_VECTOR_TABLE
#	define CORE_VECTOR_TABLE				(0x00000000)
#endif

// EINT
#define EINT_NUM							4

// SPI config
#define SPI_NUM								2
#define SPI0_ENABLE							0
#define SPI1_ENABLE							1

// I2C config
#define I2C_NUM								2
#define I2C0_ENABLE							0
#	define I2C00_SCL_ENABLE					0
#	define I2C00_SDA_ENABLE					0
#	define I2C10_SCL_ENABLE					0
#	define I2C10_SDA_ENABLE					0
#	define I2C20_SCL_ENABLE					0
#	define I2C20_SDA_ENABLE					0
#define I2C1_ENABLE							0
#	define I2C01_SCL_ENABLE					0
#	define I2C01_SDA_ENABLE					0
#	define I2C11_SCL_ENABLE					0
#	define I2C11_SDA_ENABLE					0
#	define I2C21_SCL_ENABLE					0
#	define I2C21_SDA_ENABLE					0

// USART config
#define USART_NUM							3
#define USART0_INT_EN						1
#define USART1_INT_EN						0
#define USART2_INT_EN						0
#define USART00_ENABLE						1
#	define USART00_TX_ENABLE				1
#	define USART00_RX_ENABLE				1
#define USART01_ENABLE						0
#	define USART01_TX_ENABLE				0
#	define USART01_RX_ENABLE				0
#define USART11_ENABLE						0
#	define USART11_CTS_ENABLE				0
#	define USART11_RTS_ENABLE				0
#	define USART11_TX_ENABLE				0
#	define USART11_RX_ENABLE				0
#define USART02_ENABLE						0
#	define USART02_CTS_ENABLE				0
#	define USART02_RTS_ENABLE				0
#	define USART02_TX_ENABLE				0
#	define USART02_RX_ENABLE				0

#define VSFHAL_CONST_EN						1
#define VSFHAL_FLASH_EN						0
#define VSFHAL_USART_EN						0
#define VSFHAL_SPI_EN						1
#define VSFHAL_ADC_EN						0
#define VSFHAL_GPIO_EN						1
#define VSFHAL_I2C_EN						0
#define VSFHAL_PWM_EN						0
#define VSFHAL_MICROWIRE_EN					0
#define VSFHAL_TIMER_EN						0
#define VSFHAL_EINT_EN						1
#define VSFHAL_EBI_EN						0
#define VSFHAL_SDIO_EN						1
#define VSFHAL_USBD_EN						1
#define VSFHAL_OHCI_EN						0
