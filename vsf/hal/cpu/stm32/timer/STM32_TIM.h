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

vsf_err_t stm32_timer_init(uint8_t index);
vsf_err_t stm32_timer_fini(uint8_t index);
vsf_err_t stm32_timer_config(uint8_t index, uint32_t kHz, uint32_t mode, 
								void (*overflow)(void));
vsf_err_t stm32_timer_start(uint8_t index);
vsf_err_t stm32_timer_stop(uint8_t index);
vsf_err_t stm32_timer_get_count(uint8_t index, uint32_t *count);
vsf_err_t stm32_timer_set_count(uint8_t index, uint32_t count);
vsf_err_t stm32_timer_config_channel(uint8_t index, uint8_t channel, 
										uint32_t mode, void (*callback)(void));
vsf_err_t stm32_timer_get_channel(uint8_t index, uint8_t channel,
									uint32_t *count);
vsf_err_t stm32_timer_set_channel(uint8_t index, uint8_t channel,
									uint32_t count);
