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

#if VSFHAL_HCD_EN

#include "NUC505_OHCI.h"
#include "NUC505Series.h"
#include "core.h"

extern void nuc505_unlock_reg(void);
extern void nuc505_lock_reg(void);

//GPB_MFPL_PB0MFP
#define SYS_GPB_MFPL_PB0MFP_GPIO			(0x0UL<<SYS_GPB_MFPL_PB0MFP_Pos)
#define SYS_GPB_MFPL_PB0MFP_USB0_OTG5V_ST	(0x1UL<<SYS_GPB_MFPL_PB0MFP_Pos)
#define SYS_GPB_MFPL_PB0MFP_I2C4_SCL		(0x2UL<<SYS_GPB_MFPL_PB0MFP_Pos)
#define SYS_GPB_MFPL_PB0MFP_INT11			(0x8UL<<SYS_GPB_MFPL_PB0MFP_Pos)

//GPB_MFPL_PB1MFP
#define SYS_GPB_MFPL_PB1MFP_GPIO			(0x0UL<<SYS_GPB_MFPL_PB1MFP_Pos)
#define SYS_GPB_MFPL_PB1MFP_USB0_OTG5V_EN	(0x1UL<<SYS_GPB_MFPL_PB1MFP_Pos)
#define SYS_GPB_MFPL_PB1MFP_I2C4_SDA		(0x2UL<<SYS_GPB_MFPL_PB1MFP_Pos)
#define SYS_GPB_MFPL_PB1MFP_TM1_CNT_OUT		(0x3UL<<SYS_GPB_MFPL_PB1MFP_Pos)

//GPB_MFPL_PB2MFP
#define SYS_GPB_MFPL_PB2MFP_GPIO			(0x0UL<<SYS_GPB_MFPL_PB2MFP_Pos)
#define SYS_GPB_MFPL_PB2MFP_UART1_RXD		(0x1UL<<SYS_GPB_MFPL_PB2MFP_Pos)
#define SYS_GPB_MFPL_PB2MFP_SPI2_SS0		(0x2UL<<SYS_GPB_MFPL_PB2MFP_Pos)
#define SYS_GPB_MFPL_PB2MFP_USB1_D_N		(0x3UL<<SYS_GPB_MFPL_PB2MFP_Pos)
#define SYS_GPB_MFPL_PB2MFP_EBI_AD4			(0x7UL<<SYS_GPB_MFPL_PB2MFP_Pos)

//GPB_MFPL_PB3MFP
#define SYS_GPB_MFPL_PB3MFP_GPIO			(0x0UL<<SYS_GPB_MFPL_PB3MFP_Pos)
#define SYS_GPB_MFPL_PB3MFP_UART1_TXD		(0x1UL<<SYS_GPB_MFPL_PB3MFP_Pos)
#define SYS_GPB_MFPL_PB3MFP_SPI2_CLK		(0x2UL<<SYS_GPB_MFPL_PB3MFP_Pos)
#define SYS_GPB_MFPL_PB3MFP_USB1_D_P		(0x3UL<<SYS_GPB_MFPL_PB3MFP_Pos)
#define SYS_GPB_MFPL_PB3MFP_EBI_AD5			(0x7UL<<SYS_GPB_MFPL_PB3MFP_Pos)

struct nuc505_ohci_irq_t
{
	void *param;
	vsf_err_t (*irq)(void*);
} static nuc505_ohci_irq;

static uint8_t port_enable_mask;

ROOTFUNC void USBH_IRQHandler(void)
{
	if (nuc505_ohci_irq.irq != NULL)
	{
		nuc505_ohci_irq.irq(nuc505_ohci_irq.param);
	}
}

