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

// TODO: remove MACROs below to stm32_reg.h
#define STM32_FLASH_BASEADDR			0x08000000
#define STM32_FLASH_ADDR(addr)			(STM32_FLASH_BASEADDR + (addr))
#define STM32_FLASH_SIZE_KB				(*(uint16_t *)0x1FFFF7E0)

#define STM32_FLASH_KEYR_KEY1			(uint32_t)0x45670123
#define STM32_FLASH_KEYR_KEY2			(uint32_t)0xCDEF89AB
#define STM32_FLASH_OPTKEYR_KEY1		(uint32_t)0x45670123
#define STM32_FLASH_OPTKEYR_KEY2		(uint32_t)0xCDEF89AB

#define STM32_FLASH_CR_EOPIE			((uint32_t)1 << 12)
#define STM32_FLASH_CR_LOCK				((uint32_t)1 << 7)
#define STM32_FLASH_CR_STAT				((uint32_t)1 << 6)
#define STM32_FLASH_CR_PER				((uint32_t)1 << 1)
#define STM32_FLASH_CR_PG				((uint32_t)1 << 0)

#define STM32_FLASH_SR_EOP				((uint32_t)1 << 5)
#define STM32_FLASH_SR_WRPRTERR			((uint32_t)1 << 4)
#define STM32_FLASH_SR_PGERR			((uint32_t)1 << 2)
#define STM32_FLASH_SR_BSY				((uint32_t)1 << 0)
#define STM32_FLASH_SR_ERR				\
				(STM32_FLASH_SR_WRPRTERR | STM32_FLASH_SR_PGERR)

#if VSFHAL_FLASH_EN

#include "STM32_FLASH.h"

#define STM32_FLASH_NUM					1

struct stm32_flash_t
{
	struct
	{
		void *param;
		void (*onfinish)(void*, vsf_err_t);
	} cb;
	uint8_t bank;
} static stm32_flash[STM32_FLASH_NUM];
const bool stm32_flash_direct_read = true;

vsf_err_t stm32_flash_checkidx(uint8_t index)
{
	return (index < STM32_FLASH_NUM) ? VSFERR_NONE : VSFERR_NOT_SUPPORT;
}

vsf_err_t stm32_flash_capacity(uint8_t index, uint32_t *pagesize, 
									uint32_t *pagenum)
{
	uint16_t flash_size, page_size;

	switch (index)
	{
	case 0:
		flash_size = STM32_FLASH_SIZE_KB;
		page_size = (flash_size >= 256) ? 1 : 0;
		if (NULL != pagesize)
		{
			*pagesize = 1024 << page_size;
		}
		if (NULL != pagenum)
		{
			*pagenum = flash_size >> page_size;
		}
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

uint32_t stm32_flash_baseaddr(uint8_t index)
{
	return STM32_FLASH_BASEADDR;
}

// op -- operation: 0(ERASE), 1(READ), 2(WRITE)
uint32_t stm32_flash_blocksize(uint8_t index, uint32_t addr, uint32_t size,
								int op)
{
	uint32_t pagesize;
	if (stm32_flash_capacity(index, &pagesize, NULL))
		return 0;
	return !op ? pagesize : 2;
}

vsf_err_t stm32_flash_config_cb(uint8_t index, uint32_t int_priority,
								void *param, void (*onfinish)(void*, vsf_err_t))
{
	if (stm32_flash_checkidx(index))
		return VSFERR_NOT_SUPPORT;

	stm32_flash[index].cb.param = param;
	stm32_flash[index].cb.onfinish = onfinish;
	NVIC->IP[FLASH_IRQn] = int_priority;
	NVIC->ISER[FLASH_IRQn >> 0x05] = 1UL << (FLASH_IRQn & 0x1F);
	return VSFERR_NONE;
}

vsf_err_t stm32_flash_init(uint8_t index)
{
	uint32_t size, pagenum;
	if (stm32_flash_capacity(index, &size, &pagenum))
		return VSFERR_FAIL;
	size *= pagenum;

	FLASH->KEYR = STM32_FLASH_KEYR_KEY1;
	FLASH->KEYR = STM32_FLASH_KEYR_KEY2;
	FLASH->SR |= STM32_FLASH_SR_EOP | STM32_FLASH_SR_BSY |
					STM32_FLASH_SR_PGERR | STM32_FLASH_SR_WRPRTERR;
	if (size > 512 * 1024)
	{
		FLASH->KEYR2 = STM32_FLASH_KEYR_KEY1;
		FLASH->KEYR2 = STM32_FLASH_KEYR_KEY2;
		FLASH->SR2 |= STM32_FLASH_SR_EOP | STM32_FLASH_SR_BSY |
					STM32_FLASH_SR_PGERR | STM32_FLASH_SR_WRPRTERR;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32_flash_fini(uint8_t index)
{
	return stm32_flash_checkidx(index);
}

ROOTFUNC void FLASH_IRQHandler(void)
{
	volatile uint32_t *CR, *SR;

	if (!stm32_flash[0].bank)
	{
		CR = &FLASH->CR;
		SR = &FLASH->SR;
	}
	else
	{
		CR = &FLASH->CR2;
		SR = &FLASH->SR2;
	}
	*CR &= ~(STM32_FLASH_CR_PER | STM32_FLASH_CR_PG | STM32_FLASH_CR_EOPIE);
	if (stm32_flash[0].cb.onfinish != NULL)
	{
		stm32_flash[0].cb.onfinish(stm32_flash[0].cb.param,
					(*SR & STM32_FLASH_SR_ERR) ? VSFERR_FAIL : VSFERR_NONE);
	}
}

vsf_err_t stm32_flash_erase(uint8_t index, uint32_t addr)
{
	volatile uint32_t *CR, *AR;

	switch (index)
	{
	case 0:
		if (addr < 512 * 1024)
		{
			stm32_flash[0].bank = 0;
			CR = &FLASH->CR;
			AR = &FLASH->AR;
		}
		else
		{
			stm32_flash[0].bank = 1;
			CR = &FLASH->CR2;
			AR = &FLASH->AR2;
		}
		*CR |= STM32_FLASH_CR_PER | STM32_FLASH_CR_EOPIE;
		*AR = STM32_FLASH_ADDR(addr); 
		*CR |= STM32_FLASH_CR_STAT;
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

vsf_err_t stm32_flash_read(uint8_t index, uint32_t addr, uint8_t *buff)
{
	return VSFERR_NOT_SUPPORT;
}

vsf_err_t stm32_flash_write(uint8_t index, uint32_t addr, uint8_t *buff)
{
	volatile uint32_t *CR;

	switch (index)
	{
	case 0:
		if (addr < 512 * 1024)
		{
			stm32_flash[0].bank = 0;
			CR = &FLASH->CR;
		}
		else
		{
			stm32_flash[0].bank = 1;
			CR = &FLASH->CR2;
		}
		*CR |= STM32_FLASH_CR_PG | STM32_FLASH_CR_EOPIE;
		*(uint16_t *)STM32_FLASH_ADDR(addr) = *(uint16_t *)buff;
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

#endif	// VSFHAL_FLASH_EN
