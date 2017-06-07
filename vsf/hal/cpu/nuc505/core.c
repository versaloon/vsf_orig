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

#include "app_type.h"
#include "compiler.h"
#include "vsfhal.h"

#include "NUC505Series.h"
#include "core.h"

static struct nuc505_info_t nuc505_info =
{
	0, CORE_VECTOR_TABLE,
	CORE_CLKEN,
	CORE_HCLKSRC,
	OSC_FREQ_HZ, OSC32_FREQ_HZ, LIRC_FREQ_HZ,
	CORE_PLL_FREQ_HZ, CORE_APLL_FREQ_HZ, CPU_FREQ_HZ, HCLK_FREQ_HZ, PCLK_FREQ_HZ,
};

// Pendsv
struct nuc505_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static nuc505_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (nuc505_pendsv.on_pendsv != NULL)
	{
		nuc505_pendsv.on_pendsv(nuc505_pendsv.param);
	}
}

vsf_err_t nuc505_pendsv_config(void (*on_pendsv)(void *), void *param)
{
	nuc505_pendsv.on_pendsv = on_pendsv;
	nuc505_pendsv.param = param;

	if (nuc505_pendsv.on_pendsv != NULL)
	{
		SCB->SHP[10] = 0xFF;
	}
	return VSFERR_NONE;
}

vsf_err_t nuc505_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

void HardFault_Handler(void)
{
	while (1);
}

vsf_err_t nuc505_get_info(struct nuc505_info_t **info)
{
	*info = &nuc505_info;
	return VSFERR_NONE;
}

vsf_err_t nuc505_fini(void *p)
{
	return VSFERR_NONE;
}

vsf_err_t nuc505_reset(void *p)
{
	SYS->IPRST0 |= SYS_IPRST0_CHIPRST_Msk;
	return VSFERR_NONE;
}

uint32_t nuc505_get_stack(void)
{
	return __get_MSP();
}

vsf_err_t nuc505_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void nuc505_sleep(uint32_t mode)
{
	vsf_leave_critical();
	if (mode == nuc505_SLEEP_WFI) // sleep
	{
		SCB->SCR &= ~0x4ul;
	}
	else // power down
	{
		SCB->SCR |= 0x4ul;
		CLK->PWRCTL |= CLK_PWRCTL_PDWKIF_Msk;
	}
	CLK->PWRCTL &= ~CLK_PWRCTL_HXTEN_Msk;
	__WFI();
}

vsf_err_t nuc505_init(void *p)
{
	uint32_t temp32, reg32;

	if (p != NULL)
	{
		nuc505_info = *(struct nuc505_info_t *)p;
	}

	//set spim with highspeed 4bit operation
	SPIM->CTL0 = 0xEBF00003;

	if (nuc505_info.osc_freq_hz != (12 * 1000 * 1000))
		return VSFERR_INVALID_PARAMETER;

	CLK->PWRCTL |= CLK_PWRCTL_PDWKIEN_Msk | CLK_PWRCTL_PDWTCPU_Msk;

	// switch HCLK to HXT
	CLK->CLKDIV0 &= ~CLK_CLKDIV0_HCLKSEL_Msk;

	if (nuc505_info.clk_enable & NUC505_CLK_LXT)
	{
		RTC->SET = RTC_SET_CBEN_Msk | RTC_SET_IOMSEL_Msk;
		//RTC->SET = RTC_SET_CBEN_Msk;
		RTC->CLKSRC &= ~RTC_CLKSRC_CKSRC_Msk;
	}
	else if (nuc505_info.clk_enable & NUC505_CLK_LIRC)
	{
		RTC->SET = 0;
		RTC->SET = RTC_CLKSRC_CKSRC_Msk;
	}

	if (nuc505_info.clk_enable & NUC505_CLK_PLL)
	{
		uint32_t n, m, p;

		for (p = 1; p <= 8; p++)
		{
			if ((nuc505_info.pll_freq_hz * p >= (300 * 1000 * 1000)) &&
					(nuc505_info.pll_freq_hz * p < (1000 * 1000 * 1000)))
			break;
		}
		if ((p == 8) &&
			((nuc505_info.pll_freq_hz * 8 < (300 * 1000 * 1000)) ||
				(nuc505_info.pll_freq_hz * 8 >= (1000 * 1000 * 1000))))
			return VSFERR_INVALID_PARAMETER;

		if (nuc505_info.pll_freq_hz * p > (768 * 1000 * 1000))
			m = 1;
		else
			m = 2;

		n = nuc505_info.pll_freq_hz * p * m / nuc505_info.osc_freq_hz;
		if ((n < 1) || (n > 128))
			return VSFERR_INVALID_PARAMETER;

		CLK->PLLCTL = (n - 1) + ((m - 1) << 7) + ((p - 1) << 13);
	}
	else
	{
		CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;
	}

	// TODO
	if (nuc505_info.clk_enable & NUC505_CLK_APLL)
	{

	}
	else
	{
		CLK->APLLCTL |= CLK_APLLCTL_PD_Msk;
	}

    // set pclk
	temp32 = nuc505_info.hclk_freq_hz / nuc505_info.pclk_freq_hz;
	if ((temp32 < 1) || (temp32 > 16))
		return VSFERR_INVALID_PARAMETER;
	CLK->CLKDIV0 = (CLK->CLKDIV0 & (~CLK_CLKDIV0_PCLKDIV_Msk)) |
					((temp32 - 1) << CLK_CLKDIV0_PCLKDIV_Pos);

	// set hclk
	switch (nuc505_info.hclksrc)
	{
	case NUC505_HCLKSRC_PLLFOUT:
		temp32 = nuc505_info.pll_freq_hz;
		reg32 = CLK_CLKDIV0_HCLKSEL_Msk;
		break;
	case NUC505_HCLKSRC_HXT:
		temp32 = nuc505_info.osc_freq_hz;
		reg32 = 0;
		break;
	default:
		return VSFERR_INVALID_PARAMETER;
	}
	temp32 /= nuc505_info.hclk_freq_hz;
	if ((temp32 < 1) || (temp32 > 16))
		return VSFERR_INVALID_PARAMETER;
	CLK->CLKDIV0 = ((CLK->CLKDIV0 | reg32) & ~CLK_CLKDIV0_HCLKDIV_Msk) | (temp32 - 1);

	SCB->VTOR = nuc505_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | nuc505_info.priority_group;
	return VSFERR_NONE;
}

