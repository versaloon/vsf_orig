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
#include "vsfgdb.h"

#define VSFGDB_EVT_RSP_PACKET				VSFSM_EVT_USER

static uint8_t chartou8(char c)
{
	if ((c >= 'a') && (c <= 'f'))
	{
		return c - 'a' + 10;
	}
	else if ((c >= 'A') && (c <= 'F'))
	{
		return c - 'A' + 10;
	}
	else if ((c >= '0') && (c <= '9'))
	{
		return c - '0';
	}
	return 0;
}

static uint8_t hextou8(char *hex)
{
	return (chartou8(hex[0]) << 4) + chartou8(hex[1]);
}

static uint16_t hextou16(char *hex)
{
	return ((uint16_t)hextou8(&hex[0]) << 8) + hextou8(&hex[2]);
}

static uint32_t hextou32(char *hex)
{
	return ((uint32_t)hextou16(&hex[0]) << 16) + hextou16(&hex[4]);
}

static uint64_t hextou64(char *hex)
{
	return ((uint64_t)hextou32(&hex[0]) << 32) + hextou32(&hex[8]);
}

static uint64_t hextovalue(char *hex, uint8_t bytelen, char **next)
{
	uint64_t value = 0;
	int i;

	if (!bytelen) bytelen = 8;
	for (i = 0; i < 2 * bytelen; i++)
	{
		if ((*hex >= 'a') && (*hex <= 'f'))
		{
			value = (value << 4) + *hex - 'a' + 10;
		}
		else if ((*hex >= 'A') && (*hex <= 'F'))
		{
			value = (value << 4) + *hex - 'A' + 10;
		}
		else if ((*hex >= '0') && (*hex <= '9'))
		{
			value = (value << 4) + *hex - '0';
		}
		else
		{
			break;
		}
		hex++;
	}
	if (next != NULL)
	{
		*next = hex;
	}
	return value;
}

const char hextbl[] = "0123456789ABCDEF";
static void u8tohex(uint8_t u8, char *hex)
{
	hex[0] = hextbl[(u8 >> 4) & 0x0F];
	hex[1] = hextbl[(u8 >> 0) & 0x0F];
	hex[2] = 0;
}

static void u16tohex(uint16_t u16, char *hex)
{
	u8tohex(u16 >> 8, hex + 0);
	u8tohex(u16 >> 0, hex + 2);
}

static void u32tohex(uint32_t u32, char *hex)
{
	u16tohex(u32 >> 16, hex + 0);
	u16tohex(u32 >> 0 , hex + 4);
}

static void u64tohex(uint64_t u64, char *hex)
{
	u32tohex(u64 >> 32, hex + 0);
	u32tohex(u64 >> 0 , hex + 8);
}

static void valuetohex(uint64_t value, uint8_t bytelen, char *hex)
{
	switch (bytelen)
	{
	case 1: u8tohex(value, hex); break;
	case 2: u16tohex(value, hex); break;
	case 4: u32tohex(value, hex); break;
	case 8: u64tohex(value, hex); break;
	}
}

static void vsfgdb_on_session_input(void *param, struct vsfip_buffer_t *buf)
{
	struct vsfgdb_t *gdb = (struct vsfgdb_t *)param;
	char *ptr = (char *)buf->app.buffer;
	uint32_t size = buf->app.size;

	if (!gdb->idle)
	{
		goto end;
	}

	while (size && ('\0' == gdb->dollar))
	{
		if ('$' == *ptr++)
		{
			gdb->dollar = '$';
			gdb->checksum = 0;
			gdb->checksum_calc = 0;
			gdb->checksum_size = 0;
			gdb->rsptbuf.position = 0;
		}
		size--;
	}

	while (size && ('#' != *ptr))
	{
		gdb->checksum_calc += *ptr;
		gdb->rsptbuf.buffer.buffer[gdb->rsptbuf.position] = *ptr++;
		size--;
	}
	while (size && (gdb->checksum_size < 2))
	{
		gdb->checksum_size++;
		gdb->checksum += chartou8(*ptr++) << ((2 - gdb->checksum_size) << 2);
		if (2 == gdb->checksum_size)
		{
			gdb->idle = !(gdb->checksum == gdb->checksum_calc);
			vsfsm_sem_post(&gdb->rspsem);
			break;
		}
	}
end:
	vsfip_buffer_release(buf);
}

