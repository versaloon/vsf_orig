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

#include "core.h"

#define CORE_SYSTICK_TIMER					TIMER0

#define NUC400_CLK_PLLCTL_NR(x)				(((x) - 2) << 9)
#define NUC400_CLK_PLLCTL_NF(x)				(((x) - 2) << 0)

#define NUC400_CLK_PLLCTL_NO_1				(0x0UL << CLK_PLLCTL_OUTDV_Pos)
#define NUC400_CLK_PLLCTL_NO_2				(0x1UL << CLK_PLLCTL_OUTDV_Pos)
#define NUC400_CLK_PLLCTL_NO_4				(0x3UL << CLK_PLLCTL_OUTDV_Pos)

#define NUC400_CLK_CLKSEL0_HCLKSEL_HXT		(0x00UL << CLK_CLKSEL0_HCLKSEL_Pos)
#define NUC400_CLK_CLKSEL0_HCLKSEL_LXT		(0x01UL << CLK_CLKSEL0_HCLKSEL_Pos)
#define NUC400_CLK_CLKSEL0_HCLKSEL_PLL		(0x02UL << CLK_CLKSEL0_HCLKSEL_Pos)
#define NUC400_CLK_CLKSEL0_HCLKSEL_LIRC		(0x03UL << CLK_CLKSEL0_HCLKSEL_Pos)
#define NUC400_CLK_CLKSEL0_HCLKSEL_USBPLL	(0x04UL << CLK_CLKSEL0_HCLKSEL_Pos)
#define NUC400_CLK_CLKSEL0_HCLKSEL_HIRC		(0x07UL << CLK_CLKSEL0_HCLKSEL_Pos)

#define NUC400_CLK_CLKSEL1_TIM0SEL_HXT		(0x00UL << CLK_CLKSEL1_TMR0SEL_Pos)
#define NUC400_CLK_CLKSEL1_TIM0SEL_LXT		(0x01UL << CLK_CLKSEL1_TMR0SEL_Pos)
#define NUC400_CLK_CLKSEL1_TIM0SEL_PCLK		(0x02UL << CLK_CLKSEL1_TMR0SEL_Pos)
#define NUC400_CLK_CLKSEL1_TIM0SEL_EXTTRG	(0x03UL << CLK_CLKSEL1_TMR0SEL_Pos)
#define NUC400_CLK_CLKSEL1_TIM0SEL_LIRC		(0x05UL << CLK_CLKSEL1_TMR0SEL_Pos)
#define NUC400_CLK_CLKSEL1_TIM0SEL_HIRC		(0x07UL << CLK_CLKSEL1_TMR0SEL_Pos)

#define NUC400_TCSR_MODE_ONESHOT			(0x00UL << TIMER_CTL_OPMODE_Pos)
#define NUC400_TCSR_MODE_PERIODIC			(0x01UL << TIMER_CTL_OPMODE_Pos)
#define NUC400_TCSR_MODE_TOGGLE				(0x02UL << TIMER_CTL_OPMODE_Pos)
#define NUC400_TCSR_MODE_COUNTINUOUS		(0x03UL << TIMER_CTL_OPMODE_Pos)

static struct nuc400_info_t nuc400_info =
{
	0, CORE_VECTOR_TABLE,
	CORE_CLKEN, CORE_HCLKSRC, CORE_PCLKSRC, CORE_PLLSRC, 
	OSC0_FREQ_HZ, OSC32_FREQ_HZ, 22 * 1000 * 1000, 10 * 1000,
	CORE_PLL_FREQ_HZ, CPU_FREQ_HZ, HCLK_FREQ_HZ, PCLK_FREQ_HZ,
};

vsf_err_t nuc400_get_info(struct nuc400_info_t **info)
{
	*info = &nuc400_info;
	return VSFERR_NONE;
}

// Pendsv
struct nuc400_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static nuc400_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (nuc400_pendsv.on_pendsv != NULL)
	{
		nuc400_pendsv.on_pendsv(nuc400_pendsv.param);
	}
}

