/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       GPIO.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    GPIO interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

vsf_err_t nuc505_eint_init(uint32_t index);
vsf_err_t nuc505_eint_fini(uint32_t index);
vsf_err_t nuc505_eint_config(uint32_t index, uint32_t type,
			uint32_t int_priority, void *param, void (*callback)(void *param));
vsf_err_t nuc505_eint_enable(uint32_t index);
vsf_err_t nuc505_eint_disable(uint32_t index);