static vsf_err_t vsfgdb_get_addrlen(char *str, uint64_t *addr, uint32_t *len,
										char **next)
{
	*addr = hextovalue(str, 0, &str);
	if (*str++ != ',')
	{
		return VSFERR_FAIL;
	}
	*len = (uint32_t)hextovalue(str, 4, next);
	return VSFERR_NONE;
}

static void vsfgdb_add_reply(struct vsfgdb_t *gdb, char *str)
{
	while (*str != '\0')
	{
		gdb->replybuf[gdb->replylen++] = *str++;
	}
}

static vsf_err_t vsfgdb_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfgdb_t *gdb = (struct vsfgdb_t *)pt->user_data;
	char *parambuf = (char *)gdb->rsptbuf.buffer.buffer + 1;
	vsf_err_t err;
	uint32_t i;

	vsfsm_pt_begin(pt);

	// prepare socket
	gdb->so = vsfip_socket(AF_INET, IPPROTO_TCP);
	if (NULL == gdb->so)
	{
		gdb->errcode = VSFERR_NOT_ENOUGH_RESOURCES;
		goto fail_socket;
	}
	if ((vsfip_bind(gdb->so, gdb->port) != 0) ||
		(vsfip_listen(gdb->so, 1) != 0))
	{
		gdb->errcode = VSFERR_FAIL;
		goto fail_socket_connect;
	}

	gdb->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_tcp_accept(&gdb->caller_pt, evt, gdb->so, &gdb->session);
	if (err > 0) return err; else if (err < 0)
	{
		gdb->errcode = VSFERR_FAIL;
		goto fail_socket_connect;
	}
	vsfip_socket_cb(gdb->session, gdb, vsfgdb_on_session_input, NULL);

	// target init, this will also read target information
	gdb->caller_pt.user_data = &gdb->target;
	gdb->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = gdb->target.op->init(&gdb->caller_pt, evt);
	if (err > 0) return err; else if (err < 0)
	{
		gdb->errcode = VSFERR_FAIL;
		goto fail_target_init;
	}

	// handle RSP
	while (!gdb->exit)
	{
		// wait for rsp packet
		if (vsfsm_sem_pend(&gdb->rspsem, pt->sm))
		{
			vsfsm_pt_wfe(pt, VSFGDB_EVT_RSP_PACKET);
		}

		gdb->outbuf = VSFIP_TCPBUF_GET(VSFIP_CFG_TCP_MSS);
		if (NULL == gdb->outbuf)
		{
			gdb->errcode = VSFERR_NOT_ENOUGH_RESOURCES;
			goto fail_connected;
		}
		gdb->replybuf = (char*)&gdb->outbuf->app.buffer[gdb->outbuf->app.size];
		gdb->replylen = 0;
		vsfgdb_add_reply(gdb, gdb->idle ? "-" : "+");
		if (gdb->idle)
		{
			goto reply;
		}
		vsfgdb_add_reply(gdb, "$");

		// rsp parser
		gdb->cmd = (char)*gdb->rsptbuf.buffer.buffer;
		if ('?' == gdb->cmd)
		{
		}
		else if ('D' == gdb->cmd)
		{
			// Detach
			gdb->exit = true;
			vsfgdb_add_reply(gdb, "OK");
		}
		else if ('g' == gdb->cmd)
		{
			struct vsfgdb_reg_t *reg = gdb->target.regs;
			char buf[17];

			for (i = 0; i < gdb->target.regnum; i++)
			{
				valuetohex(reg[i].value, reg[i].size, buf);
				vsfgdb_add_reply(gdb, buf);
			}
		}
		else if ('G' == gdb->cmd)
		{
			struct vsfgdb_reg_t *reg = gdb->target.regs;

			for (i = 0; i < gdb->target.regnum; i++)
			{
				reg[i].value = hextovalue(parambuf, reg[i].size, &parambuf);
				reg[i].dirty = true;
			}
			vsfgdb_add_reply(gdb, "OK");
		}
		else if ('p' == gdb->cmd)
		{
			uint32_t idx = (uint32_t)hextovalue(parambuf, 4, NULL);
			if (idx < gdb->target.regnum)
			{
				struct vsfgdb_reg_t *reg = &gdb->target.regs[idx];
				char buf[17];
				valuetohex(reg->value, reg->size, buf);
				vsfgdb_add_reply(gdb, buf);
			}
			else
			{
				vsfgdb_add_reply(gdb, "E01");
			}
		}
		else if ('P' == gdb->cmd)
		{
			uint32_t idx = hextovalue(parambuf, 4, &parambuf);
			uint64_t value;
			if (*parambuf++ != '=')
			{
				vsfgdb_add_reply(gdb, "E01");
				goto reply;
			}
			value = hextovalue(parambuf, 0, NULL);

			if (idx < gdb->target.regnum)
			{
				gdb->target.regs[idx].dirty = true;
				gdb->target.regs[idx].value = value;
			}
			else
			{
				vsfgdb_add_reply(gdb, "E01");
			}
		}
		else if ('m' == gdb->cmd)
		{
			if (vsfgdb_get_addrlen(parambuf, &gdb->addr, &gdb->len, &parambuf))
			{
				vsfgdb_add_reply(gdb, "E01");
				goto reply;
			}
			gdb->ptr = (uint8_t *)gdb->rsptbuf.buffer.buffer;

			gdb->caller_pt.user_data = &gdb->target;
			gdb->caller_pt.state = 0;
			vsfsm_pt_entry(pt);
			err = gdb->target.op->readmem(&gdb->caller_pt, evt, gdb->addr,
											gdb->len, gdb->ptr);
			if (err > 0) return err; else if (err < 0)
			{
				gdb->exit = true;
				vsfgdb_add_reply(gdb, "E01");
				goto reply;
			}

			{
				char buf[3];
				for (i = 0; i < gdb->len; i++)
				{
					u8tohex(*gdb->ptr++, buf);
					vsfgdb_add_reply(gdb, buf);
				}
			}
		}
		else if ('M' == gdb->cmd)
		{
			if (vsfgdb_get_addrlen(parambuf, &gdb->addr, &gdb->len, &parambuf) ||
				(*parambuf++ != ':'))
			{
				vsfgdb_add_reply(gdb, "E01");
				goto reply;
			}
			gdb->ptr = (uint8_t *)gdb->rsptbuf.buffer.buffer;
			for (i = 0; i < gdb->len; i++)
			{
				gdb->ptr[i] = hextou8(parambuf + i * 2);
			}

			gdb->caller_pt.user_data = &gdb->target;
			gdb->caller_pt.state = 0;
			vsfsm_pt_entry(pt);
			err = gdb->target.op->writemem(&gdb->caller_pt, evt, gdb->addr,
											gdb->len, gdb->ptr);
			if (err > 0) return err; else if (err < 0)
			{
				gdb->exit = true;
				vsfgdb_add_reply(gdb, "E01");
				goto reply;
			}
			vsfgdb_add_reply(gdb, "OK");
		}
		else if ('z' == gdb->cmd)
		{
			gdb->ztype.type = (enum vsfgdb_wptype_t)*parambuf++;
			if ((*parambuf++ != ',') ||
				vsfgdb_get_addrlen(parambuf, &gdb->addr, &gdb->len, NULL))
			{
				vsfgdb_add_reply(gdb, "E01");
				goto reply;
			}

			if (gdb->ztype.bptype <= VSFGDB_BPTYPE_HARD)
			{
				// '0' for software bp, '1' for hardware breakpoing
				gdb->caller_pt.user_data = &gdb->target;
				gdb->caller_pt.state = 0;
				vsfsm_pt_entry(pt);
				err = gdb->target.op->breakpoint(&gdb->caller_pt, evt,
								gdb->addr, gdb->len, gdb->ztype.bptype, false);
				if (err > 0) return err; else if (err < 0)
				{
					gdb->exit = true;
					vsfgdb_add_reply(gdb, "E01");
					goto reply;
				}
				vsfgdb_add_reply(gdb, "OK");
			}
			else if (gdb->ztype.wptype <= VSFGDB_WPTYPE_ACCESS)
			{
				// '2' for write wp, '3' for read wp, '4' for access wp
				gdb->caller_pt.user_data = &gdb->target;
				gdb->caller_pt.state = 0;
				vsfsm_pt_entry(pt);
				err = gdb->target.op->watchpoint(&gdb->caller_pt, evt,
								gdb->addr, gdb->len, gdb->ztype.wptype, false);
				if (err > 0) return err; else if (err < 0)
				{
					gdb->exit = true;
					vsfgdb_add_reply(gdb, "E01");
					goto reply;
				}
				vsfgdb_add_reply(gdb, "OK");
			}
		}
		else if ('Z' == gdb->cmd)
		{
			gdb->ztype.type = (enum vsfgdb_wptype_t)*parambuf++;
			if ((*parambuf++ != ',') ||
				vsfgdb_get_addrlen(parambuf, &gdb->addr, &gdb->len, NULL))
			{
				vsfgdb_add_reply(gdb, "E01");
				goto reply;
			}

			if (gdb->ztype.bptype <= VSFGDB_BPTYPE_HARD)
			{
				// '0' for software bp, '1' for hardware breakpoing
				gdb->caller_pt.user_data = &gdb->target;
				gdb->caller_pt.state = 0;
				vsfsm_pt_entry(pt);
				err = gdb->target.op->breakpoint(&gdb->caller_pt, evt,
								gdb->addr, gdb->len, gdb->ztype.bptype, true);
				if (err > 0) return err; else if (err < 0)
				{
					gdb->exit = true;
					vsfgdb_add_reply(gdb, "E01");
					goto reply;
				}
				vsfgdb_add_reply(gdb, "OK");
			}
			else if (gdb->ztype.wptype <= VSFGDB_WPTYPE_ACCESS)
			{
				// '2' for write wp, '3' for read wp, '4' for access wp
				gdb->caller_pt.user_data = &gdb->target;
				gdb->caller_pt.state = 0;
				vsfsm_pt_entry(pt);
				err = gdb->target.op->watchpoint(&gdb->caller_pt, evt,
								gdb->addr, gdb->len, gdb->ztype.wptype, true);
				if (err > 0) return err; else if (err < 0)
				{
					gdb->exit = true;
					vsfgdb_add_reply(gdb, "E01");
					goto reply;
				}
				vsfgdb_add_reply(gdb, "OK");
			}
		}
		else if (('c' == gdb->cmd) || ('C' == gdb->cmd))
		{
		}
		else if (('s' == gdb->cmd) || ('S' == gdb->cmd))
		{
		}
		else if ('k' == gdb->cmd)
		{
			gdb->exit = true;
			break;
		}
		else if ('v' == gdb->cmd)
		{
			
		}

	reply:
		gdb->dollar = gdb->cmd = '\0';
		gdb->idle = true;
		// reply
		{
			struct vsfip_socket_t *so = gdb->session;
			char buf[4];

			gdb->checksum_calc = 0;
			for (i = 1; i < gdb->outbuf->app.size; i++)
			{
				gdb->checksum_calc += gdb->outbuf->app.buffer[i];
			}
			buf[0] = '#';
			u8tohex(gdb->checksum_calc, &buf[1]);
			vsfgdb_add_reply(gdb, buf);
			gdb->outbuf->app.size = gdb->replylen;
			vsfip_tcp_async_send(so, &so->remote_sockaddr, gdb->outbuf);
		}
	}

