/**************************************************************************//**
 * @file        spiflash_drv.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       SPIM SPI Flash driver
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include "NUC505Series.h"
#include "spiflash_drv.h"
#include "spim.h"

/**
  * @brief      Helper function for bulk write through SPI bus.
  */
void SPIMUtil_BulkWriteTx(SPIM_T *spim, const uint8_t *pu8TxBuf, uint32_t u32Len)
{
    while (u32Len) {
        unsigned int dataNum = 0, dataNum2;

        if (u32Len >= 16) {
            dataNum = 4;
        }
        else if (u32Len >= 12) {
            dataNum = 3;
        }
        else if (u32Len >= 8) {
            dataNum = 2;
        }
        else if (u32Len >= 4) {
            dataNum = 1;
        }

        dataNum2 = dataNum;
        while (dataNum2) {
            uint32_t tmp;

#if 0
            tmp = *pu8TxBuf ++;
            tmp <<= 8;
            tmp += *pu8TxBuf ++;
            tmp <<= 8;
            tmp += *pu8TxBuf ++;
            tmp <<= 8;
            tmp += *pu8TxBuf ++;
#else
            tmp = *pu8TxBuf;
            pu8TxBuf ++;
            tmp += *pu8TxBuf << 8;
            pu8TxBuf ++;
            tmp += *pu8TxBuf << 16;
            pu8TxBuf ++;
            tmp += *pu8TxBuf << 24;
            pu8TxBuf ++;

#endif
            u32Len -= 4;

            dataNum2 --;
            *((__O uint32_t *) &spim->TX0 + dataNum2) = tmp;
        }

        if (dataNum) {
            SPIM_ENABLE_IO_MODE(spim, SPIM_CTL0_BITMODE_STAN, 1);   // Switch to I/O mode, 1-bit output.
            SPIM_SET_DATA_WIDTH(spim, 32);
            SPIM_SET_BURST_NUM(spim, dataNum);
            SPIM_TRIGGER(spim);
            while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.
        }

        if (u32Len && u32Len < 4) {
            int rmn = u32Len;
            uint32_t tmp;

#if 0
            tmp = *pu8TxBuf ++;
            u32Len --;
            if (u32Len) {
                tmp <<= 8;
                tmp += *pu8TxBuf ++;
                u32Len --;
            }
            if (u32Len) {
                tmp <<= 8;
                tmp += *pu8TxBuf ++;
                u32Len --;
            }
#else
            tmp = *pu8TxBuf;
            pu8TxBuf ++;
            u32Len --;
            if (u32Len) {
                tmp += *pu8TxBuf << 8;
                pu8TxBuf ++;
                u32Len --;
            }
            if (u32Len) {
                tmp += *pu8TxBuf << 16;
                pu8TxBuf ++;
                u32Len --;
            }
#endif
            spim->TX0 = tmp;

            SPIM_ENABLE_IO_MODE(spim, SPIM_CTL0_BITMODE_STAN, 1);   // Switch to I/O mode, 1-bit output.
            SPIM_SET_DATA_WIDTH(spim, rmn * 8);
            SPIM_SET_BURST_NUM(spim, 1);
            SPIM_TRIGGER(spim);
            while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.
        }
    }
}

/**
  * @brief      Helper function for bulk read through SPI bus.
  */