vsf_err_t nuc400_pendsv_config(void (*on_pendsv)(void *), void *param)
{
	nuc400_pendsv.on_pendsv = on_pendsv;
	nuc400_pendsv.param = param;

	if (nuc400_pendsv.on_pendsv != NULL)
	{
		SCB->SHP[10] = 0xFF;
	}
	return VSFERR_NONE;
}

vsf_err_t nuc400_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

void HardFault_Handler(void)
{
	while (1);
}

vsf_err_t nuc400_fini(void *p)
{
	return VSFERR_NONE;
}

vsf_err_t nuc400_reset(void *p)
{
	return VSFERR_NONE;
}

uint32_t nuc400_get_stack(void)
{
	return __get_MSP();
}

vsf_err_t nuc400_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void nuc400_sleep(uint32_t mode)
{
	vsf_leave_critical();
	__WFI();
}

void nuc400_unlock_reg(void)
{
	while(SYS->REGLCTL != SYS_REGLCTL_REGLCTL_Msk) {
        SYS->REGLCTL = 0x59;
        SYS->REGLCTL = 0x16;
        SYS->REGLCTL = 0x88;
    }
}

void nuc400_lock_reg(void)
{
	SYS->REGLCTL = 0;
}

vsf_err_t nuc400_init(void *p)
{
	uint32_t temp32;
	uint32_t freq_in;
	
	if (p != NULL)
	{
		nuc400_info = *(struct nuc400_info_t *)p;
	}

	nuc400_unlock_reg();
	
	// enable clocks
	CLK->PWRCTL = 0x18;
	CLK->PWRCTL |= nuc400_info.clk_enable;
	temp32 = nuc400_info.clk_enable;
	temp32 = ((temp32 & NUC400_CLK_HXT) ? CLK_STATUS_HXTSTB_Msk : 0) |
				((temp32 & NUC400_CLK_LXT) ? CLK_STATUS_LXTSTB_Msk : 0) |
				((temp32 & NUC400_CLK_HIRC) ? CLK_STATUS_HIRCSTB_Msk : 0) |
				((temp32 & NUC400_CLK_LIRC) ? CLK_STATUS_LIRCSTB_Msk : 0);
	while ((CLK->STATUS & temp32) != temp32);

	// enable PLLs
	if (nuc400_info.pllsrc != NUC400_PLLSRC_NONE)
	{
		uint8_t no;
		uint32_t no_mask;

		switch (nuc400_info.pllsrc)
		{
		case NUC400_PLLSRC_HXT:
			temp32 = 0;
			freq_in = nuc400_info.osc_freq_hz;
			break;
		case NUC400_PLLSRC_HIRC:
			temp32 = CLK_PLLCTL_PLLSRC_Pos;
			freq_in = nuc400_info.hirc_freq_hz;
			break;
		default:
			return VSFERR_INVALID_PARAMETER;
		}
		// Fin/NR: 2MHz
		if ((nuc400_info.pll_freq_hz * 1 > (120 * 1000 * 1000)) &&
				(nuc400_info.pll_freq_hz * 1 < (200 * 1000 * 1000)))
		{
			no = 1;
			no_mask = NUC400_CLK_PLLCTL_NO_1;
		}
		else if ((nuc400_info.pll_freq_hz * 2 > (120 * 1000 * 1000)) &&
				(nuc400_info.pll_freq_hz * 2 < (200 * 1000 * 1000)))
		{
			no = 2;
			no_mask = NUC400_CLK_PLLCTL_NO_2;
		}
		else if ((nuc400_info.pll_freq_hz * 4 > (120 * 1000 * 1000)) &&
				(nuc400_info.pll_freq_hz * 4 < (200 * 1000 * 1000)))
		{
			no = 4;
			no_mask = NUC400_CLK_PLLCTL_NO_4;
		}
		else
		{
			return VSFERR_INVALID_PARAMETER;
		}

		CLK->PLLCTL = temp32 | no_mask |
			NUC400_CLK_PLLCTL_NR(freq_in / 2000000) |
			NUC400_CLK_PLLCTL_NF(nuc400_info.pll_freq_hz * no / 2000000);
		while ((CLK->STATUS & CLK_STATUS_PLLSTB_Msk) != CLK_STATUS_PLLSTB_Msk);
	}
	
	// set hclk
	switch (nuc400_info.hclksrc)
	{
	case NUC400_HCLKSRC_HIRC:
		freq_in = nuc400_info.hirc_freq_hz;
		temp32 = NUC400_CLK_CLKSEL0_HCLKSEL_HIRC;
		break;
	case NUC400_HCLKSRC_PLL2FOUT:
		freq_in = 480 * 1000 * 1000;
		temp32 = NUC400_CLK_CLKSEL0_HCLKSEL_USBPLL;
		break;
	case NUC400_HCLKSRC_LIRC:
		freq_in = nuc400_info.lirc_freq_hz;
		temp32 = NUC400_CLK_CLKSEL0_HCLKSEL_LIRC;
		break;
	case NUC400_HCLKSRC_PLLFOUT:
		freq_in = nuc400_info.pll_freq_hz;
		temp32 = NUC400_CLK_CLKSEL0_HCLKSEL_PLL;
		break;
	case NUC400_HCLKSRC_LXT:
		freq_in = nuc400_info.osc32k_freq_hz;
		temp32 = NUC400_CLK_CLKSEL0_HCLKSEL_LXT;
		break;
	case NUC400_HCLKSRC_HXT:
		freq_in = nuc400_info.osc_freq_hz;
		temp32 = NUC400_CLK_CLKSEL0_HCLKSEL_HXT;
		break;
	default:
		return VSFERR_INVALID_PARAMETER;
	}
	CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_HCLKDIV_Msk) |
			((freq_in / nuc400_info.hclk_freq_hz) - 1);
	CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | temp32;


	if (nuc400_info.cpu_freq_hz <= 24 * 1000 * 1000)
	{
		FMC->FTCTL = 0x1 << 4;
	}
	else if (nuc400_info.cpu_freq_hz <= 48 * 1000 * 1000)
	{
		FMC->FTCTL = 0x2 << 4;
	}
	else if (nuc400_info.cpu_freq_hz <= 72 * 1000 * 1000)
	{
		FMC->FTCTL = 0x3 << 4;
	}
	
