#include "vsf.h"
#include "app_hw_cfg.h"

const struct app_hwcfg_t app_hwcfg =
{
	.board =				APP_BOARD_NAME,

	.key.port =				KEY_PORT,
	.key.pin =				KEY_PIN,

	.usbd.pullup.port =		USB_PULLUP_PORT,
	.usbd.pullup.pin =		USB_PULLUP_PIN,

#ifdef VSFCFG_FUNC_BCMWIFI
	.bcmwifi.type =			BCMWIFI_PORT_TYPE,
	.bcmwifi.index =		BCMWIFI_PORT,
	.bcmwifi.freq_khz =		BCMWIFI_FREQ,
	.bcmwifi.rst.port =		BCMWIFI_RST_PORT,
	.bcmwifi.rst.pin =		BCMWIFI_RST_PIN,
	.bcmwifi.wakeup.port =	BCMWIFI_WAKEUP_PORT,
	.bcmwifi.wakeup.pin =	BCMWIFI_WAKEUP_PIN,
	.bcmwifi.mode.port =	BCMWIFI_MODE_PORT,
	.bcmwifi.mode.pin =		BCMWIFI_MODE_PIN,
	.bcmwifi.spi.cs.port =	BCMWIFI_SPI_CS_PORT,
	.bcmwifi.spi.cs.pin =	BCMWIFI_SPI_CS_PIN,
	.bcmwifi.spi.eint.port =BCMWIFI_EINT_PORT,
	.bcmwifi.spi.eint.pin =	BCMWIFI_EINT_PIN,
	.bcmwifi.spi.eint_idx =	BCMWIFI_EINT,
	.bcmwifi.pwrctrl.port =	BCMWIFI_PWRCTRL_PORT,
	.bcmwifi.pwrctrl.pin =	BCMWIFI_PWRCTRL_PIN,
#endif
};

struct vsfapp_t
{
	uint8_t bufmgr_buffer[APPCFG_BUFMGR_SIZE];
} static app;

void main(void)
{
	struct vsf_module_t *module;
	struct vsf_module_info_t *minfo =
						(struct vsf_module_info_t *)APPCFG_MODULES_ADDR;
	int32_t pagesize;

	vsf_enter_critical();
	vsfhal_core_init(NULL);
	vsfhal_flash_capacity(0, (uint32_t*)&pagesize, NULL);
	pagesize = msb(pagesize);
	vsf_bufmgr_init(app.bufmgr_buffer, sizeof(app.bufmgr_buffer));

	// register modules
	while (minfo->entry != 0xFFFFFFFF)
	{
		module = vsf_bufmgr_malloc(sizeof(struct vsf_module_t));
		if (NULL == module)
		{
			break;
		}

		module->flash = minfo;
		minfo = (struct vsf_module_info_t *)((uint8_t *)minfo +\
			((minfo->size + (1 << pagesize) - 1) & ~((1 << pagesize) - 1)));
		vsf_module_register(module);
	}

	vsf_module_load("vsf.os", true);
	// vsfos module SHALL never return;
}