void SPIMUtil_BulkReadRx(SPIM_T *spim, uint8_t *pu8RxBuf, uint32_t u32Len)
{
    while (u32Len) {
        unsigned int dataNum = 0;

        if (u32Len >= 16) {
            dataNum = 4;
        }
        else if (u32Len >= 12) {
            dataNum = 3;
        }
        else if (u32Len >= 8) {
            dataNum = 2;
        }
        else if (u32Len >= 4) {
            dataNum = 1;
        }
        if (dataNum) {
            SPIM_ENABLE_IO_MODE(spim, SPIM_CTL0_BITMODE_STAN, 0);   // Switch to I/O mode, 1-bit input.
            SPIM_SET_DATA_WIDTH(spim, 32);
            SPIM_SET_BURST_NUM(spim, dataNum);
            SPIM_TRIGGER(spim);
            while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.
        }

        while (dataNum) {
            uint32_t tmp;

            tmp = *((__I uint32_t *) &spim->RX0 + dataNum - 1);
#if 0
            *pu8RxBuf ++ = (uint8_t) (tmp >> 24);
            *pu8RxBuf ++ = (uint8_t) (tmp >> 16);
            *pu8RxBuf ++ = (uint8_t) (tmp >> 8);
            *pu8RxBuf ++ = (uint8_t) tmp;
#else
            *pu8RxBuf ++ = (uint8_t) tmp;
            *pu8RxBuf ++ = (uint8_t) (tmp >> 8);
            *pu8RxBuf ++ = (uint8_t) (tmp >> 16);
            *pu8RxBuf ++ = (uint8_t) (tmp >> 24);
#endif
            dataNum --;
            u32Len -= 4;
        }

        if (u32Len && u32Len < 4) {
            uint32_t tmp;

            SPIM_ENABLE_IO_MODE(spim, SPIM_CTL0_BITMODE_STAN, 0);   // Switch to I/O mode, 1-bit input.
            SPIM_SET_DATA_WIDTH(spim, u32Len * 8);
            SPIM_SET_BURST_NUM(spim, 1);
            SPIM_TRIGGER(spim);
            while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.

            tmp = spim->RX0;
#if 0
            if (u32Len == 3) {
                *pu8RxBuf ++ = (uint8_t) (tmp >> 16);
                u32Len --;
            }
            if (u32Len == 2) {
                *pu8RxBuf ++ = (uint8_t) (tmp >> 8);
                u32Len --;
            }
            if (u32Len == 1) {
                *pu8RxBuf ++ = (uint8_t) tmp;
                u32Len --;
            }
#else
            if (u32Len == 3) {
                *pu8RxBuf ++ = (uint8_t) tmp;
                u32Len --;
            }
            if (u32Len == 2) {
                *pu8RxBuf ++ = (uint8_t) (tmp >> 8);
                u32Len --;
            }
            if (u32Len == 1) {
                *pu8RxBuf ++ = (uint8_t) (tmp >> 16);
                u32Len --;
            }
#endif
        }
    }
}

/**
  * @brief      Initialize SPI Flash device.
  * @note       Not all SPI Flash devices support this command.
  */
void SPIFlash_Init(void)
{
    /* Not knowing in SPI or QPI mode, do QPI reset and then SPI reset. */
    /* QPI Reset Enable */
    SPIM_SET_SS_LOW(SPIM);                                  // SS activated.
    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_QUAD, 1);   // I/O mode, 4-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_RSTEN);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.
    SPIM_SET_SS_HIGH(SPIM);                                 // SS deactivated.

    /* QPI Reset */
    SPIM_SET_SS_LOW(SPIM);                                  // SS activated.
    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_QUAD, 1);   // I/O mode, 4-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_RST);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.
    SPIM_SET_SS_HIGH(SPIM);                                 // SS deactivated.

    /* SPI Reset Enable */
    SPIM_SET_SS_LOW(SPIM);                                  // SS activated.
    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);   // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_RSTEN);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.
    SPIM_SET_SS_HIGH(SPIM);                                 // SS deactivated.

    /* SPI Reset */
    SPIM_SET_SS_LOW(SPIM);                                  // SS activated.
    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);   // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_RST);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                             // Wait for ready.
    SPIM_SET_SS_HIGH(SPIM);                                 // SS deactivated.
}

/**
  * @brief      Read JEDEC ID.
  * @note       Support by most SPI Flash devices.
  */
uint32_t SPIFlash_ReadJedecID(void)
{
    uint32_t u32JedecID;

    SPIM_SET_SS_LOW(SPIM);                                          // SS activated.

    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);           // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_RDID);
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

    return u32JedecID;
}

/**
  * @brief      Read Status Register-1.
  * @note       Support by most SPI Flash devices.
  */
uint8_t SPIFlash_ReadStatusRegister(void)
{
    uint8_t u8Status;

    SPIM_SET_SS_LOW(SPIM);                                          // SS activated.

    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);           // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_RDSR);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.

    SPIM_SET_QDIODIR(SPIM, 0);                                      // Change I/O direction to input.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.
    u8Status = SPIM_READ_RX0(SPIM);

    SPIM_SET_SS_HIGH(SPIM);                                         // SS deactivated.

    return u8Status;
}

/**
  * @brief      Write Status Register-1 of SPI Flash device.
  * @note       Support by most SPI Flash devices.
  */
