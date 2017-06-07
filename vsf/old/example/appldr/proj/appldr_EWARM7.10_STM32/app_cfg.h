/**************************************************************************
 *  Copyright (C) 2008 - 2012 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    VSF                                                       *
 *  File:       app_cfg.h                                                 *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    configuration file                                        *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

// hardware config file
#include "hw_cfg_STM32.h"

// compiler config
#include "compiler.h"

#define APPCFG_BUFMGR_SIZE				(60 * 1024)
#define APPCFG_MODULES_ADDR				0x08008000
