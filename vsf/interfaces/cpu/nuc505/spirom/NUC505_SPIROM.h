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

vsf_err_t nuc505_spirom_init(void);
vsf_err_t nuc505_spirom_fini(void);
vsf_err_t nuc505_spirom_erase(uint32_t addr, uint32_t len);
vsf_err_t nuc505_spirom_write(uint8_t *buf, uint32_t addr, uint32_t len);
vsf_err_t nuc505_spirom_read(uint8_t *buf, uint32_t addr, uint32_t len);