void SPIFlash_WriteStatusRegister(uint8_t u8Status)
{
    SPIM_SET_SS_LOW(SPIM);                                      // SS activated.

    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);       // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_WRSR);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                                 // Wait for ready.

    SPIM_SET_SS_HIGH(SPIM);                                     // SS deactivated.
}

/**
  * @brief      Read Status Register-2.
  * @note       Support by Winbond W25Q series SPI Flash devices.
  */
uint8_t SPIFlash_W25Q_ReadStatusRegister2(void)
{
    uint8_t u8Status;

    SPIM_SET_SS_LOW(SPIM);                                      // SS activated.

    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);       // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_RDSR2);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                                 // Wait for ready.

    SPIM_SET_QDIODIR(SPIM, 0);                                  // Change I/O direction to input.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                                 // Wait for ready.
    u8Status = SPIM_READ_RX0(SPIM);

    SPIM_SET_SS_HIGH(SPIM);                                     // SS deactivated.

    return u8Status;
}

/**
  * @brief      Write Status Register-2.
  * @note       Support by Winbond W25Q series SPI Flash devices.
  */
void SPIFlash_W25Q_WriteStatusRegister2(uint8_t u8Status)
{
    SPIM_SET_SS_LOW(SPIM);                                      // SS activated.

    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);       // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_WRSR2);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                                 // Wait for ready.

    SPIM_SET_SS_HIGH(SPIM);                                     // SS deactivated.
}

/**
  * @brief      Set Write Enable Latch (WEL) bit of Status Register-1 to 1.
  * @note       Support by most SPI Flash devices.
  */
void SPIFlash_EnableWrite(void)
{
    SPIM_SET_SS_LOW(SPIM);                                      // SS activated.

    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);       // I/O mode, 1-bit, output.
    SPIM_SET_DATA_WIDTH(SPIM, 8);
    SPIM_SET_BURST_NUM(SPIM, 1);
    SPIM_WRITE_TX0(SPIM, OPCODE_WREN);
    SPIM_TRIGGER(SPIM);
    while (SPIM_IS_BUSY(SPIM));                                 // Wait for ready.

    SPIM_SET_SS_HIGH(SPIM);                                     // SS deactivated.
}

/**
  * @brief      Wait until Write/Erase In Progress (BUSY) bit becomes 0.
  */
void SPIFlash_WaitWriteDone(void)
{
    while (SPIFlash_ReadStatusRegister() & SR_WIP);
}

/**
  * @brief      Set Quad Enable (QE) bit of Status Register-2 to 1.
  * @note       Support by Winbond W25Q series SPI Flash devices.
  */
void SPIFlash_W25Q_SetQuadEnable(int isEn)
{
    uint8_t u8Status = SPIFlash_W25Q_ReadStatusRegister2();
    if (isEn) {
        u8Status |= SR2_QE;
    }
    else {
        u8Status &= ~SR2_QE;
    }
    SPIFlash_EnableWrite();
    SPIFlash_W25Q_WriteStatusRegister2(u8Status);
    SPIFlash_WaitWriteDone();
}

/**
  * @brief      Set Quad Enable (QE) bit of Status Register-1 to 1.
  * @note       Support by MXIC MX25 series SPI Flash devices.
  */
void SPIFlash_MX25_SetQuadEnable(int isEn)
{
    uint8_t u8Status = SPIFlash_ReadStatusRegister();
    if (isEn) {
        u8Status |= SR_QE;
    }
    else {
        u8Status &= ~SR_QE;
    }
    SPIFlash_EnableWrite();
    SPIFlash_WriteStatusRegister(u8Status);
    SPIFlash_WaitWriteDone();
}

/**
  * @brief      Enable/disable QPI mode.
  * @note       Support by EON EN25Q series SPI Flash devices.
  */
