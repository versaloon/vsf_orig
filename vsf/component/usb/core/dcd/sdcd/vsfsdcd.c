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

#include "vsf.h"

#ifndef VSFSDCD_CFG_EP_NUM
#	define VSFSDCD_CFG_EP_NUM				8
#endif

const uint8_t vsfsdcd_ep_num = VSFSDCD_CFG_EP_NUM;
struct vsfhal_usbd_callback_t vsfsdcd_callback;

struct vsfsdcd_ep_t
{
	uint8_t addr;			// MSB is IN/OUT
	unsigned toggle : 1;
	unsigned transfered : 1;
	uint16_t size;
	uint16_t count;
	uint8_t token;
	// SYNC + PID + DATA + CRC16
	uint8_t buf[1 + 1 + 8 + 2];
} static vsfsdcd_ep[VSFSDCD_CFG_EP_NUM];
static uint8_t vsfsdcd_addr;
static volatile uint16_t vsfsdcd_fn;
#define SDCD_ON_RST							(1 << 0)
#define SDCD_ON_SOF							(1 << 1)
#define SDCD_ON_SETUP						(1 << 2)
#define SDCD_ON_IN							(1 << 3)
#define SDCD_ON_OUT							(1 << 4)
static volatile uint32_t vsfsdcd_evt;
enum vsfsdcd_state_t
{
	SDCD_IDLE,
	SDCD_SETUP,
	SDCD_OUT,
	SDCD_IN,
} static volatile vsfsdcd_state;

static struct vsfsdcd_ep_t * vsfsdcd_find_ep(uint8_t idx)
{
	for (int i = 0; i < dimof(vsfsdcd_ep); i++)
		if (idx == vsfsdcd_ep[i].addr)
			return &vsfsdcd_ep[i];
	return NULL;
}

static inline vsf_err_t vsfsdcd_send_token(uint8_t token)
{
	uint8_t buf[] = {0x80, token};
	return vsfhal_usbdio_tx(buf, sizeof(buf));
}

static inline vsf_err_t vsfsdcd_send_data(struct vsfsdcd_ep_t *ep)
{
	ep->buf[1] = ep->toggle ? USB_PID_DATA1 : USB_PID_DATA0;
	ep->toggle = !ep->toggle;
	return vsfhal_usbdio_tx(ep->buf, ep->count);
}

static void vsfsdcd_reset(void)
{
	vsfsdcd_addr = 0;
	vsfsdcd_evt = 0;
	vsfsdcd_state = SDCD_IDLE;
	memset(vsfsdcd_ep, 0, sizeof(vsfsdcd_ep));
	for (int i = 0; i < dimof(vsfsdcd_ep); i++)
		vsfsdcd_ep[i].addr = 0xFF;
}

