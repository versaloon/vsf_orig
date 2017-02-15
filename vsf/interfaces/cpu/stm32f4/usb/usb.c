#include "app_cfg.h"
#include "app_type.h"
#include "interfaces.h"

#include "stm32f4xx.h"

#if IFS_HCD_EN

#define STM32F4_USB_NUM			2
#define USB_FS					0
#define USB_HS					1

static vsf_err_t (*otg_irq[STM32F4_USB_NUM])(void*);
static void *otg_irq_param[STM32F4_USB_NUM];
ROOTFUNC void OTG_FS_IRQHandler(void)
{
	if(otg_irq[USB_FS] != NULL)
		otg_irq[USB_FS](otg_irq_param[USB_FS]);
}

ROOTFUNC void OTG_HS_IRQHandler(void)
{
	if(otg_irq[USB_HS] != NULL)
		otg_irq[USB_HS](otg_irq_param[USB_HS]);
}

vsf_err_t stm32f4_hcd_init(uint32_t index, vsf_err_t (*irq)(void *), void *param)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= STM32F4_USB_NUM)
		return VSFERR_NOT_SUPPORT;

	// enable 48M clock
	// see core.c
	
	otg_irq[usb_id] = irq;
	otg_irq_param[usb_id] = param;

	if (usb_id == USB_FS)
	{
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
		// PA10 id AF OD
		GPIOA->MODER &= ~(3 << (10 * 2));
		GPIOA->MODER |= 2 << (10 * 2);
		GPIOA->AFR[1] |= 10 << (10 * 4 - 32);
		// GPIOA->OSPEEDR |= 3 << (10 * 2);
		GPIOA->OTYPER |= 1 << 10;
		GPIOA->PUPDR &= ~(3 << (10 * 2));
		GPIOA->PUPDR |= 1 << (10 * 2);

		//  PA11
		GPIOA->MODER &= ~(3 << (11 * 2));
		GPIOA->MODER |= 2 << (11 * 2);
		GPIOA->AFR[1] |= 10 << (11 * 4 - 32);
		GPIOA->OSPEEDR |= 3 << (11 * 2);
		GPIOA->PUPDR &= ~(3 << (11 * 2));
		
		// PA12
		GPIOA->MODER &= ~(3 << (12 * 2));
		GPIOA->MODER |= 2 << (12 * 2);
		GPIOA->AFR[1] |= 10 << (12 * 4 - 32);
		GPIOA->OSPEEDR |= 3 << (12 * 2);
		GPIOA->PUPDR &= ~(3 << (12 * 2));		
		
		SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);
		NVIC_EnableIRQ(OTG_FS_IRQn);
	}
	else if (usb_id == USB_HS)
	{
		// TODO
	}

	return VSFERR_NONE;
}

vsf_err_t stm32f4_hcd_fini(uint32_t index)
{
	uint16_t usb_id = index >> 16;

	if (usb_id >= STM32F4_USB_NUM)
		return VSFERR_NOT_SUPPORT;

	otg_irq[usb_id] = NULL;
	otg_irq_param[usb_id] = NULL;
	
	if (usb_id == USB_FS)
	{
		CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);
		NVIC_DisableIRQ(OTG_FS_IRQn);
	}
	else if (usb_id == USB_HS)
	{
		// TODO
	}

	return VSFERR_NONE;
}


void* stm32f4_hcd_regbase(uint32_t index)
{
	switch (index >> 16)
	{
	case 0:
		return (void*)USB_OTG_FS;
	default:
		return NULL;
	}
}

#endif // IFS_HCD_EN

#if IFS_USB_DCD_EN

#endif // IFS_HCD_EN

