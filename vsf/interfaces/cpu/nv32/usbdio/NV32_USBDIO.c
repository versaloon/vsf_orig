/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       BDM.c                                                     *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    BDM interface implementation file                         *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2011-05-09:     created(by SimonQian)                             *
 **************************************************************************/

#include "app_cfg.h"
#include "app_type.h"
#include "interfaces.h"
#include "compiler.h"

#if IFS_USBDIO_EN
// some check of the configurations
#if ((IFS_USBDIO_PORT == 0) &&\
		((IFS_USBDIO_DP != 12) && (IFS_USBDIO_DP != 13) && (IFS_USBDIO_DP != 24) && (IFS_USBDIO_DP != 25) ||\
		 (IFS_USBDIO_DM != 12) && (IFS_USBDIO_DM != 13) && (IFS_USBDIO_DM != 24) && (IFS_USBDIO_DM != 25))) ||\
	((IFS_USBDIO_PORT == 1) &&\
		((IFS_USBDIO_DP != 0) && (IFS_USBDIO_DP != 1) && (IFS_USBDIO_DP != 24) && (IFS_USBDIO_DP != 25) ||\
		 (IFS_USBDIO_DM != 0) && (IFS_USBDIO_DM != 1) && (IFS_USBDIO_DM != 24) && (IFS_USBDIO_DM != 25)))
#	error "DP and DM MUST be High Drive PINs"
#endif
#if (IFS_USBDIO_DM != IFS_USBDIO_DP + 1) && (IFS_USBDIO_DM != IFS_USBDIO_DP - 1)
#	error "DP and DM MUST be consecutive PINs
#endif

// NOTE PB12 is used for debug if enabled
#define IFS_USBDIO_DBG_PORT			0
#define IFS_USBDIO_DBG_PIN			0

//#define IFS_USBDIO_RX_LOOP
#define IFS_USBDIO_TX_LOOP

// IFS_USBDIO_PORT
// IFS_USBDIO_DP, IFS_USBDIO_DM
static void (*nv32_usbdio_onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len);

static void usbdio_set_mode(uint8_t in0_out1)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + (IFS_USBDIO_PORT << 6));
	if (in0_out1)
	{
		// disable EXTI before set to output
		IRQ->SC = 0;

		gpio->PIDR |= (1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM);
		gpio->PDDR |= (1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM);
	}
	else
	{
		gpio->PIDR &= ~((1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM));
		gpio->PDDR &= ~((1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM));

		// enable EXTI after set to input
		IRQ->SC = IRQ_SC_IRQPE_MASK | IRQ_SC_IRQIE_MASK;
	}
}

vsf_err_t nv32_usbdio_init(void (*onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len))
{
	nv32_usbdio_onrx = onrx;
	vsfhal_gpio_init(IFS_USBDIO_PORT);
	vsfhal_gpio_set(IFS_USBDIO_PORT, 1 << IFS_USBDIO_DP);
	// enable High Drive
#if IFS_USBDIO_PORT == 0
#	if (IFS_USBDIO_DM == 12) || (IFS_USBDIO_DP == 12)
	PORT->HDRVE |= 0x03;
#	elif (IFS_USBDIO_DM == 24) || (IFS_USBDIO_DP == 24)
	PORT->HDRVE |= 0x0C;
#	endif
#elif IFS_USBDIO_PORT == 1
#	if (IFS_USBDIO_DM == 0) || (IFS_USBDIO_DP == 0)
	PORT->HDRVE |= 0x30;
#	elif (IFS_USBDIO_DM == 24) || (IFS_USBDIO_DP == 24)
	PORT->HDRVE |= 0xC0;
#	endif
#endif

#ifdef IFS_USBDIO_DBG_PORT
	vsfhal_gpio_init(IFS_USBDIO_DBG_PORT);
	vsfhal_gpio_clear(IFS_USBDIO_DBG_PORT, 1 << IFS_USBDIO_DBG_PIN);
	vsfhal_gpio_config_pin(IFS_USBDIO_DBG_PORT, IFS_USBDIO_DBG_PIN, GPIO_OUTPP);
#endif

	SIM->SOPT &= ~SIM_SOPT_RSTPE_MASK;
	SIM->SCGC |= SIM_SCGC_IRQ_MASK;
	NVIC->ISER[IRQ_IRQn >> 0x05] = 1UL << (IRQ_IRQn & 0x1F);

	usbdio_set_mode(0);
#if 0
// test TX
uint8_t buff[] = {0x80, 0xFF};
nv32_usbdio_tx(buff, sizeof(buff));
#endif
	vsf_leave_critical();
	return VSFERR_NONE;
}

vsf_err_t nv32_usbdio_fini(void)
{
	return VSFERR_NONE;
}

// configure delay parameter for porting
#define VSFSDCD_NOP(n)						do{REPEAT_CODE((n), asm("nop");)}while (0)
#define VSFSDCD_RX_NOP_EVT()				VSFSDCD_NOP(1)

// DP fall edge interrupt may be served by asm(not C)
// nv32_usbdio_rx is the non-timing-critical part of the handler
ROOTFUNC void nv32_usbdio_rx(uint8_t *buff, uint8_t len)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + (IFS_USBDIO_PORT << 6));
#ifdef IFS_USBDIO_DBG_PORT
	GPIO_Type *debug = (GPIO_Type *)(GPIOA_BASE + (IFS_USBDIO_DBG_PORT << 6));
	uint32_t debug_value = 1 << IFS_USBDIO_DBG_PIN;
#endif
	enum usbdio_evt_t evt;

	if (!len)
	{
		// for SOF, the sample point MUST be j
		// for RST, MUST be k
		VSFSDCD_RX_NOP_EVT();
#ifdef IFS_USBDIO_DBG_PORT
		debug->PTOR = debug_value;
#endif
		if (gpio->PDIR & (1 << IFS_USBDIO_DP))
			evt = USBDIO_EVT_SOF;
		else
			evt = USBDIO_EVT_RST;
	}
	else
		evt = USBDIO_EVT_DAT;

#ifdef IFS_USBDIO_DBG_PORT
	debug->PTOR = debug_value;
	debug->PTOR = debug_value;
#endif
	if (nv32_usbdio_onrx != NULL)
		nv32_usbdio_onrx(evt, buff, len);
#ifdef IFS_USBDIO_DBG_PORT
	debug->PTOR = debug_value;
	debug->PTOR = debug_value;
	debug->PCOR = debug_value;
#endif
}

#define VSFSDCD_TX_NOP()					VSFSDCD_NOP(1)
extern void USBDIO_Tx(uint8_t *txbuf, uint16_t len);
vsf_err_t nv32_usbdio_tx(uint8_t *txbuf, uint16_t len)
{
#ifdef IFS_USBDIO_DBG_PORT
	GPIO_Type *debug = (GPIO_Type *)(GPIOA_BASE + (IFS_USBDIO_DBG_PORT << 6));
	uint32_t debug_value = 1 << IFS_USBDIO_DBG_PIN;
	debug->PTOR = debug_value;
	debug->PTOR = debug_value;
#endif
	usbdio_set_mode(1);
	VSFSDCD_TX_NOP();
	USBDIO_Tx(txbuf, len);
	usbdio_set_mode(0);
	return VSFERR_NONE;
}

#endif // IFS_USBDIO_EN

