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
#ifndef __NV32_INTERFACE_CONST_H_INCLUDED__
#define __NV32_INTERFACE_CONST_H_INCLUDED__

#include "core.h"
#include "NV32.h"

#define nv32_SLEEP_WFI				0
#define nv32_SLEEP_PWRDOWN			1

#define nv32_GPIO_INFLOAT			0x00
#define nv32_GPIO_INPU				0x02
#define nv32_GPIO_INPD				0x00
#define nv32_GPIO_OUTPP				0x01
#define nv32_GPIO_OUTOD				0x03

#endif	// __NV32_INTERFACE_CONST_H_INCLUDED__