#if (VSFHAL_USBD_EN | VSFHAL_HCD_EN)
	CLK->AHBCLK |= CLK_AHBCLK_USBDCKEN_Msk;
	SYS->USBPHY = 0x100;
	USBD->PHYCTL |= USBD_PHYCTL_PHYEN_Msk;
	while (USBD->EPAMPS != 0x20)
	{
		USBD->EPAMPS = 0x20;
	}	
	while (USBD->EPAMPS != 0)
	{
		USBD->EPAMPS = 0;
	}		
#endif
	
	nuc400_lock_reg();

	SCB->VTOR = nuc400_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | nuc400_info.priority_group;
	return VSFERR_NONE;
}

uint32_t nuc400_uid_get(uint8_t *buffer, uint32_t size)
{
	return 0;
}

static void (*tickclk_callback)(void *param) = NULL;
static void *tickclk_param = NULL;
static volatile uint32_t tickcnt = 0;

static uint32_t tickclk_get_count_local(void)
{
	return tickcnt;
}

uint32_t nuc400_tickclk_get_count(void)
{
	uint32_t count1, count2;

	do {
		count1 = tickclk_get_count_local();
		count2 = tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

ROOTFUNC void SysTick_Handler(void)
{
	tickcnt++;
	if (tickclk_callback != NULL)
	{
		tickclk_callback(tickclk_param);
	}
}

vsf_err_t nuc400_tickclk_config_cb(void (*callback)(void*), void *param)
{
	tickclk_callback = callback;
	tickclk_param = param;
	return VSFERR_NONE;
}

vsf_err_t nuc400_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t nuc400_tickclk_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t nuc400_tickclk_init(void)
{
	SysTick->LOAD = nuc400_info.cpu_freq_hz / 1000;
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	return VSFERR_NONE;
}

vsf_err_t nuc400_tickclk_fini(void)
{
	return nuc400_tickclk_stop();
}
