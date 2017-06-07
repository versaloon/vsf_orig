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

#ifndef HSE_VALUE
#define HSE_VALUE						((uint32_t)12000000)
#endif

#define OSC_HZ							HSE_VALUE

#define SYS_MAIN_ADDR					0x08000000
// system locates in the first 64KB area, and followed by application
#define APP_MAIN_ADDR					(SYS_MAIN_ADDR + 512 * 1024)

#define KEY_PORT						0
#define KEY_PIN							4

#define USB_PULLUP_PORT					2
#define USB_PULLUP_PIN					13

// bcmwifi
#define BCMWIFI_PORT_TYPE				BCM_BUS_TYPE_SPI
#define BCMWIFI_PORT					1
#define BCMWIFI_FREQ					18000

#define BCMWIFI_RST_PORT				1
#define BCMWIFI_RST_PIN					11
#define BCMWIFI_WAKEUP_PORT				VSFHAL_DUMMY_PORT
#define BCMWIFI_WAKEUP_PIN				0
#define BCMWIFI_MODE_PORT				VSFHAL_DUMMY_PORT
#define BCMWIFI_MODE_PIN				0

#define BCMWIFI_SPI_CS_PORT				1
#define BCMWIFI_SPI_CS_PIN				10
#define BCMWIFI_EINT_PORT				0
#define BCMWIFI_EINT_PIN				8
#define BCMWIFI_EINT					0x08

#define BCMWIFI_PWRCTRL_PORT			0
#define BCMWIFI_PWRCTRL_PIN				3
