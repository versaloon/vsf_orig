#include "app_cfg.h"
#include "app_type.h"
#include "interfaces.h"

#include "cmem7.h"

#if IFS_HCD_EN

#define CMEM7_USB_NUM			1

static vsf_err_t (*irq0)(void*);
static void *irq0_param;
static vsf_err_t (*irq1)(void*);
static void *irq1_param;

ROOTFUNC void OTG_HS_IRQHandler(void)
{
	if (irq0 != NULL)
		irq0(irq0_param);
}

ROOTFUNC void OTG_FS_IRQHandler(void)
{
	if(irq1 != NULL)
		irq1(irq1_param);
}

vsf_err_t cmem7_hcd_init(uint32_t index, vsf_err_t (*irq)(void *), void *param)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= CMEM7_USB_NUM)
		return VSFERR_NOT_SUPPORT;


	return VSFERR_NONE;
}

vsf_err_t cmem7_hcd_fini(uint32_t index)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= CMEM7_USB_NUM)
		return VSFERR_NOT_SUPPORT;


	return VSFERR_NONE;
}


void* cmem7_hcd_regbase(uint32_t index)
{
	switch (index >> 16)
	{
	default:
		return NULL;
	}
}

#endif // IFS_HCD_EN


#if IFS_USB_DCD_EN

#endif // IFS_HCD_EN

