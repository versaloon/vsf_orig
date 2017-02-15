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
#ifndef __INTERFACE_CONST_H_INCLUDED__
#define __INTERFACE_CONST_H_INCLUDED__

#include "NUC505Series.h"

#define nuc505_SLEEP_WFI			0
#define nuc505_SLEEP_PWRDOWN		1

#define nuc505_GPIO_INFLOAT			0x00
#define nuc505_GPIO_INPU			0x00
#define nuc505_GPIO_INPD			0x00
#define nuc505_GPIO_OUTPP			0x01
#define nuc505_GPIO_OUTOD			0x02

#define nuc505_EINT_ONFALL			0x01
#define nuc505_EINT_ONRISE			0x02
#define nuc505_EINT_ONLOW			0		// not support
#define nuc505_EINT_ONHIGH			0		// not support

#define nuc505_HCD_PORT1			(0x1 << 0)
#define nuc505_HCD_PORT2			(0x1 << 1)

#define nuc505_USART_CLKEN
#define nuc505_USART_STOPBITS_0P5
#define nuc505_USART_STOPBITS_1
#define nuc505_USART_STOPBITS_1P5
#define nuc505_USART_STOPBITS_2
#define nuc505_USART_PARITY_NONE
#define nuc505_USART_PARITY_ODD
#define nuc505_USART_PARITY_EVEN

#define nuc505_SPI_MASTER			0x00
#define nuc505_SPI_SLAVE			(SPI_CTL_SLAVE_Msk)
#define nuc505_SPI_MODE0			(SPI_CTL_TXNEG_Msk)                             /*!< CLKPOL=0; RXNEG=0; TXNEG=1 */
#define nuc505_SPI_MODE1			(SPI_CTL_RXNEG_Msk)                             /*!< CLKPOL=0; RXNEG=1; TXNEG=0 */
#define nuc505_SPI_MODE2			(SPI_CTL_CLKPOL_Msk | SPI_CTL_RXNEG_Msk)        /*!< CLKPOL=1; RXNEG=1; TXNEG=0 */
#define nuc505_SPI_MODE3			(SPI_CTL_CLKPOL_Msk | SPI_CTL_TXNEG_Msk)        /*!< CLKPOL=1; RXNEG=0; TXNEG=1 */
#define nuc505_SPI_MSB_FIRST		0x00
#define nuc505_SPI_LSB_FIRST		(SPI_CTL_LSB_Msk)

/*
#define m45x_SLEEP_WFI			0
#define m45x_SLEEP_PWRDOWN		1


#define m45x_SPI_MASTER
#define m45x_SPI_SLAVE
#define m45x_SPI_MODE0
#define m45x_SPI_MODE1
#define m45x_SPI_MODE2
#define m45x_SPI_MODE3
#define m45x_SPI_MSB_FIRST
#define m45x_SPI_LSB_FIRST

#define m45x_ADC_ALIGNLEFT
#define m45x_ADC_ALIGNRIGHT

#define m45x_EINT_ONFALL			0x01
#define m45x_EINT_ONRISE			0x02
#define m45x_EINT_INT				0x04
#define m45x_EINT_EVT				0x08

#define m45x_SDIO_RESP_NONE
#define m45x_SDIO_RESP_SHORT
#define m45x_SDIO_RESP_LONG
*/

// NUC505 GPIO BITBAND
#define MEM_ADDR(address)  *((volatile unsigned long *) (address))
#define BITBAND(address,bitnum) ((address & 0xF0000000)+0x02000000+((address & 0xFFFFF)<<5)+(bitnum<<2))

