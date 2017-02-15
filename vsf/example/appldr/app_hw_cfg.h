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

#ifndef __APP_HW_CFG_H_INCLUDED__
#define __APP_HW_CFG_H_INCLUDED__

#define APP_BOARD_NAME				"VersaloonProR3"

struct app_hwcfg_t
{
	char *board;
	struct interface_gpio_pin_t key;
	struct
	{
		struct interface_gpio_pin_t pullup;
	} usbd;

#ifdef VSFCFG_FUNC_BCMWIFI
	struct
	{
		int type;
		
		uint8_t index;
		uint32_t freq_khz;
		
		struct interface_gpio_pin_t rst;
		struct interface_gpio_pin_t wakeup;
		struct interface_gpio_pin_t mode;
		struct interface_gpio_pin_t pwrctrl;
		
		struct
		{
			struct interface_gpio_pin_t cs;
			struct interface_gpio_pin_t eint;
			uint8_t eint_idx;
		} spi;
	} bcmwifi;
#endif
};

extern const struct app_hwcfg_t app_hwcfg;

#endif		// __APP_HW_CFG_H_INCLUDED__
