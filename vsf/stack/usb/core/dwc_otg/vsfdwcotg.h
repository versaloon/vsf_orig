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
#ifndef __VSFDWCOTG_H___
#define __VSFDWCOTG_H___

/**************************************************************************
 * export data
 **************************************************************************/

struct vsfdwcotg_hcd_param_t
{
	uint32_t index;

	uint8_t speed;
	uint8_t dma_en;
	uint8_t ulpi_en;
	uint8_t vbus_en;
	uint8_t hc_amount;
	uint8_t iso_packet_limit;

	uint16_t fifo_size;
	uint16_t periodic_out_packet_size_max;
	uint16_t non_periodic_out_packet_size_max;
	uint16_t in_packet_size_max;
};

extern const struct vsfusbh_hcddrv_t vsfdwcotgh_drv;

#endif // __VSFDWCOTG_H___