#define GPIOA_DOUT             (GPIOA_BASE + 0x08)
#define GPIOB_DOUT             (GPIOB_BASE + 0x08)
#define GPIOC_DOUT             (GPIOC_BASE + 0x08)
#define GPIOD_DOUT             (GPIOD_BASE + 0x08)
#define GPIOA_PIN             (GPIOA_BASE + 0x0C)
#define GPIOB_PIN             (GPIOB_BASE + 0x0C)
#define GPIOC_PIN             (GPIOC_BASE + 0x0C)
#define GPIOD_PIN             (GPIOD_BASE + 0x0C)
#define PA0_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 0))  /*!< Specify PA0 Pin Data Output */
#define PA1_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 1))  /*!< Specify PA1 Pin Data Output */
#define PA2_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 2))  /*!< Specify PA2 Pin Data Output */
#define PA3_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 3))  /*!< Specify PA3 Pin Data Output */
#define PA4_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 4))  /*!< Specify PA4 Pin Data Output */
#define PA5_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 5))  /*!< Specify PA5 Pin Data Output */
#define PA6_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 6))  /*!< Specify PA6 Pin Data Output */
#define PA7_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 7))  /*!< Specify PA7 Pin Data Output */
#define PA8_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 8))  /*!< Specify PA8 Pin Data Output */
#define PA9_DOUT             MEM_ADDR(BITBAND(GPIOA_DOUT, 9))  /*!< Specify PA9 Pin Data Output */
#define PA10_DOUT            MEM_ADDR(BITBAND(GPIOA_DOUT, 10)) /*!< Specify PA10 Pin Data Output */
#define PA11_DOUT            MEM_ADDR(BITBAND(GPIOA_DOUT, 11)) /*!< Specify PA11 Pin Data Output */
#define PA12_DOUT            MEM_ADDR(BITBAND(GPIOA_DOUT, 12)) /*!< Specify PA12 Pin Data Output */
#define PA13_DOUT            MEM_ADDR(BITBAND(GPIOA_DOUT, 13)) /*!< Specify PA13 Pin Data Output */
#define PA14_DOUT            MEM_ADDR(BITBAND(GPIOA_DOUT, 14)) /*!< Specify PA14 Pin Data Output */
#define PA15_DOUT            MEM_ADDR(BITBAND(GPIOA_DOUT, 15)) /*!< Specify PA15 Pin Data Output */

#define PB0_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 0))  /*!< Specify PB0 Pin Data Output */
#define PB1_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 1))  /*!< Specify PB1 Pin Data Output */
#define PB2_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 2))  /*!< Specify PB2 Pin Data Output */
#define PB3_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 3))  /*!< Specify PB3 Pin Data Output */
#define PB4_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 4))  /*!< Specify PB4 Pin Data Output */
#define PB5_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 5))  /*!< Specify PB5 Pin Data Output */
#define PB6_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 6))  /*!< Specify PB6 Pin Data Output */
#define PB7_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 7))  /*!< Specify PB7 Pin Data Output */
#define PB8_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 8))  /*!< Specify PB8 Pin Data Output */
#define PB9_DOUT             MEM_ADDR(BITBAND(GPIOB_DOUT, 9))  /*!< Specify PB9 Pin Data Output */
#define PB10_DOUT            MEM_ADDR(BITBAND(GPIOB_DOUT, 10)) /*!< Specify PB10 Pin Data Output */
#define PB11_DOUT            MEM_ADDR(BITBAND(GPIOB_DOUT, 11)) /*!< Specify PB11 Pin Data Output */
#define PB12_DOUT            MEM_ADDR(BITBAND(GPIOB_DOUT, 12)) /*!< Specify PB12 Pin Data Output */
#define PB13_DOUT            MEM_ADDR(BITBAND(GPIOB_DOUT, 13)) /*!< Specify PB13 Pin Data Output */
#define PB14_DOUT            MEM_ADDR(BITBAND(GPIOB_DOUT, 14)) /*!< Specify PB14 Pin Data Output */
#define PB15_DOUT            MEM_ADDR(BITBAND(GPIOB_DOUT, 15)) /*!< Specify PB15 Pin Data Output */