vsf_err_t nuc505_hcd_init(uint32_t index, vsf_err_t (*ohci_irq)(void *), void *param)
{
	uint8_t div;
	struct nuc505_info_t *info;

	if ((index & 0x3) == 0)
		return VSFERR_NOT_SUPPORT;

	if (ohci_irq != NULL)
	{
		nuc505_ohci_irq.irq = ohci_irq;
		nuc505_ohci_irq.param = param;
	}

	if (port_enable_mask == 0)
	{
		// usbh clock 48M
		nuc505_get_info(&info);
		div = info->pll_freq_hz / 48000000;
		CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_USBHDIV_Msk) | 
				CLK_CLKDIV0_USBHSEL_Msk | ((div - 1) << CLK_CLKDIV0_USBHDIV_Pos);
		CLK->AHBCLK |= CLK_AHBCLK_USBHCKEN_Msk;

		NVIC_SetPriority(USBH_IRQn,5);
		NVIC_EnableIRQ(USBH_IRQn);
	}
	if ((index & nuc505_HCD_PORT1) && !(port_enable_mask & nuc505_HCD_PORT1))
	{
		#if OHCI_PORT1_PB12_DP_ENABLE
		PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL12_Msk) | (0x2ul << GPIO_PUEN_PULLSEL12_Pos);
		SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB12MFP_Msk) |
						(2 << SYS_GPB_MFPH_PB12MFP_Pos);
		#endif
		#if OHCI_PORT1_PB13_DM_ENABLE
		PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL13_Msk) | (0x2ul << GPIO_PUEN_PULLSEL13_Pos);
		SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB13MFP_Msk) |
						(2 << SYS_GPB_MFPH_PB13MFP_Pos);
		#endif
		#if OHCI_PORT1_PB14_DP_ENABLE
		PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL14_Msk) | (0x2ul << GPIO_PUEN_PULLSEL14_Pos);
		SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB14MFP_Msk) |
						(1 << SYS_GPB_MFPH_PB14MFP_Pos);
		#endif
		#if OHCI_PORT1_PB15_DM_ENABLE
		PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL15_Msk) | (0x2ul << GPIO_PUEN_PULLSEL15_Pos);
		SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB15MFP_Msk) |
						(1 << SYS_GPB_MFPH_PB15MFP_Pos);
		#endif
		port_enable_mask |= nuc505_HCD_PORT1;
	}
	if ((index & nuc505_HCD_PORT2) && !(port_enable_mask & nuc505_HCD_PORT2))
	{
		PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL13_Msk) | (0x2ul << GPIO_PUEN_PULLSEL13_Pos);
		PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL14_Msk) | (0x2ul << GPIO_PUEN_PULLSEL14_Pos);
		SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC13MFP_Msk | SYS_GPC_MFPH_PC14MFP_Msk)) |
						(1 << SYS_GPC_MFPH_PC13MFP_Pos) | (1 << SYS_GPC_MFPH_PC14MFP_Pos);
		port_enable_mask |= nuc505_HCD_PORT2;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_hcd_fini(uint32_t index)
{
	if (port_enable_mask == 0)
		return VSFERR_NONE;
	if ((index & 0x3) == 0)
		return VSFERR_NOT_SUPPORT;

	// TODO
	if ((index & nuc505_HCD_PORT1) && (port_enable_mask & nuc505_HCD_PORT1))
	{
		#if OHCI_PORT1_PB12_DP_ENABLE
		SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB12MFP_Msk;
		#endif
		#if OHCI_PORT1_PB13_DM_ENABLE
		SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB13MFP_Msk;
		#endif
		#if OHCI_PORT1_PB14_DP_ENABLE
		SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB14MFP_Msk;
		#endif
		#if OHCI_PORT1_PB15_DM_ENABLE
		SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB15MFP_Msk;
		#endif
		port_enable_mask &= ~nuc505_HCD_PORT1;
	}
	if ((index & nuc505_HCD_PORT2) && (port_enable_mask & nuc505_HCD_PORT2))
	{
		SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC13MFP_Msk | SYS_GPC_MFPH_PC14MFP_Msk);
		port_enable_mask &= ~nuc505_HCD_PORT2;
	}

	if (port_enable_mask == 0)
	{
		NVIC_DisableIRQ(USBH_IRQn);
		CLK->AHBCLK &= ~CLK_AHBCLK_USBHCKEN_Msk;
	}

	return VSFERR_NONE;
}

void* nuc505_hcd_regbase(uint32_t index)
{
	switch (index >> 16)
	{
	case 0:
		return (void*)USBH;
	default:
		return NULL;
	}
}

#endif