void SPIFlash_EN25Q_SetQPIMode(int isEn)
{
    if (isEn) {                                                         // Assume in SPI mode.
        SPIM_SET_SS_LOW(SPIM);                                          // SS activated.

        SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);           // I/O mode, 1-bit, output.
        SPIM_SET_DATA_WIDTH(SPIM, 8);
        SPIM_SET_BURST_NUM(SPIM, 1);
        SPIM_WRITE_TX0(SPIM, OPCODE_ENQPI);
        SPIM_TRIGGER(SPIM);
        while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.

        SPIM_SET_SS_HIGH(SPIM);                                         // SS deactivated.
    }
    else {                                                              // Assume in QPI mode.
        SPIM_SET_SS_LOW(SPIM);                                          // SS activated.

        SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_QUAD, 1);           // I/O mode, 4-bit, output.
        SPIM_SET_DATA_WIDTH(SPIM, 8);
        SPIM_SET_BURST_NUM(SPIM, 1);
        SPIM_WRITE_TX0(SPIM, OPCODE_EXQPI);
        SPIM_TRIGGER(SPIM);
        while (SPIM_IS_BUSY(SPIM));                                     // Wait for ready.

        SPIM_SET_SS_HIGH(SPIM);                                         // SS deactivated.
    }
}

/**
  * @brief      Erase the block where the specified address is located.
  * @note       Block size is 64KB.
  * @note       Support by most SPI Flash devices.
  */
void SPIFlash_EraseBlock(uint32_t u32Addr)
{
    uint8_t au8CmdBuf[16];
    uint8_t *pu8CmdBufInd = (uint8_t *) au8CmdBuf;

    SPIFlash_EnableWrite();

    *pu8CmdBufInd ++ = OPCODE_SE;                   // Erase block command.
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 16);   // 3-byte address.
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 8);
    *pu8CmdBufInd ++ = (uint8_t) u32Addr;

    SPIM_SET_SS_LOW(SPIM);                          // SS activated.
    SPIMUtil_BulkWriteTx(SPIM, au8CmdBuf, pu8CmdBufInd - (uint8_t *) au8CmdBuf);
    SPIM_SET_SS_HIGH(SPIM);                         // SS deactivated.

    SPIFlash_WaitWriteDone();
}
void SPIFlash_Erase32k(uint32_t u32Addr)
{
    uint8_t au8CmdBuf[16];
    uint8_t *pu8CmdBufInd = (uint8_t *) au8CmdBuf;

    SPIFlash_EnableWrite();

    *pu8CmdBufInd ++ = OPCODE_BE_32K;                   // Erase block command.
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 16);   // 3-byte address.
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 8);
    *pu8CmdBufInd ++ = (uint8_t) u32Addr;

    SPIM_SET_SS_LOW(SPIM);                          // SS activated.
    SPIMUtil_BulkWriteTx(SPIM, au8CmdBuf, pu8CmdBufInd - (uint8_t *) au8CmdBuf);
    SPIM_SET_SS_HIGH(SPIM);                         // SS deactivated.

    SPIFlash_WaitWriteDone();
}
void SPIFlash_Erase4k(uint32_t u32Addr)
{
    uint8_t au8CmdBuf[16];
    uint8_t *pu8CmdBufInd = (uint8_t *) au8CmdBuf;

    SPIFlash_EnableWrite();

    *pu8CmdBufInd ++ = OPCODE_BE_4K;                   // Erase block command.
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 16);   // 3-byte address.
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 8);
    *pu8CmdBufInd ++ = (uint8_t) u32Addr;

    SPIM_SET_SS_LOW(SPIM);                          // SS activated.
    SPIMUtil_BulkWriteTx(SPIM, au8CmdBuf, pu8CmdBufInd - (uint8_t *) au8CmdBuf);
    SPIM_SET_SS_HIGH(SPIM);                         // SS deactivated.

    SPIFlash_WaitWriteDone();
}


/**
  * @brief      Erase the blocks where the specified address range is located.
  * @note       Block size is 64KB.
  * @note       Support by most SPI Flash devices.
  */
void SPIFlash_EraseAddrRange(uint32_t u32Addr, uint32_t u32Len)
{
	unsigned long end = u32Addr + u32Len;

	while (u32Addr < end)
	{
		if (((u32Addr & 0xffff) == 0) && ((end - u32Addr) >= 64 * 1024))
		{
			SPIFlash_EraseBlock(u32Addr);
			u32Addr += 64 * 1024;
		}
		//else if (((u32Addr & 0x7fff) == 0) && ((end - u32Addr) >= 32 * 1024))
		//{
		//	SPIFlash_Erase32k(u32Addr);
		//	u32Addr += 32 * 1024;
		//}
		else if (((u32Addr & 0xfff) == 0) && ((end - u32Addr) >= 4 * 1024))
		{
			SPIFlash_Erase4k(u32Addr);
			u32Addr += 4 * 1024;
		}
		else
		{
			// error
			return;
		}
	}
}

