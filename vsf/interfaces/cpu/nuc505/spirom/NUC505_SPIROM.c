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

#include "NUC505Series.h"
#include "spim.h"
#include "spiflash_drv.h"

static uint8_t mfgid;

uint32_t cortexm_get_pc(void);
vsf_err_t nuc505_spirom_init(void)
{
	uint32_t vecmap_addr = SYS->RVMPADDR, vecmap_len = SYS->RVMPLEN >> 14;
	uint32_t pc = cortexm_get_pc();

	if (((pc >= 0x20000000) && (pc < 0x20020000)) ||
		((pc >= 0x1ff00000) && (pc < 0x1ff20000)) ||
		((vecmap_addr != 0) && (pc < vecmap_len)))
	{
		uint32_t u32JedecID;

		CLK->AHBCLK |= CLK_AHBCLK_SPIMCKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_SPIMRST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPIMRST_Msk;

		/* There are two interfaces for internal SPI Flash. Read JEDEC ID for check. */
		SPIM->CTL1 = (SPIM->CTL1 & ~SPIM_CTL1_IFSEL_Msk) | SPIM_CTL1_IFSEL_MCP;         // Set Slave Peripheral to MCP.
	
		/* Read JEDEC ID. */
		SPIM_SET_SS_LOW(SPIM);                                          // SS activated.
		SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);           // I/O mode, 1-bit, output.
		SPIM_SET_DATA_WIDTH(SPIM, 8);
		SPIM_SET_BURST_NUM(SPIM, 1);
		SPIM_WRITE_TX0(SPIM, 0x9f);
		SPIM_TRIGGER(SPIM);
		while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.
		SPIM_SET_QDIODIR(SPIM, 0);                                      // Change I/O direction to input.
		SPIM_SET_DATA_WIDTH(SPIM, 24);
		SPIM_SET_BURST_NUM(SPIM, 1);
		SPIM_TRIGGER(SPIM);
		while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.
		u32JedecID = SPIM_READ_RX0(SPIM);
		u32JedecID &= 0x00FFFFFF;
		SPIM_SET_SS_HIGH(SPIM);                                         // SS deactivated.

		if (u32JedecID == 0x00000000 || u32JedecID == 0x00FFFFFF)
		{
			SPIM->CTL1 = (SPIM->CTL1 & ~SPIM_CTL1_IFSEL_Msk) | SPIM_CTL1_IFSEL_MCP64;   // Set Slave Peripheral MCP64.
			/* Read JEDEC ID. */
			SPIM_SET_SS_LOW(SPIM);                                          // SS activated.
			SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);           // I/O mode, 1-bit, output.
			SPIM_SET_DATA_WIDTH(SPIM, 8);
			SPIM_SET_BURST_NUM(SPIM, 1);
			SPIM_WRITE_TX0(SPIM, 0x9f);
			SPIM_TRIGGER(SPIM);
			while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.
			SPIM_SET_QDIODIR(SPIM, 0);                                      // Change I/O direction to input.
			SPIM_SET_DATA_WIDTH(SPIM, 24);
			SPIM_SET_BURST_NUM(SPIM, 1);
			SPIM_TRIGGER(SPIM);
			while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.
			u32JedecID = SPIM_READ_RX0(SPIM);
			u32JedecID &= 0x00FFFFFF;
			SPIM_SET_SS_HIGH(SPIM);                                         // SS deactivated.
		}
	
		mfgid = u32JedecID & 0xff;
		return VSFERR_NONE;
	}
	else
		return VSFERR_NOT_READY;
}

vsf_err_t nuc505_spirom_fini(void)
{
	return VSFERR_NONE;
}

vsf_err_t nuc505_spirom_erase(uint32_t addr, uint32_t len)
{
	if (((addr + len) > 0x0200000) || (len == 0) || (addr & 0xfff) || (len & 0xfff))
		return VSFERR_INVALID_PARAMETER;

	SPIFlash_EraseAddrRange(addr, len);
	return VSFERR_NONE;
}


vsf_err_t nuc505_spirom_write(uint8_t *buf, uint32_t addr, uint32_t len)
{
	uint32_t u32QuadWriteCmdCode;

	if (((addr + len) > 0x0200000) || (len == 0) || (addr & 0xff) || (len & 0xff))
		return VSFERR_INVALID_PARAMETER;

	if (mfgid == MFGID_WINBOND)
	{
		SPIFlash_W25Q_SetQuadEnable(1);
		u32QuadWriteCmdCode = SPIM_CTL0_CMDCODE_QUAD_PAGE_PROG_TYPE1;
	}
	else if (mfgid == MFGID_MXIC)
	{
		SPIFlash_MX25_SetQuadEnable(1);
		u32QuadWriteCmdCode = SPIM_CTL0_CMDCODE_QUAD_PAGE_PROG_TYPE2;
	}
	else if (mfgid == MFGID_EON)
	{
		SPIFlash_EN25Q_SetQPIMode(1);
		u32QuadWriteCmdCode = SPIM_CTL0_CMDCODE_QUAD_PAGE_PROG_TYPE3;
	}
	else
	{
		return VSFERR_FAIL;
	}

	SPIFlash_DMAWrite(SPIM, addr, 0, len, buf, u32QuadWriteCmdCode);

	if (mfgid == MFGID_WINBOND)
	{
		SPIFlash_W25Q_SetQuadEnable(0);
	}
	else if (mfgid == MFGID_MXIC)
	{
		SPIFlash_MX25_SetQuadEnable(0);
	}
	else if (mfgid == MFGID_EON)
	{
		SPIFlash_EN25Q_SetQPIMode(0);
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_spirom_read(uint8_t *buf, uint32_t addr, uint32_t len)
{
	if (((addr + len) > 0x0200000) || (len == 0))
		return VSFERR_INVALID_PARAMETER;

	SPIFlash_DMARead(SPIM, addr, 0, len, buf, SPIM_CTL0_CMDCODE_FAST_READ_QUAD_IO);

	return VSFERR_NONE;
}