static void vsfsdcd_onrx(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len)
{
	uint8_t addr, epno, toggle;
	static uint8_t curep = 0;
	struct vsfsdcd_ep_t *ep;

	if (evt == USBDIO_EVT_RST)
	{
		vsfsdcd_reset();
		vsfsdcd_evt |= SDCD_ON_RST;
		return;
	}
	else if (evt == USBDIO_EVT_SOF)
	{
		vsfsdcd_evt |= SDCD_ON_SOF;
		return;
	}

	// first byte is SYNC
	if (buf[0] != 0x80)
		return;
	buf++;

	// parse addr and ep
	addr = buf[1] & 0x7F;
	epno = ((buf[1] >> 7) | (buf[2] << 1)) & 0x0F;

	switch (buf[0])
	{
	case USB_PID_SOF:
		vsfsdcd_fn = (buf[1] | (buf[2] << 8)) & 0x7FF;
		break;
	case USB_PID_SETUP:
		vsfsdcd_state = SDCD_IDLE;
		if (addr == vsfsdcd_addr)
		{
			curep = epno;
			vsfsdcd_state = SDCD_SETUP;
		}
		break;
	case USB_PID_OUT:
		vsfsdcd_state = SDCD_IDLE;
		if (addr == vsfsdcd_addr)
		{
			curep = epno;
			vsfsdcd_state = SDCD_OUT;
		}
		break;
	case USB_PID_IN:
		vsfsdcd_state = SDCD_IDLE;
		if (addr == vsfsdcd_addr)
		{
			curep = epno | 0x80;
			ep = vsfsdcd_find_ep(curep);
			if (ep != NULL)
			{
				if (ep->token == USB_PID_ACK)
				{
					vsfsdcd_send_data(ep);
					vsfsdcd_state = SDCD_IN;
				}
				else
					vsfsdcd_send_token(ep->token);
			}
		}
		break;
	case USB_PID_ACK:
		if (vsfsdcd_state == SDCD_IN)
		{
			vsfsdcd_state = SDCD_IDLE;
			ep = vsfsdcd_find_ep(curep);
			if (ep != NULL)
			{
				ep->token = USB_PID_NAK;
				ep->transfered = 1;
				vsfsdcd_evt |= SDCD_ON_IN;
			}
		}
		break;
	case USB_PID_NAK:
		break;
	case USB_PID_DATA0:
		toggle = 0;
		goto on_data;
	case USB_PID_DATA1:
		toggle = 1;
	on_data:
		if (vsfsdcd_state != SDCD_IDLE)
		{
			ep = vsfsdcd_find_ep(curep);
			if (ep != NULL)
			{
				// check toggle
				if (((vsfsdcd_state == SDCD_SETUP) && (toggle != 0)) ||
					((vsfsdcd_state != SDCD_SETUP) && (toggle != ep->toggle)))
					break;

				if (vsfsdcd_state == SDCD_SETUP)
					vsfsdcd_send_token(USB_PID_ACK);
				else
					vsfsdcd_send_token(ep->token);

				if ((vsfsdcd_state == SDCD_SETUP) || (ep->token == USB_PID_ACK))
				{
					ep->count = len - 4;
					memcpy(ep->buf, &buf[1], ep->count);
					ep->token = USB_PID_NAK;
					ep->toggle = !ep->toggle;

					switch (vsfsdcd_state)
					{
					case SDCD_SETUP:
						vsfsdcd_evt |= SDCD_ON_SETUP;
						// first 2 ep is EP0_OUT and EP0_IN
						vsfsdcd_ep[0].toggle = 1;
						vsfsdcd_ep[1].toggle = 1;
						break;
					case SDCD_OUT:
						ep->transfered = 1;
						vsfsdcd_evt |= SDCD_ON_OUT;
						break;
					}
					vsfsdcd_state = SDCD_IDLE;
				}
			}
			break;
		}
	}
}

