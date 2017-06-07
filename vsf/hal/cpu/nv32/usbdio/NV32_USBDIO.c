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

#include "app_cfg.h"
#include "app_type.h"
#include "vsfhal.h"
#include "compiler.h"

#if VSFHAL_USBDIO_EN
// some check of the configurations
#if ((VSFHAL_USBDIO_PORT == 0) &&\
		((VSFHAL_USBDIO_DP != 12) && (VSFHAL_USBDIO_DP != 13) && (VSFHAL_USBDIO_DP != 24) && (VSFHAL_USBDIO_DP != 25) ||\
		 (VSFHAL_USBDIO_DM != 12) && (VSFHAL_USBDIO_DM != 13) && (VSFHAL_USBDIO_DM != 24) && (VSFHAL_USBDIO_DM != 25))) ||\
	((VSFHAL_USBDIO_PORT == 1) &&\
		((VSFHAL_USBDIO_DP != 0) && (VSFHAL_USBDIO_DP != 1) && (VSFHAL_USBDIO_DP != 24) && (VSFHAL_USBDIO_DP != 25) ||\
		 (VSFHAL_USBDIO_DM != 0) && (VSFHAL_USBDIO_DM != 1) && (VSFHAL_USBDIO_DM != 24) && (VSFHAL_USBDIO_DM != 25)))
#	error "DP and DM MUST be High Drive PINs"
#endif
#if (VSFHAL_USBDIO_DM != VSFHAL_USBDIO_DP + 1) && (VSFHAL_USBDIO_DM != VSFHAL_USBDIO_DP - 1)
#	error "DP and DM MUST be consecutive PINs
#endif

// NOTE PB12 is used for debug if enabled
#define USBDIO_DBG_PORT			0
#define USBDIO_DBG_PIN			0

//#define USBDIO_RX_LOOP
#define USBDIO_TX_LOOP

// VSFHAL_USBDIO_PORT
// VSFHAL_USBDIO_DP, VSFHAL_USBDIO_DM
static void (*nv32_usbdio_onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len);

static void usbdio_set_mode(uint8_t in0_out1)
{
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + (VSFHAL_USBDIO_PORT << 6));
	if (in0_out1)
	{
		// disable EXTI before set to output
		IRQ->SC = 0;

		gpio->PIDR |= (1 << VSFHAL_USBDIO_DP) | (1 << VSFHAL_USBDIO_DM);
		gpio->PDDR |= (1 << VSFHAL_USBDIO_DP) | (1 << VSFHAL_USBDIO_DM);
	}
	else
	{
		gpio->PIDR &= ~((1 << VSFHAL_USBDIO_DP) | (1 << VSFHAL_USBDIO_DM));
		gpio->PDDR &= ~((1 << VSFHAL_USBDIO_DP) | (1 << VSFHAL_USBDIO_DM));

		// enable EXTI after set to input
		IRQ->SC = IRQ_SC_IRQPE_MASK | IRQ_SC_IRQIE_MASK;
	}
}

vsf_err_t nv32_usbdio_init(void (*onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len))
{
	nv32_usbdio_onrx = onrx;
	vsfhal_gpio_init(VSFHAL_USBDIO_PORT);
	vsfhal_gpio_set(VSFHAL_USBDIO_PORT, 1 << VSFHAL_USBDIO_DP);
	// enable High Drive
#if VSFHAL_USBDIO_PORT == 0
#	if (VSFHAL_USBDIO_DM == 12) || (VSFHAL_USBDIO_DP == 12)
	PORT->HDRVE |= 0x03;
#	elif (VSFHAL_USBDIO_DM == 24) || (VSFHAL_USBDIO_DP == 24)
	PORT->HDRVE |= 0x0C;
#	endif
#elif VSFHAL_USBDIO_PORT == 1
#	if (VSFHAL_USBDIO_DM == 0) || (VSFHAL_USBDIO_DP == 0)
	PORT->HDRVE |= 0x30;
#	elif (VSFHAL_USBDIO_DM == 24) || (VSFHAL_USBDIO_DP == 24)
	PORT->HDRVE |= 0xC0;
#	endif
#endif

#ifdef USBDIO_DBG_PORT
	vsfhal_gpio_init(USBDIO_DBG_PORT);
	vsfhal_gpio_clear(USBDIO_DBG_PORT, 1 << USBDIO_DBG_PIN);
	vsfhal_gpio_config_pin(USBDIO_DBG_PORT, USBDIO_DBG_PIN, GPIO_OUTPP);
#endif

	// CRC is enabled at 48MHz sys_clock
	{
		struct nv32_info_t *info;
		nv32_get_info(&info);
		if (info->sys_freq_hz == 48000000)
		{
			SIM->SCGC |= SIM_SCGC_CRC_MASK;
			CRC0->CTRL = 0;
			CRC0->GPOLY = 0x0000A001;
		}
	}

	SIM->SOPT &= ~SIM_SOPT_RSTPE_MASK;
	SIM->SCGC |= SIM_SCGC_IRQ_MASK;
	NVIC_EnableIRQ(IRQ_IRQn);

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
	GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + (VSFHAL_USBDIO_PORT << 6));
#ifdef USBDIO_DBG_PORT
	GPIO_Type *debug = (GPIO_Type *)(GPIOA_BASE + (USBDIO_DBG_PORT << 6));
	uint32_t debug_value = 1 << USBDIO_DBG_PIN;
#endif
	enum usbdio_evt_t evt;

	if (!len)
	{
		// for SOF, the sample point MUST be j
		// for RST, MUST be k
		VSFSDCD_RX_NOP_EVT();
#ifdef USBDIO_DBG_PORT
		debug->PTOR = debug_value;
#endif
		if (gpio->PDIR & (1 << VSFHAL_USBDIO_DP))
			evt = USBDIO_EVT_SOF;
		else
			evt = USBDIO_EVT_RST;
	}
	else
		evt = USBDIO_EVT_DAT;

#ifdef USBDIO_DBG_PORT
	debug->PTOR = debug_value;
	debug->PTOR = debug_value;
#endif
	if (nv32_usbdio_onrx != NULL)
		nv32_usbdio_onrx(evt, buff, len);
#ifdef USBDIO_DBG_PORT
	debug->PTOR = debug_value;
	debug->PTOR = debug_value;
	debug->PCOR = debug_value;
#endif
}

#define VSFSDCD_TX_NOP()					VSFSDCD_NOP(1)
extern void USBDIO_Tx(uint8_t *txbuf, uint16_t len);
vsf_err_t nv32_usbdio_tx(uint8_t *txbuf, uint16_t len)
{
#ifdef USBDIO_DBG_PORT
	GPIO_Type *debug = (GPIO_Type *)(GPIOA_BASE + (USBDIO_DBG_PORT << 6));
	uint32_t debug_value = 1 << USBDIO_DBG_PIN;
	debug->PTOR = debug_value;
	debug->PTOR = debug_value;
#endif
	usbdio_set_mode(1);
	VSFSDCD_TX_NOP();
	USBDIO_Tx(txbuf, len);
	usbdio_set_mode(0);
	return VSFERR_NONE;
}

#endif // VSFHAL_USBDIO_EN