#define PC0_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 0))  /*!< Specify PC0 Pin Data Output */
#define PC1_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 1))  /*!< Specify PC1 Pin Data Output */
#define PC2_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 2))  /*!< Specify PC2 Pin Data Output */
#define PC3_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 3))  /*!< Specify PC3 Pin Data Output */
#define PC4_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 4))  /*!< Specify PC4 Pin Data Output */
#define PC5_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 5))  /*!< Specify PC5 Pin Data Output */
#define PC6_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 6))  /*!< Specify PC6 Pin Data Output */
#define PC7_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 7))  /*!< Specify PC7 Pin Data Output */
#define PC8_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 8))  /*!< Specify PC8 Pin Data Output */
#define PC9_DOUT             MEM_ADDR(BITBAND(GPIOC_DOUT, 9))  /*!< Specify PC9 Pin Data Output */
#define PC10_DOUT            MEM_ADDR(BITBAND(GPIOC_DOUT, 10)) /*!< Specify PC10 Pin Data Output */
#define PC11_DOUT            MEM_ADDR(BITBAND(GPIOC_DOUT, 11)) /*!< Specify PC11 Pin Data Output */
#define PC12_DOUT            MEM_ADDR(BITBAND(GPIOC_DOUT, 12)) /*!< Specify PC12 Pin Data Output */
#define PC13_DOUT            MEM_ADDR(BITBAND(GPIOC_DOUT, 13)) /*!< Specify PC13 Pin Data Output */
#define PC14_DOUT            MEM_ADDR(BITBAND(GPIOC_DOUT, 14)) /*!< Specify PC14 Pin Data Output */

#define PD0_DOUT             MEM_ADDR(BITBAND(GPIOD_DOUT, 0))  /*!< Specify PD0 Pin Data Output */
#define PD1_DOUT             MEM_ADDR(BITBAND(GPIOD_DOUT, 1))  /*!< Specify PD1 Pin Data Output */
#define PD2_DOUT             MEM_ADDR(BITBAND(GPIOD_DOUT, 2))  /*!< Specify PD2 Pin Data Output */
#define PD3_DOUT             MEM_ADDR(BITBAND(GPIOD_DOUT, 3))  /*!< Specify PD3 Pin Data Output */
#define PD4_DOUT             MEM_ADDR(BITBAND(GPIOD_DOUT, 4))  /*!< Specify PD4 Pin Data Output */

#define PA0_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 0))  /*!< Specify PA0 Pin Data Input */
#define PA1_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 1))  /*!< Specify PA1 Pin Data Input */
#define PA2_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 2))  /*!< Specify PA2 Pin Data Input */
#define PA3_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 3))  /*!< Specify PA3 Pin Data Input */
#define PA4_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 4))  /*!< Specify PA4 Pin Data Input */
#define PA5_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 5))  /*!< Specify PA5 Pin Data Input */
#define PA6_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 6))  /*!< Specify PA6 Pin Data Input */
#define PA7_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 7))  /*!< Specify PA7 Pin Data Input */
#define PA8_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 8))  /*!< Specify PA8 Pin Data Input */
#define PA9_PIN             MEM_ADDR(BITBAND(GPIOA_PIN, 9))  /*!< Specify PA9 Pin Data Input */
#define PA10_PIN            MEM_ADDR(BITBAND(GPIOA_PIN, 10)) /*!< Specify PA10 Pin Data Input */
#define PA11_PIN            MEM_ADDR(BITBAND(GPIOA_PIN, 11)) /*!< Specify PA11 Pin Data Input */
#define PA12_PIN            MEM_ADDR(BITBAND(GPIOA_PIN, 12)) /*!< Specify PA12 Pin Data Input */
#define PA13_PIN            MEM_ADDR(BITBAND(GPIOA_PIN, 13)) /*!< Specify PA13 Pin Data Input */
#define PA14_PIN            MEM_ADDR(BITBAND(GPIOA_PIN, 14)) /*!< Specify PA14 Pin Data Input */
#define PA15_PIN            MEM_ADDR(BITBAND(GPIOA_PIN, 15)) /*!< Specify PA15 Pin Data Input */

