/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

vsf_err_t stm32_spi_init(uint8_t index);
vsf_err_t stm32_spi_fini(uint8_t index);
vsf_err_t stm32_spi_get_ability(uint8_t index, struct spi_ability_t *ability);
vsf_err_t stm32_spi_enable(uint8_t index);
vsf_err_t stm32_spi_disable(uint8_t index);
vsf_err_t stm32_spi_config(uint8_t index, uint32_t kHz, uint32_t mode);
vsf_err_t stm32_spi_config_cb(uint8_t index, uint32_t int_priority,
									void *p, void (*onready)(void *));
vsf_err_t stm32_spi_select(uint8_t index, uint8_t cs);
vsf_err_t stm32_spi_deselect(uint8_t index, uint8_t cs);
vsf_err_t stm32_spi_start(uint8_t index, uint8_t *out, uint8_t *in,
							uint32_t len);
uint32_t stm32_spi_stop(uint8_t index);
