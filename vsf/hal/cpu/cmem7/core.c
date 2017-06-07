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

#include "cmem7.h"
#include "core.h"

static struct cmem7_info_t cmem7_info =
{
	200000000, 0, CORE_VECTOR_TABLE,
};

// Pendsv
struct cmem7_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static cmem7_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (cmem7_pendsv.on_pendsv != NULL)
	{
		cmem7_pendsv.on_pendsv(cmem7_pendsv.param);
	}
}

vsf_err_t cmem7_pendsv(void (*on_pendsv)(void *), void *param)
{
	cmem7_pendsv.on_pendsv = on_pendsv;
	cmem7_pendsv.param = param;

	if (cmem7_pendsv.on_pendsv != NULL)
	{
		SCB->SHP[10] = 0xFF;
	}
	return VSFERR_NONE;
}

void HardFault_Handler(void)
{
	while (1);
}

vsf_err_t cmem7_get_info(struct cmem7_info_t **info)
{
	*info = &cmem7_info;
	return VSFERR_NONE;
}

vsf_err_t cmem7_fini(void *p)
{
	return VSFERR_NONE;
}

vsf_err_t cmem7_reset(void *p)
{
	// TODO
	return VSFERR_NONE;
}

uint32_t cmem7_get_stack(void)
{
	// TODO
	//return __get_MSP();
	return 0;
}

vsf_err_t cmem7_set_stack(uint32_t sp)
{
	// TODO
	//__set_MSP(sp);
	return VSFERR_NONE;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void cmem7_sleep(uint32_t mode)
{
	// TODO
	vsf_leave_critical();
	/*
	if (mode == cmem7_SLEEP_WFI) // sleep
	{
		SCB->SCR &= ~0x4ul;
	}
	else // power down
	{
		SCB->SCR |= 0x4ul;
		CLK->PWRCTL |= CLK_PWRCTL_PDWKIF_Msk;
	}
	__WFI();
	*/
}

vsf_err_t cmem7_init(void *p)
{
	GLOBAL_CTRL->CLK_SEL_1_b.SYS_CLK = 3;
	GLOBAL_CTRL->NMI_SEL_b.NMI = PAD_INT_IRQn;

	SCB->VTOR = cmem7_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | cmem7_info.priority_group | (0x7ul << 8);
	return VSFERR_NONE;
}

static void (*cmem7_tickclk_callback)(void *param) = NULL;
static void *cmem7_tickclk_param = NULL;
static volatile uint32_t cmem7_tickcnt = 0;

vsf_err_t cmem7_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t cmem7_tickclk_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

static uint32_t cmem7_tickclk_get_count_local(void)
{
	return cmem7_tickcnt;
}

uint32_t cmem7_tickclk_get_count(void)
{
	uint32_t count1, count2;

	do {
		count1 = cmem7_tickclk_get_count_local();
		count2 = cmem7_tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

vsf_err_t cmem7_tickclk_set_callback(void (*callback)(void*), void *param)
{
	cmem7_tickclk_param = param;
	cmem7_tickclk_callback = callback;
	return VSFERR_NONE;
}

vsf_err_t cmem7_tickclk_init(void)
{
	cmem7_tickcnt = 0;
	SysTick->LOAD = 200000000 / 1000;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
	NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
	return VSFERR_NONE;
}

vsf_err_t cmem7_tickclk_fini(void)
{
	return cmem7_tickclk_stop();
}

ROOTFUNC void SysTick_Handler(void)
{
	cmem7_tickcnt++;
	if (cmem7_tickclk_callback != NULL)
	{
		cmem7_tickclk_callback(cmem7_tickclk_param);
	}
}
