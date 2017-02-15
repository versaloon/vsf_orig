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

static struct vsfsm_state_t *
vsftimer_init_handler(struct vsfsm_t *sm, vsfsm_evt_t evt);

struct vsftimer_info_t
{
	struct vsfsm_t sm;
	struct vsfq_t timerlist;

	struct vsftimer_mem_op_t *mem_op;
} static vsftimer =
{
	.sm.init_state.evt_handler = vsftimer_init_handler,
};

// vsftimer_callback_int is called in interrupt,
// simply send event to vsftimer SM
void vsftimer_callback_int(void)
{
	vsfsm_post_evt_pending(&vsftimer.sm, VSFSM_EVT_TIMER);
}

vsf_err_t vsftimer_init(struct vsftimer_mem_op_t *mem_op)
{
	vsftimer.mem_op = mem_op;

	vsfq_init(&vsftimer.timerlist);
	return vsfsm_init(&vsftimer.sm);
}

static struct vsftimer_t *vsftimer_allocate(void)
{
	return vsftimer.mem_op->alloc();
}

void vsftimer_enqueue(struct vsftimer_t *timer)
{
	timer->node.addr = timer->interval + vsfhal_tickclk_get_count();
	vsftimer_dequeue(timer);
	vsfq_enqueue(&vsftimer.timerlist, &timer->node);
}

void vsftimer_dequeue(struct vsftimer_t *timer)
{
	vsfq_remove(&vsftimer.timerlist, &timer->node);
}

static struct vsfsm_state_t *
vsftimer_init_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	uint32_t cur_tick = vsfhal_tickclk_get_count();
	struct vsftimer_t *timer;

	switch (evt)
	{
	case VSFSM_EVT_TIMER:
		timer = (struct vsftimer_t *)vsftimer.timerlist.head;
		while (timer != NULL)
		{
			if (cur_tick >= timer->node.addr)
			{
				if (timer->trigger_cnt > 0)
				{
					timer->trigger_cnt--;
				}
				if (timer->trigger_cnt != 0)
				{
					vsftimer_enqueue(timer);
				}
				else
				{
					vsftimer_free(timer);
				}

				if (timer->evt != VSFSM_EVT_INVALID)
				{
					if (timer->sm != NULL)
					{
						vsfsm_post_evt(timer->sm, timer->evt);
					}
				}
				else
				{
					if (timer->cb != NULL)
					{
						timer->cb(timer->param);
					}
				}
				timer = (struct vsftimer_t *)vsftimer.timerlist.head;
			}
			else
			{
				break;
			}
		}
		break;
	}
	return NULL;
}

struct vsftimer_t *vsftimer_create_cb(uint32_t interval, int16_t trigger_cnt,
									void (*cb)(void *), void *param)
{
	struct vsftimer_t *timer = vsftimer_allocate();
	if (NULL == timer)
	{
		return NULL;
	}

	timer->evt = VSFSM_EVT_INVALID;
	timer->cb = cb;
	timer->param = param;
	timer->interval = interval;
	timer->trigger_cnt = trigger_cnt;
	vsftimer_enqueue(timer);
	return timer;
}

struct vsftimer_t *vsftimer_create(struct vsfsm_t *sm, uint32_t interval,
									int16_t trigger_cnt, vsfsm_evt_t evt)
{
	struct vsftimer_t *timer = vsftimer_allocate();
	if (NULL == timer)
	{
		return NULL;
	}

	timer->sm = sm;
	timer->evt = evt;
	timer->interval = interval;
	timer->trigger_cnt = trigger_cnt;
	vsftimer_enqueue(timer);
	return timer;
}

void vsftimer_free(struct vsftimer_t *timer)
{
	vsftimer_dequeue(timer);
	vsftimer.mem_op->free(timer);
}