#define PB0_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 0))  /*!< Specify PB0 Pin Data Input */
#define PB1_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 1))  /*!< Specify PB1 Pin Data Input */
#define PB2_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 2))  /*!< Specify PB2 Pin Data Input */
#define PB3_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 3))  /*!< Specify PB3 Pin Data Input */
#define PB4_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 4))  /*!< Specify PB4 Pin Data Input */
#define PB5_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 5))  /*!< Specify PB5 Pin Data Input */
#define PB6_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 6))  /*!< Specify PB6 Pin Data Input */
#define PB7_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 7))  /*!< Specify PB7 Pin Data Input */
#define PB8_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 8))  /*!< Specify PB8 Pin Data Input */
#define PB9_PIN             MEM_ADDR(BITBAND(GPIOB_PIN, 9))  /*!< Specify PB9 Pin Data Input */
#define PB10_PIN            MEM_ADDR(BITBAND(GPIOB_PIN, 10)) /*!< Specify PB10 Pin Data Input */
#define PB11_PIN            MEM_ADDR(BITBAND(GPIOB_PIN, 11)) /*!< Specify PB11 Pin Data Input */
#define PB12_PIN            MEM_ADDR(BITBAND(GPIOB_PIN, 12)) /*!< Specify PB12 Pin Data Input */
#define PB13_PIN            MEM_ADDR(BITBAND(GPIOB_PIN, 13)) /*!< Specify PB13 Pin Data Input */
#define PB14_PIN            MEM_ADDR(BITBAND(GPIOB_PIN, 14)) /*!< Specify PB14 Pin Data Input */
#define PB15_PIN            MEM_ADDR(BITBAND(GPIOB_PIN, 15)) /*!< Specify PB15 Pin Data Input */

#define PC0_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 0))  /*!< Specify PC0 Pin Data Input */
#define PC1_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 1))  /*!< Specify PC1 Pin Data Input */
#define PC2_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 2))  /*!< Specify PC2 Pin Data Input */
#define PC3_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 3))  /*!< Specify PC3 Pin Data Input */
#define PC4_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 4))  /*!< Specify PC4 Pin Data Input */
#define PC5_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 5))  /*!< Specify PC5 Pin Data Input */
#define PC6_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 6))  /*!< Specify PC6 Pin Data Input */
#define PC7_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 7))  /*!< Specify PC7 Pin Data Input */
#define PC8_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 8))  /*!< Specify PC8 Pin Data Input */
#define PC9_PIN             MEM_ADDR(BITBAND(GPIOC_PIN, 9))  /*!< Specify PC9 Pin Data Input */
#define PC10_PIN            MEM_ADDR(BITBAND(GPIOC_PIN, 10)) /*!< Specify PC10 Pin Data Input */
#define PC11_PIN            MEM_ADDR(BITBAND(GPIOC_PIN, 11)) /*!< Specify PC11 Pin Data Input */
#define PC12_PIN            MEM_ADDR(BITBAND(GPIOC_PIN, 12)) /*!< Specify PC12 Pin Data Input */
#define PC13_PIN            MEM_ADDR(BITBAND(GPIOC_PIN, 13)) /*!< Specify PC13 Pin Data Input */
#define PC14_PIN            MEM_ADDR(BITBAND(GPIOC_PIN, 14)) /*!< Specify PC14 Pin Data Input */

#define PD0_PIN             MEM_ADDR(BITBAND(GPIOD_PIN, 0))  /*!< Specify PD0 Pin Data Input */
#define PD1_PIN             MEM_ADDR(BITBAND(GPIOD_PIN, 1))  /*!< Specify PD1 Pin Data Input */
#define PD2_PIN             MEM_ADDR(BITBAND(GPIOD_PIN, 2))  /*!< Specify PD2 Pin Data Input */
#define PD3_PIN             MEM_ADDR(BITBAND(GPIOD_PIN, 3))  /*!< Specify PD3 Pin Data Input */
#define PD4_PIN             MEM_ADDR(BITBAND(GPIOD_PIN, 4))  /*!< Specify PD4 Pin Data Input */


#endif	// __INTERFACE_CONST_H_INCLUDED__
