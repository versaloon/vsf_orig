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
// IMPORTANT: DONOT CHANGE ANYTHING IN THIS FILE
#include "vsf.h"
#include "usrapp.h"

#if defined(APPCFG_SRT_QUEUE_LEN) && (APPCFG_SRT_QUEUE_LEN > 0)
#define APPCFG_PENDSVQ_EN
#endif
#if defined(APPCFG_NRT_QUEUE_LEN) && (APPCFG_NRT_QUEUE_LEN > 0)
#define APPCFG_MAINQ_EN
#endif
#if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
#define APPCFG_VSFTIMER_EN
#endif

#ifdef APPCFG_PENDSVQ_EN
static void app_pendsv_activate(struct vsfsm_evtq_t *q);
#endif
struct vsfapp_t
{
	struct usrapp_t *usrapp;

#ifdef APPCFG_VSFTIMER_EN
	VSFPOOL_DEFINE(vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
#endif

#if VSFSM_CFG_PREMPT_EN
#ifdef APPCFG_PENDSVQ_EN
	struct vsfsm_evtq_t pendsvq;
	struct vsfsm_evtq_element_t pendsvq_ele[APPCFG_SRT_QUEUE_LEN];
#endif

#ifdef APPCFG_MAINQ_EN
	struct vsfsm_evtq_t mainq;
	struct vsfsm_evtq_element_t mainq_ele[APPCFG_NRT_QUEUE_LEN];
#endif
#endif

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	uint8_t bufmgr_buffer[APPCFG_BUFMGR_SIZE];
#endif
} static app =
{
	.usrapp = &usrapp,
#if VSFSM_CFG_PREMPT_EN
#ifdef APPCFG_PENDSVQ_EN
	.pendsvq.size = dimof(app.pendsvq_ele),
	.pendsvq.queue = app.pendsvq_ele,
	.pendsvq.activate = app_pendsv_activate,
#endif
#ifdef APPCFG_MAINQ_EN
	.mainq.size = dimof(app.mainq_ele),
	.mainq.queue = app.mainq_ele,
	.mainq.activate = NULL,
#endif
#endif
};

#ifdef APPCFG_VSFTIMER_EN
static struct vsftimer_t* vsftimer_memop_alloc(void)
{
	return VSFPOOL_ALLOC(&app.vsftimer_pool, struct vsftimer_t);
}

static void vsftimer_memop_free(struct vsftimer_t *timer)
{
	VSFPOOL_FREE(&app.vsftimer_pool, timer);
}

const struct vsftimer_mem_op_t vsftimer_memop =
{
	.alloc			= vsftimer_memop_alloc,
	.free			= vsftimer_memop_free,
};

// tickclk interrupt, simply call vsftimer_callback_int
static void app_tickclk_callback_int(void *param)
{
#ifdef VSFCFG_FUNC_SHCI
	vsfshci_callback_int();
#endif
	vsftimer_callback_int();
}
#endif

static void vsfapp_init(struct vsfapp_t *app)
{
#if VSFSM_CFG_PREMPT_EN
#if defined(APPCFG_PENDSVQ_EN)
	vsfsm_evtq_set(&app->pendsvq);
#elif defined(APPCFG_MAINQ_EN)
	vsfsm_evtq_set(&app->mainq);
#else
	vsfsm_evtq_set(NULL);
#endif
#endif

	vsfhal_core_init(NULL);
	vsfhal_tickclk_init();
	vsfhal_tickclk_start();

#ifdef APPCFG_VSFTIMER_EN
	VSFPOOL_INIT(&app->vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
	vsftimer_init((struct vsftimer_mem_op_t *)&vsftimer_memop);
	vsfhal_tickclk_config_cb(app_tickclk_callback_int, NULL);
#endif

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	vsf_bufmgr_init(app->bufmgr_buffer, sizeof(app->bufmgr_buffer));
#endif

#ifdef APPCFG_SRT_QUEUE_LEN
	usrapp_srt_init(app->usrapp);
#endif

#ifdef APPCFG_NRT_QUEUE_LEN
#if VSFSM_CFG_PREMPT_EN
#ifdef APPCFG_MAINQ_EN
	vsfsm_evtq_set(&app->mainq);
#else
	vsfsm_evtq_set(NULL);
#endif
#endif
	usrapp_nrt_init(app->usrapp);
#endif
}

#ifdef APPCFG_PENDSVQ_EN
static void app_on_pendsv(void *param)
{
	struct vsfsm_evtq_t *evtq_cur = param, *evtq_old = vsfsm_evtq_set(evtq_cur);

	while (vsfsm_get_event_pending())
		vsfsm_poll();
	vsfsm_evtq_set(evtq_old);
}

static void app_pendsv_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_core_pendsv_trigger();
}
#endif

int main(void)
{
	vsf_enter_critical();

#ifdef APPCFG_MAINQ_EN
	vsfsm_evtq_init(&app.mainq);
#endif
#ifdef APPCFG_PENDSVQ_EN
	vsfsm_evtq_init(&app.pendsvq);
	vsfhal_core_pendsv_config(app_on_pendsv, &app.pendsvq);
#endif

	vsfapp_init(&app);
	vsf_leave_critical();

	while (1)
	{
#if defined(APPCFG_USR_POLL)
		usrapp_poll(app.usrapp);
#elif defined(APPCFG_MAINQ_EN)
		vsfsm_poll();
		vsf_enter_critical();
		if (!vsfsm_get_event_pending())
			vsfhal_core_sleep(SLEEP_WFI);	// will enable interrupt
		else
			vsf_leave_critical();
#else
		vsfhal_core_sleep(SLEEP_WFI);
#endif
	}
}
