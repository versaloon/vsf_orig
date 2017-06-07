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

#ifndef __VSFGDB_H_INCLUDED__
#define __VSFGDB_H_INCLUDED__

struct vsfgdb_reg_t
{
	uint8_t idx;
	uint8_t size;
	uint8_t dirty;
	uint8_t reserved;
	uint64_t value;
};

struct vsfgdb_hwbp_t
{
	bool enabled;
	uint64_t addr;
	uint32_t size;
};

// software breadpoint is a list because can allocate more if needed
struct vsfgdb_swbp_t
{
	bool enabled;
	uint64_t addr;
	uint32_t size;
	uint64_t orig;
	struct vsfgdb_swbp_t *next;
};

struct vsfgdb_wp_t
{
	bool enabled;
	uint64_t addr;
	uint32_t size;
};

enum vsfgdb_wptype_t
{
	VSFGDB_WPTYPE_WRITE			= (int)'2',
	VSFGDB_WPTYPE_READ			= (int)'3',
	VSFGDB_WPTYPE_ACCESS		= (int)'4',
};

enum vsfgdb_bptype_t
{
	VSFGDB_BPTYPE_SOFT			= '0',
	VSFGDB_BPTYPE_HARD			= '1',
};

struct vsfgdb_target_op_t
{
	vsf_err_t (*init)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*fini)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);

	vsf_err_t (*readreg)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfgdb_reg_t *reg);
	vsf_err_t (*writereg)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfgdb_reg_t *reg);

	vsf_err_t (*readmem)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							uint64_t addr, uint32_t size, uint8_t *buf);
	vsf_err_t (*writemem)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							uint64_t addr, uint32_t size, uint8_t *buf);

	vsf_err_t (*breakpoint)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							uint64_t addr, uint32_t size,
							enum vsfgdb_bptype_t type, bool enable);
	vsf_err_t (*watchpoint)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							uint64_t addr, uint32_t size,
							enum vsfgdb_wptype_t type, bool enable);
	vsf_err_t (*step)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*cont)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
};

struct vsfgdb_target_t
{
	struct vsfgdb_target_op_t *op;
	uint8_t regnum;
	uint8_t swbpnum;
	uint8_t hwbpnum;
	uint8_t wpnum;

	// private
	struct vsfgdb_reg_t *regs;
	struct vsfgdb_swbp_t *swbps;
	struct vsfgdb_hwbp_t *hwbps;
	struct vsfgdb_watchpoint_t *wps;
};

struct vsfgdb_t
{
	struct vsfgdb_target_t target;
	uint16_t port;
	bool exit;
	struct vsf_transaction_buffer_t rsptbuf;

	// private
	struct vsfsm_pt_t pt;
	struct vsfsm_t sm;
	struct vsfsm_pt_t caller_pt;
	struct vsfsm_sem_t rspsem;
	struct vsfip_socket_t *so;
	struct vsfip_socket_t *session;
	struct vsfip_buffer_t *outbuf;

	// rsp
	struct vsfsm_pt_t packet_pt;
	bool idle;
	char dollar;
	char cmd;
	uint8_t checksum;
	uint8_t checksum_calc;
	uint8_t checksum_size;
	uint64_t addr;
	uint32_t len;
	union
	{
		enum vsfgdb_wptype_t wptype;
		enum vsfgdb_bptype_t bptype;
		char type;
	} ztype;
	uint8_t *ptr;
	char *replybuf;
	uint16_t replylen;

	vsf_err_t errcode;
};

vsf_err_t vsfgdb_start(struct vsfgdb_t *gdb);

#endif		// __VSFGDB_H_INCLUDED__