/**
  * @brief      Program the page where the specified address is located.
  * @note       Page size is 256B.
  * @note       Support by most SPI Flash devices.
  */
void SPIFlash_WriteInPageData(uint32_t u32Addr, uint32_t u32NTx, uint8_t *pu8TxBuf)
{
    uint8_t au8CmdBuf[16];
    uint8_t *pu8CmdBufOrig = (uint8_t *) au8CmdBuf;
    uint8_t *pu8CmdBufInd = (uint8_t *) au8CmdBuf;

    SPIFlash_EnableWrite();

    SPIM_SET_SS_LOW(SPIM);                                              // SS activated.

    *pu8CmdBufInd ++ = OPCODE_PP;
    SPIMUtil_BulkWriteTx(SPIM, pu8CmdBufOrig, pu8CmdBufInd - pu8CmdBufOrig);   // Write out command.
    pu8CmdBufOrig = pu8CmdBufInd;

    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 16);
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 8);
    *pu8CmdBufInd ++ = (uint8_t) u32Addr;

    SPIMUtil_BulkWriteTx(SPIM, pu8CmdBufOrig, pu8CmdBufInd - pu8CmdBufOrig);   // Write out address.
    pu8CmdBufOrig = pu8CmdBufInd;

    SPIMUtil_BulkWriteTx(SPIM, pu8TxBuf, u32NTx);                              // Write out data.

    SPIM_SET_SS_HIGH(SPIM);                                             // SS deactivated.

    SPIFlash_WaitWriteDone();
}

/**
  * @brief      Program the pages where the specified address range is located.
  * @note       Use Page Program command supported by most SPI Flash devices.
  */
void SPIFlash_WriteData(uint32_t u32Addr, uint32_t u32NTx, uint8_t *pu8TxBuf)
{
    uint32_t u32PageOffset = u32Addr % 256;

    if ((u32PageOffset + u32NTx) <= 256) {                          // Do all the bytes fit onto one page ?
        SPIFlash_WriteInPageData(u32Addr, u32NTx, pu8TxBuf);
    }
    else {
        uint32_t u32ToWr = 256 - u32PageOffset;                     // Size of data remaining on the first page.

        SPIFlash_WriteInPageData(u32Addr, u32ToWr, pu8TxBuf);
        u32Addr += u32ToWr;                                         // Advance indicator.
        u32NTx -= u32ToWr;
        pu8TxBuf += u32ToWr;

        while (u32NTx) {
            u32ToWr = 256;
            if (u32ToWr > u32NTx) {
                u32ToWr = u32NTx;
            }

            SPIFlash_WriteInPageData(u32Addr, u32ToWr, pu8TxBuf);
            u32Addr += u32ToWr;                                     // Advance indicator.
            u32NTx -= u32ToWr;
            pu8TxBuf += u32ToWr;
        }
    }
}

/**
  * @brief      Read SPI Flash data starting from the specified address.
  * @note       Use Fast Read command supported by most SPI Flash devices.
  */
void SPIFlash_ReadData(uint32_t u32Addr, uint32_t u32NRx, uint8_t *pu8RxBuf)
{
    uint8_t au8CmdBuf[16];
    uint8_t *pu8CmdBufInd = (uint8_t *) au8CmdBuf;
    uint8_t *pu8CmdBufOrig = (uint8_t *) au8CmdBuf;

    SPIM_SET_SS_LOW(SPIM);                                              // SS activated.

    *pu8CmdBufInd ++ = OPCODE_FAST_READ;
    SPIMUtil_BulkWriteTx(SPIM, pu8CmdBufOrig, pu8CmdBufInd - pu8CmdBufOrig);   // Write out command.
    pu8CmdBufOrig = pu8CmdBufInd;

    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 16);
    *pu8CmdBufInd ++ = (uint8_t) (u32Addr >> 8);
    *pu8CmdBufInd ++ = (uint8_t) u32Addr;
    SPIMUtil_BulkWriteTx(SPIM, pu8CmdBufOrig, pu8CmdBufInd - pu8CmdBufOrig);   // Write out address.
    pu8CmdBufOrig = pu8CmdBufInd;

    *pu8CmdBufInd ++ = 0x00;
    SPIMUtil_BulkWriteTx(SPIM, pu8CmdBufOrig, pu8CmdBufInd - pu8CmdBufOrig);   // Write out dummy bytes.
    pu8CmdBufOrig = pu8CmdBufInd;

    SPIMUtil_BulkReadRx(SPIM, pu8RxBuf, u32NRx);                              // Read back data.

    SPIM_SET_SS_HIGH(SPIM);                                             // SS deactivated.
}

