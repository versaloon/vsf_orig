/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       GPIO.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    GPIO interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

#include "app_type.h"
#include "compiler.h"
#include "interfaces.h"

#if IFS_EINT_EN

#include "NUC505Series.h"
#include "NUC505_EINT.h"
#include "GPIO/NUC505_GPIO.h"

#define NUC505_EINT_INDEX_NUM	4

struct
{
	uint8_t allocated;
	uint8_t enable;
	uint8_t eint_idx;
	uint8_t port_idx;
	void (*callback)(void *);
	void *param;
} static eint_info[EINT_NUM];

vsf_err_t nuc505_eint_init(uint32_t index)
{
	uint8_t eint_idx = index & 0x0F;

	// INTCTL
	*(uint32_t *)(GPIOA_BASE + 0xa0) = GPIO_INTCTL_INTCTL_Msk | 0xf0;
	// LATCHDAT
	//*(uint32_t *)(GPIOA_BASE + 0xa4) = 0xffff;
	//*(uint32_t *)(GPIOA_BASE + 0xa8) = 0xffff;
	//*(uint32_t *)(GPIOA_BASE + 0xac) = 0xffff;
	//*(uint32_t *)(GPIOA_BASE + 0xb0) = 0xffff;

	NVIC_EnableIRQ((IRQn_Type)(EINT0_IRQn + eint_idx));

	return VSFERR_NONE;
}

vsf_err_t nuc505_eint_fini(uint32_t index)
{
	uint8_t eint_idx = index & 0x0F;

	NVIC_DisableIRQ((IRQn_Type)(EINT0_IRQn + eint_idx));

	return VSFERR_NONE;
}

// need configure related gpio first
vsf_err_t nuc505_eint_config(uint32_t index, uint32_t type,
			uint32_t int_priority, void *param, void (*callback)(void *param))
{
	uint8_t i, eint_idx = index & 0x0F, port_idx = (index & 0xF0) >> 4;

	// Input mode , no pull up / pull down
	nuc505_gpio_config(eint_idx, 0x1ul << port_idx, 0, 0, 0);

	for (i =0; i < EINT_NUM; i++)
	{
		if (eint_info[i].allocated == 0)
			break;
	}
	if (i >= EINT_NUM)
		return VSFERR_NOT_ENOUGH_RESOURCES;

	if (type & EINT_ONFALL)
	{
		// INTEN
		*(uint32_t *)(GPIOA_BASE + 0x90 + 0x4 * eint_idx) |= 0x1ul << port_idx;
	}
	else
	{
		// INTEN
		*(uint32_t *)(GPIOA_BASE + 0x90 + 0x4 * eint_idx) &= ~(0x1ul << port_idx);
	}

	if (type & EINT_ONRISE)
	{
		// INTEN
		*(uint32_t *)(GPIOA_BASE + 0x90 + 0x4 * eint_idx) |= 0x10000ul << port_idx;
	}
	else
	{
		// INTEN
		*(uint32_t *)(GPIOA_BASE + 0x90 + 0x4 * eint_idx) &= ~(0x10000ul << port_idx);
	}

	eint_info[i].allocated = 1;
	eint_info[i].enable = 0;
	eint_info[i].eint_idx = eint_idx;
	eint_info[i].port_idx = port_idx;
	eint_info[i].callback = callback;
	eint_info[i].param = param;

	return VSFERR_NONE;
}

static int8_t eint_find(uint8_t eint_idx, uint8_t port_idx)
{
	uint8_t i;
	for (i =0; i < EINT_NUM; i++)
	{
		if ((eint_info[i].allocated == 1) &&
			(eint_info[i].eint_idx == eint_idx) &&
			(eint_info[i].port_idx == port_idx))
			return i;
	}
	return -1;
}

vsf_err_t nuc505_eint_enable(uint32_t index)
{
	int8_t i;

	i = eint_find(index & 0x0F, (index & 0xF0) >> 4);
	if (i < 0)
		return VSFERR_INVALID_PARAMETER;
	else
		eint_info[i].enable = 1;

	return VSFERR_NONE;
}

vsf_err_t nuc505_eint_disable(uint32_t index)
{
	int8_t i;

	i = eint_find(index & 0x0F, (index & 0xF0) >> 4);
	if (i < 0)
		return VSFERR_INVALID_PARAMETER;
	else
		eint_info[i].enable = 0;

	return VSFERR_NONE;
}

ROOTFUNC void EINT0_IRQHandler(void)
{
	uint8_t i;
	int8_t j;
	uint32_t intsts = *(uint32_t *)(GPIOA_BASE + 0xb4) & 0xffff;

	*(uint32_t *)(GPIOA_BASE + 0xb4) |= intsts;

	for (i = 0; i < 16; i++)
	{
		if (intsts & (0x1ul << i))
		{
			j = eint_find(0, i);
			if ((j >= 0) && eint_info[j].enable)
			{
				if (eint_info[j].callback)
					eint_info[j].callback(eint_info[j].param);
			}
		}
	}
}
ROOTFUNC void EINT1_IRQHandler(void)
{
	uint8_t i;
	int8_t j;
	uint32_t intsts = *(uint32_t *)(GPIOA_BASE + 0xb4) & 0xffff0000;

	*(uint32_t *)(GPIOA_BASE + 0xb4) |= intsts;

	for (i = 0; i < 16; i++)
	{
		if (intsts & (0x10000ul << i))
		{
			j = eint_find(1, i);
			if ((j >= 0) && eint_info[j].enable)
			{
				if (eint_info[j].callback)
					eint_info[j].callback(eint_info[j].param);
			}
		}
	}
}
ROOTFUNC void EINT2_IRQHandler(void)
{
	int8_t i, j;
	uint32_t intsts = *(uint32_t *)(GPIOA_BASE + 0xb8) & 0xffff;

	*(uint32_t *)(GPIOA_BASE + 0xb8) |= intsts;

	for (i = 0; i < 16; i++)
	{
		if (intsts & (0x1ul << i))
		{
			j = eint_find(2, i);
			if ((j >= 0) && eint_info[j].enable)
			{
				if (eint_info[j].callback)
					eint_info[j].callback(eint_info[j].param);
			}
		}
	}
}
ROOTFUNC void EINT3_IRQHandler(void)
{
	uint8_t i;
	int8_t j;
	uint32_t intsts = *(uint32_t *)(GPIOA_BASE + 0xb8) & 0xffff0000;

	*(uint32_t *)(GPIOA_BASE + 0xb8) |= intsts;

	for (i = 0; i < 16; i++)
	{
		if (intsts & (0x10000ul << i))
		{
			j = eint_find(3, i);
			if ((j >= 0) && eint_info[j].enable)
			{
				if (eint_info[j].callback)
					eint_info[j].callback(eint_info[j].param);
			}
		}
	}
}

#endif

