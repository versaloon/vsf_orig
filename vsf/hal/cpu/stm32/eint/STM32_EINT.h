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

vsf_err_t stm32_eint_init(uint32_t index);
vsf_err_t stm32_eint_fini(uint32_t index);
vsf_err_t stm32_eint_config(uint32_t index, uint32_t type,
			uint32_t int_priority, void *param, void (*callback)(void *param));
vsf_err_t stm32_eint_enable(uint32_t index);
vsf_err_t stm32_eint_disable(uint32_t index);
vsf_err_t stm32_eint_trigger(uint32_t index);