static void SPIM_DMAWritePage(SPIM_T *spim, uint32_t u32FlashAddr, uint32_t u32Len, uint8_t *pu8TxBuf)
{
    spim->SRAMADDR = (uint32_t) pu8TxBuf;               // SRAM address.
    spim->DMATBCNT = u32Len;                            // Transfer length.
    spim->FADDR = u32FlashAddr;                         // Flash address.
    SPIM_TRIGGER(spim);                                 // Go.

    /* User must call SPIM_IS_BUSY to check finish or not. */
}

void SPIM_DMAReadFlash(SPIM_T *spim, uint32_t u32FlashAddr, uint32_t u32Len, uint8_t *pu8RxBuf)
{
    spim->SRAMADDR = (uint32_t) pu8RxBuf;               // SRAM address.
    spim->DMATBCNT = u32Len;                            // Transfer length.
    spim->FADDR = u32FlashAddr;                         // Flash address.
    SPIM_TRIGGER(spim);                                 // Go.

    /* User must call SPIM_IS_BUSY to check finish or not. */
}

/**
  * @brief      Program the pages where the specified address range is located through SPIM DMA Write mode.
  * @note       Which SPI Flash devices are supported depends on the specified Page Program command.
  */
void SPIFlash_DMAWrite(SPIM_T *spim, uint32_t u32FlashAddr, int is4ByteAddr, uint32_t u32Len, uint8_t *pu8TxBuf,
    uint32_t u32WriteCmdCode)
{
    do {
        uint32_t pageOffset = u32FlashAddr % 256;

        if ((pageOffset + u32Len) <= 256) {             // Do all the bytes fit onto one page ?
            SPIM_ENABLE_DMA_MODE(spim, 1, u32WriteCmdCode, is4ByteAddr);
            SPIM_DMAWritePage(spim, u32FlashAddr, u32Len, pu8TxBuf);
            while (SPIM_IS_BUSY(spim));                 // Wait for ready.
        }
        else {
            uint32_t u32ToWrite = 256 - pageOffset;     // Size of data remaining on the first page.

            SPIM_ENABLE_DMA_MODE(spim, 1, u32WriteCmdCode, is4ByteAddr);
            SPIM_DMAWritePage(spim, u32FlashAddr, u32ToWrite, pu8TxBuf);
            while (SPIM_IS_BUSY(spim));                 // Wait for ready.

            u32FlashAddr += u32ToWrite;                                     // Advance indicator.
            u32Len -= u32ToWrite;
            pu8TxBuf += u32ToWrite;

            while (u32Len) {
                u32ToWrite = 256;
                if (u32ToWrite > u32Len) {
                    u32ToWrite = u32Len;
                }

                SPIM_ENABLE_DMA_MODE(spim, 1, u32WriteCmdCode, is4ByteAddr);
                SPIM_DMAWritePage(spim, u32FlashAddr, u32ToWrite, pu8TxBuf);
                while (SPIM_IS_BUSY(spim));                                 // Wait for ready.

                u32FlashAddr += u32ToWrite;                                 // Advance indicator.
                u32Len -= u32ToWrite;
                pu8TxBuf += u32ToWrite;
            }
        }
    }
    while (0);
}

/**
  * @brief      Read SPI Flash data starting from the specified address through SPIM DMA Read mode.
  * @note       Which SPI Flash devices are supported depends on the specified Read command.
  */
void SPIFlash_DMARead(SPIM_T *spim, uint32_t u32FlashAddr, int is4ByteAddr, uint32_t u32Len, uint8_t *pu8RxBuf,
    uint32_t u32ReadCmdCode)
{
    SPIM_ENABLE_DMA_MODE(spim, 0, u32ReadCmdCode, is4ByteAddr); // Switch to DMA Read mode.

    SPIM_DMAReadFlash(spim, u32FlashAddr, u32Len, pu8RxBuf);
    while (SPIM_IS_BUSY(spim)); // Wait for ready.
}
