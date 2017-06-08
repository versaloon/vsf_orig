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

#define OSC_HZ							((uint32_t)8000000)

#define USB_PULLUP_PORT					1
#define USB_PULLUP_PIN					5
