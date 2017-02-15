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

vsf_err_t nuc505_spi_init(uint8_t index);
vsf_err_t nuc505_spi_fini(uint8_t index);
vsf_err_t nuc505_spi_get_ability(uint8_t index, struct spi_ability_t *ability);
vsf_err_t nuc505_spi_enable(uint8_t index);
vsf_err_t nuc505_spi_disable(uint8_t index);
vsf_err_t nuc505_spi_config(uint8_t index, uint32_t kHz, uint32_t mode);
vsf_err_t nuc505_spi_config_cb(uint8_t index, uint32_t int_priority,
									void *p, void (*onready)(void *));
vsf_err_t nuc505_spi_select(uint8_t index, uint8_t cs);
vsf_err_t nuc505_spi_deselect(uint8_t index, uint8_t cs);
vsf_err_t nuc505_spi_start(uint8_t index, uint8_t *out, uint8_t *in,
							uint32_t len);
uint32_t nuc505_spi_stop(uint8_t index);