// tickclk
#define CM3_SYSTICK_ENABLE				(1 << 0)
#define CM3_SYSTICK_INT					(1 << 1)
#define CM3_SYSTICK_CLKSOURCE			(1 << 2)
#define CM3_SYSTICK_COUNTFLAG			(1 << 16)

static void (*nuc505_tickclk_cb)(void *param) = NULL;
static void *nuc505_tickclk_param = NULL;
static uint32_t nuc505_tickcnt = 0;
vsf_err_t nuc505_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= CM3_SYSTICK_ENABLE;
	return VSFERR_NONE;
}

vsf_err_t nuc505_tickclk_stop(void)
{
	SysTick->CTRL &= ~CM3_SYSTICK_ENABLE;
	return VSFERR_NONE;
}

static uint32_t nuc505_tickclk_get_count_local(void)
{
	return nuc505_tickcnt;
}

uint32_t nuc505_tickclk_get_count(void)
{
	uint32_t count1, count2;

	do {
		count1 = nuc505_tickclk_get_count_local();
		count2 = nuc505_tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

ROOTFUNC void SysTick_Handler(void)
{
	nuc505_tickcnt++;
	if (nuc505_tickclk_cb != NULL)
	{
		nuc505_tickclk_cb(nuc505_tickclk_param);
	}
}

vsf_err_t nuc505_tickclk_config_cb(void (*callback)(void*), void *param)
{
	uint32_t tmp = SysTick->CTRL;

	SysTick->CTRL &= ~CM3_SYSTICK_INT;
	nuc505_tickclk_cb = callback;
	nuc505_tickclk_param = param;
	SysTick->CTRL = tmp;
	return VSFERR_NONE;
}

vsf_err_t nuc505_tickclk_init(void)
{
	nuc505_tickcnt = 0;
	SysTick->LOAD = nuc505_info.cpu_freq_hz / 1000;
	SysTick->CTRL = CM3_SYSTICK_INT | CM3_SYSTICK_CLKSOURCE;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	return VSFERR_NONE;
}

vsf_err_t nuc505_tickclk_fini(void)
{
	return nuc505_tickclk_stop();
}

uint32_t cortexm_get_pc(void)
{
	uint32_t pc;
	asm("MOV	%0,	pc" : "=r" (pc));
	return pc;
}

// special
int32_t nuc505_is_running_on_ram(void)
{
	uint32_t vecmap_addr = SYS->RVMPADDR, vecmap_len = SYS->RVMPLEN >> 14;
	uint32_t pc = cortexm_get_pc();

	if (((pc >= 0x20000000) && (pc < 0x20020000)) ||
		((pc >= 0x1ff00000) && (pc < 0x1ff20000)) ||
		((vecmap_addr != 0) && (pc < vecmap_len)))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

vsf_err_t nuc505_code_map(uint8_t en, uint8_t rst, uint8_t len_kb, uint32_t addr)
{
	if (!en)
	{
		SYS->LVMPADDR = 0;
		SYS->LVMPLEN = 0;
	}
	else if ((len_kb > 0) && (len_kb <= 128))
	{
		memcpy((void *)0x20000000, (void *)addr, 1024ul * len_kb);
		SYS->LVMPADDR = 0x20000000;
		SYS->LVMPLEN = len_kb;
	}
	else
	{
		return VSFERR_INVALID_PARAMETER;
	}

	if (rst)
	{
		SYS->IPRST0 |= SYS_IPRST0_CPURST_Msk;
	}
	else
	{
		SYS->RVMPLEN |= 0x1;
	}
	return VSFERR_NONE;
}

