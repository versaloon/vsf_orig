/**************************************************************************
 *  Copyright (C) 2008 - 2012 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       hw_cfg_VSFCore_STM32.h                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    hardware configuration file for VSFCore_STM32             *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 *      2008-11-22:     rewrite GPIO_Dir(by SimonQian)                    *
 **************************************************************************/

#define SYS_MAIN_ADDR					0
#define APP_MAIN_ADDR					(SYS_MAIN_ADDR + 512 * 1024)

// USB
#define USB_PULLUP_PORT					0xFF
#define USB_PULLUP_PIN					0

#define KEY_PORT						VSFHAL_DUMMY_PORT
#define KEY_PIN							0

// bcm wifi
#define BCM_PORT_TYPE					BCM_BUS_TYPE_SDIO

#define BCM_PORT						1
#define BCM_FREQ						32000 //48000

#if BCM_PORT_TYPE == BCM_BUS_TYPE_SPI
#define BCM_RST_PORT					2
#define BCM_RST_PIN						14
#define BCM_WAKEUP_PORT					2
#define BCM_WAKEUP_PIN					13
#define BCM_MODE_PORT					2
#define BCM_MODE_PIN					12

#define BCM_SPI_CS_PORT					1
#define BCM_SPI_CS_PIN					10
#define BCM_EINT_PORT					2
#define BCM_EINT_PIN					5
#define BCM_EINT						0x52
#endif

#define BCM_PWRCTRL_PORT				VSFHAL_DUMMY_PORT
#define BCM_PWRCTRL_PIN					0

#define BCM_CFG_FWADDR					0x00020000