fail_connected:
	gdb->caller_pt.user_data = &gdb->target;
	gdb->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = gdb->target.op->fini(&gdb->caller_pt, evt);
	if (err > 0) return err;
fail_target_init:
	gdb->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_tcp_close(&gdb->caller_pt, evt, gdb->session);
	if (err > 0) return err;

	gdb->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_tcp_close(&gdb->caller_pt, evt, gdb->so);
	if (err > 0) return err;
fail_socket_connect:
	vsfip_close(gdb->so);
	gdb->so = NULL;
fail_socket:
	// start again
	vsfgdb_start(gdb);

	vsfsm_pt_end(pt);
	return gdb->errcode;
}

vsf_err_t vsfgdb_start(struct vsfgdb_t *gdb)
{
	gdb->idle = true;
	gdb->dollar = gdb->cmd = '\0';
	gdb->exit = false;
	gdb->errcode = VSFERR_NONE;
	gdb->target.regs = (struct vsfgdb_reg_t *)\
		vsf_bufmgr_malloc(gdb->target.regnum * sizeof(struct vsfgdb_reg_t));

	vsfsm_sem_init(&gdb->rspsem, 0, VSFGDB_EVT_RSP_PACKET);
	gdb->pt.thread = vsfgdb_thread;
	gdb->pt.user_data = gdb;
	return vsfsm_pt_init(&gdb->sm, &gdb->pt);
}
