#include "vsf.h"

#define APPCFG_VSFTIMER_NUM				16
#define APPCFG_VSFSM_PENDSVQ_LEN		16
#define APPCFG_VSFSM_MAINQ_LEN			16

static struct vsfsm_state_t* app_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt);
static void app_pendsv_activate(struct vsfsm_evtq_t *q);

struct vsfapp_t
{
	struct vsfdwcotg_hcd_param_t hcd_param;
	struct vsfusbh_t usbh;

	struct vsfsm_t sm;

	struct vsfsm_evtq_t pendsvq;
	struct vsfsm_evtq_t mainq;

	// private
	// buffer mamager
	VSFPOOL_DEFINE(vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);

	struct vsfsm_evtq_element_t pendsvq_ele[APPCFG_VSFSM_PENDSVQ_LEN];
	struct vsfsm_evtq_element_t mainq_ele[APPCFG_VSFSM_MAINQ_LEN];
	uint8_t bufmgr_buffer[12 * 1024];
} static app =
{
	.hcd_param.index						= 1,
	.hcd_param.speed						= USB_SPEED_HIGH,
	.hcd_param.dma_en						= 1,
	.hcd_param.ulpi_en						= 1,
	.hcd_param.vbus_en						= 1,
	.hcd_param.hc_amount					= 16,
	.hcd_param.fifo_size						= 0x1000,
	.hcd_param.periodic_out_packet_size_max		= 512,
	.hcd_param.non_periodic_out_packet_size_max = 512,
	.hcd_param.in_packet_size_max				= 1024,
	
	.usbh.hcd								= &vsfdwcotgh_drv,
	.usbh.hcd_param							= &app.hcd_param,

	//.shell.stream_tx						= (struct vsf_stream_t *)&app.usbd.cdc.stream_tx,
	//.shell.stream_rx						= (struct vsf_stream_t *)&app.usbd.cdc.stream_rx,

	.sm.init_state.evt_handler				= app_evt_handler,

	.pendsvq.size							= dimof(app.pendsvq_ele),
	.pendsvq.queue							= app.pendsvq_ele,
	.pendsvq.activate						= app_pendsv_activate,
	.mainq.size								= dimof(app.mainq_ele),
	.mainq.queue							= app.mainq_ele,
	.mainq.activate							= NULL,
};

// tickclk interrupt, simply call vsftimer_callback_int
static void app_tickclk_callback_int(void *param)
{
	vsftimer_callback_int();
}

static void app_pendsv_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_core_pendsv_trigger();
}

// vsftimer buffer mamager
static struct vsftimer_t* vsftimer_memop_alloc(void)
{
	return VSFPOOL_ALLOC(&app.vsftimer_pool, struct vsftimer_t);
}

static void vsftimer_memop_free(struct vsftimer_t *timer)
{
	VSFPOOL_FREE(&app.vsftimer_pool, timer);
}

struct vsftimer_mem_op_t vsftimer_memop =
{
	.alloc	= vsftimer_memop_alloc,
	.free	= vsftimer_memop_free,
};

void cdc_stream_debug_init(struct vsf_stream_t *stream)
{
	stream->callback_tx.param = stream;
	stream->callback_tx.on_connect = (void (*)(void *))debug_init;
	stream_connect_tx(stream);
}

static struct vsfsm_state_t *
app_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	switch (evt)
	{
	case VSFSM_EVT_INIT:
		vsfhal_core_init(NULL);
		vsfhal_tickclk_init();
		vsfhal_tickclk_start();

		VSFPOOL_INIT(&app.vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
		vsftimer_init(&vsftimer_memop);
		vsfhal_tickclk_config_cb(app_tickclk_callback_int, NULL);

		vsf_bufmgr_init(app.bufmgr_buffer, sizeof(app.bufmgr_buffer));

		vsfusbh_init(&app.usbh);
		vsfusbh_register_driver(&app.usbh, &vsfusbh_hub_drv);
		vsfusbh_register_driver(&app.usbh, &vsfusbh_hid_drv);
		break;
	}
	return NULL;
}

static void app_on_pendsv(void *param)
{
	struct vsfsm_evtq_t *evtq_cur = param, *evtq_old = vsfsm_evtq_set(evtq_cur);

	while (vsfsm_get_event_pending())
	{
		vsfsm_poll();
	}
	vsfsm_evtq_set(evtq_old);
}

int main(void)
{
	vsf_enter_critical();
	vsfsm_evtq_init(&app.pendsvq);
	vsfsm_evtq_init(&app.mainq);

	vsfsm_evtq_set(&app.pendsvq);
	vsfsm_init(&app.sm);

	vsfhal_core_pendsv_config(app_on_pendsv, &app.pendsvq);
	vsf_leave_critical();

	vsfsm_evtq_set(&app.mainq);
	while (1)
	{
		// no thread runs in mainq, just sleep in main loop
		vsfhal_core_sleep(SLEEP_WFI);
	}
}

