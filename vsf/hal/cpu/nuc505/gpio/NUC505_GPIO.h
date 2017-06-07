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

vsf_err_t nuc505_gpio_init(uint8_t index);
vsf_err_t nuc505_gpio_fini(uint8_t index);
vsf_err_t nuc505_gpio_config_pin(uint8_t index, uint8_t pin_idx, uint32_t mode);
vsf_err_t nuc505_gpio_config(uint8_t index, uint32_t pin_mask, uint32_t io,
							uint32_t pull_en_mask, uint32_t input_pull_mask);
vsf_err_t nuc505_gpio_set(uint8_t index, uint32_t pin_mask);
vsf_err_t nuc505_gpio_clear(uint8_t index, uint32_t pin_mask);
vsf_err_t nuc505_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value);
vsf_err_t nuc505_gpio_in(uint8_t index, uint32_t pin_mask, uint32_t *value);
uint32_t nuc505_gpio_get(uint8_t index, uint32_t pin_mask);