static vsf_err_t vsfsdcd_init(int32_t int_priority)
{
	vsfsdcd_reset();
	vsfhal_usbdio_init(vsfsdcd_onrx);
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_fini(void)
{
	return vsfhal_usbdio_fini();
}

static vsf_err_t vsfsdcd_poll(void)
{
	if (vsfsdcd_evt)
	{
		if (vsfsdcd_evt & SDCD_ON_RST)
		{
			vsfsdcd_evt = 0;
			if (vsfsdcd_callback.on_reset != NULL)
				vsfsdcd_callback.on_reset(vsfsdcd_callback.param);
		}
		if (vsfsdcd_evt & SDCD_ON_SOF)
		{
			vsfsdcd_evt &= ~SDCD_ON_SOF;
			if (vsfsdcd_callback.on_sof != NULL)
				vsfsdcd_callback.on_sof(vsfsdcd_callback.param);
		}
		if (vsfsdcd_evt & SDCD_ON_SETUP)
		{
			vsfsdcd_evt &= ~SDCD_ON_SETUP;

			if (vsfsdcd_callback.on_setup != NULL)
				vsfsdcd_callback.on_setup(vsfsdcd_callback.param);
		}
		for (int i = 0; i < dimof(vsfsdcd_ep); i++)
		{
			if ((vsfsdcd_ep[i].addr != 0xFF) && vsfsdcd_ep[i].transfered)
			{
				vsfsdcd_ep[i].transfered = 0;
				if (vsfsdcd_ep[i].addr & 0x80)
				{
					if (vsfsdcd_callback.on_in != NULL)
						vsfsdcd_callback.on_in(vsfsdcd_callback.param, vsfsdcd_ep[i].addr & 0x7F);
				}
				else
				{
					if (vsfsdcd_callback.on_out != NULL)
						vsfsdcd_callback.on_out(vsfsdcd_callback.param, vsfsdcd_ep[i].addr & 0x7F);
				}
			}
		}
	}
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_dummy(void)
{
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_u8_dummy(uint8_t level)
{
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_set_address(uint8_t address)
{
	vsfsdcd_addr = address;
	return VSFERR_NONE;
}

static uint8_t vsfsdcd_get_address(void)
{
	return vsfsdcd_addr;
}

static uint32_t vsfsdcd_get_frame_number(void)
{
	return vsfsdcd_fn;
}

static uint16_t vsfsdcd_ep_get_OUT_count(uint8_t idx);
static vsf_err_t vsfsdcd_ep_read_OUT_buffer(uint8_t idx, uint8_t *buffer, uint16_t size);
static vsf_err_t vsfsdcd_get_setup(uint8_t *buffer)
{
	return vsfsdcd_ep_get_OUT_count(0x00) == 8 ?
		vsfsdcd_ep_read_OUT_buffer(0x00, buffer, 8) : VSFERR_FAIL;
}

static struct vsfsdcd_ep_t * vsfsdcd_get_ep(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_find_ep(idx);
	if (ep != NULL)
		return ep;

	for (int i = 0; i < dimof(vsfsdcd_ep); i++)
	{
		if (0xFF == vsfsdcd_ep[i].addr)
		{
			vsfsdcd_ep[i].addr = idx;
			return &vsfsdcd_ep[i];
		}
	}
	return NULL;
}

static vsf_err_t vsfsdcd_ep_set_type(uint8_t idx, enum vsfhal_usbd_eptype_t type)
{
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_ep_not_support(uint8_t idx)
{
	return VSFERR_NOT_SUPPORT;
}

static bool vsfsdcd_ep_false(uint8_t idx)
{
	return false;
}

static vsf_err_t vsfsdcd_ep_set_epsize(uint8_t idx, uint16_t epsize, uint8_t token)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	if (!ep || (epsize > 8))
		return VSFERR_FAIL;

	ep->token = token;
	ep->size = epsize;
	return VSFERR_NONE;
}

static uint16_t vsfsdcd_ep_get_epsize(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	return ep ? ep->size : 0;
}

static vsf_err_t vsfsdcd_ep_set_stall(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	if (!ep)
		return VSFERR_FAIL;

	ep->token = USB_PID_STALL;
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_ep_clear_stall(uint8_t idx, uint8_t token)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	if (!ep)
		return VSFERR_FAIL;

	ep->token = token;
	return VSFERR_NONE;
}

static bool vsfsdcd_ep_is_stall(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	return ep ? ep->token == USB_PID_STALL : false;
}

static vsf_err_t vsfsdcd_ep_reset_toggle(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	if (!ep)
		return VSFERR_FAIL;

	ep->toggle = 0;
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_ep_toggle_toggle(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	if (!ep)
		return VSFERR_FAIL;

	ep->toggle = !ep->toggle;
	return VSFERR_NONE;
}

// IN
static vsf_err_t vsfsdcd_ep_set_IN_epsize(uint8_t idx, uint16_t epsize)
{
	return vsfsdcd_ep_set_epsize(idx | 0x80, epsize, USB_PID_NAK);
}

static uint16_t vsfsdcd_ep_get_IN_epsize(uint8_t idx)
{
	return vsfsdcd_ep_get_epsize(idx | 0x80);
}

static vsf_err_t vsfsdcd_ep_set_IN_stall(uint8_t idx)
{
	return vsfsdcd_ep_set_stall(idx | 0x80);
}

static vsf_err_t vsfsdcd_ep_clear_IN_stall(uint8_t idx)
{
	return vsfsdcd_ep_clear_stall(idx | 0x80, USB_PID_NAK);
}

static bool vsfsdcd_ep_is_IN_stall(uint8_t idx)
{
	return vsfsdcd_ep_is_stall(idx | 0x80);
}

static vsf_err_t vsfsdcd_ep_reset_IN_toggle(uint8_t idx)
{
	return vsfsdcd_ep_reset_toggle(idx | 0x80);
}

static vsf_err_t vsfsdcd_ep_toggle_IN_toggle(uint8_t idx)
{
	return vsfsdcd_ep_toggle_toggle(idx | 0x80);
}

static vsf_err_t vsfsdcd_ep_set_IN_count(uint8_t idx, uint16_t size)
{
	uint16_t crc;
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx | 0x80);
	if (!ep || (size > ep->size))
		return VSFERR_FAIL;

	ep->buf[0] = 0x80;			// SYNC
	crc = usb_crc16(&ep->buf[2], size);
	crc = SYS_TO_LE_U16(crc);
	memcpy(&ep->buf[2 + size], (uint8_t *)&crc, 2);

	ep->count = size + 4;		// add SYNC, TOKEN and CRC16
	ep->token = USB_PID_ACK;
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_ep_write_IN_buffer(uint8_t idx, uint8_t *buffer, uint16_t size)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx | 0x80);
	if (!ep || (size > ep->size))
		return VSFERR_FAIL;

	memcpy(&ep->buf[2], buffer, size);
	return VSFERR_NONE;
}

// OUT
static vsf_err_t vsfsdcd_ep_set_OUT_epsize(uint8_t idx, uint16_t epsize)
{
	uint8_t token = idx ? USB_PID_NAK : USB_PID_ACK;
	return vsfsdcd_ep_set_epsize(idx, epsize, token);
}

static vsf_err_t vsfsdcd_ep_clear_OUT_stall(uint8_t idx)
{
	uint8_t token = idx ? USB_PID_NAK : USB_PID_ACK;
	return vsfsdcd_ep_clear_stall(idx, token);
}

static uint16_t vsfsdcd_ep_get_OUT_count(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	return ep ? ep->count : 0;
}

static vsf_err_t vsfsdcd_ep_read_OUT_buffer(uint8_t idx, uint8_t *buffer, uint16_t size)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	if (!ep || (size > ep->size))
		return VSFERR_FAIL;

	memcpy(buffer, ep->buf, size);
	ep->count = 0;
	return VSFERR_NONE;
}

static vsf_err_t vsfsdcd_ep_enable_OUT(uint8_t idx)
{
	struct vsfsdcd_ep_t *ep = vsfsdcd_get_ep(idx);
	if (!ep)
		return VSFERR_FAIL;

	ep->count = 0;
	ep->token = USB_PID_ACK;
	return VSFERR_NONE;
}

const struct vsfhal_usbd_t vsfsdcd_usbd =
{
	.init = vsfsdcd_init,
	.fini = vsfsdcd_fini,
	.poll = vsfsdcd_poll,
	.reset = vsfsdcd_dummy,
	.connect = vsfsdcd_dummy,
	.disconnect = vsfsdcd_dummy,
	.set_address = vsfsdcd_set_address,
	.get_address = vsfsdcd_get_address,
	.suspend = vsfsdcd_dummy,
	.resume = vsfsdcd_dummy,
	.lowpower = vsfsdcd_u8_dummy,
	.get_frame_number = vsfsdcd_get_frame_number,
	.get_setup = vsfsdcd_get_setup,
	.prepare_buffer = vsfsdcd_dummy,
	.ep.num_of_ep = &vsfsdcd_ep_num,
	.ep.reset = vsfsdcd_u8_dummy,
	.ep.set_type = vsfsdcd_ep_set_type,
	.ep.set_IN_dbuffer = vsfsdcd_ep_not_support,
	.ep.is_IN_dbuffer = vsfsdcd_ep_false,
	.ep.switch_IN_buffer = vsfsdcd_ep_not_support,
	.ep.set_IN_epsize = vsfsdcd_ep_set_IN_epsize,
	.ep.get_IN_epsize = vsfsdcd_ep_get_IN_epsize,
	.ep.set_IN_stall = vsfsdcd_ep_set_IN_stall,
	.ep.clear_IN_stall = vsfsdcd_ep_clear_IN_stall,
	.ep.is_IN_stall = vsfsdcd_ep_is_IN_stall,
	.ep.reset_IN_toggle = vsfsdcd_ep_reset_IN_toggle,
	.ep.toggle_IN_toggle = vsfsdcd_ep_toggle_IN_toggle,
	.ep.set_IN_count = vsfsdcd_ep_set_IN_count,
	.ep.write_IN_buffer = vsfsdcd_ep_write_IN_buffer,
	.ep.set_OUT_dbuffer = vsfsdcd_ep_not_support,
	.ep.is_OUT_dbuffer = vsfsdcd_ep_false,
	.ep.switch_OUT_buffer = vsfsdcd_ep_not_support,
	.ep.set_OUT_epsize = vsfsdcd_ep_set_OUT_epsize,
	.ep.get_OUT_epsize = vsfsdcd_ep_get_epsize,
	.ep.set_OUT_stall = vsfsdcd_ep_set_stall,
	.ep.clear_OUT_stall = vsfsdcd_ep_clear_OUT_stall,
	.ep.is_OUT_stall = vsfsdcd_ep_is_stall,
	.ep.reset_OUT_toggle = vsfsdcd_ep_reset_toggle,
	.ep.toggle_OUT_toggle = vsfsdcd_ep_toggle_toggle,
	.ep.get_OUT_count = vsfsdcd_ep_get_OUT_count,
	.ep.read_OUT_buffer = vsfsdcd_ep_read_OUT_buffer,
	.ep.enable_OUT = vsfsdcd_ep_enable_OUT,
	.callback = &vsfsdcd_callback,
};
