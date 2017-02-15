/**************************************************************************//**
 * @file        spiflash_drv.h
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       SPIM SPI Flash driver
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
 
#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __SPIFLASH_DRV_H__
#define __SPIFLASH_DRV_H__


#define OPCODE_WREN             0x06    /* Write enable */
#define OPCODE_RDSR             0x05    /* Read status register #1*/
#define OPCODE_WRSR             0x01    /* Write status register #1 */
#define OPCODE_RDSR2            0x35    /* Read status register #2*/
#define OPCODE_WRSR2            0x31    /* Write status register #2 */
#define OPCODE_RDSR3            0x15    /* Read status register #3*/
#define OPCODE_WRSR3            0x11    /* Write status register #3 */
#define OPCODE_NORM_READ        0x03    /* Read data bytes (low frequency) */
#define OPCODE_FAST_READ        0x0b    /* Read data bytes (high frequency) */
#define OPCODE_PP               0x02    /* Page program (up to 256 bytes) */
#define OPCODE_BE_4K            0x20    /* Erase 4KiB block */
#define OPCODE_BE_32K           0x52    /* Erase 32KiB block */
#define OPCODE_CHIP_ERASE       0xc7    /* Erase whole flash chip */
#define OPCODE_SE               0xd8    /* Sector erase (usually 64KiB) */
#define OPCODE_RDID             0x9f    /* Read JEDEC ID */

#define OPCODE_FAST_READ_DUAL_OUT           0x3b    /* Fast Read Dual Output */
#define OPCODE_FAST_READ_QUAD_OUT           0x6b    /* Fast Read Quad Output */
#define OPCODE_FAST_READ_DUAL_IO            0xbb    /* Fast Read Dual I/O */
#define OPCODE_FAST_READ_QUAD_IO            0xeb    /* Fast Read Quad I/O */
#define OPCODE_QUAD_PP_WINBOND              0x32    /* Quad page program (up to 256 bytes) */
#define OPCODE_QUAD_PP_MXIC                 0x38    /* Quad page program (up to 256 bytes) */

#define OPCODE_RSTEN            0x66
#define OPCODE_RST              0x99

#define OPCODE_ENQPI            0x38
#define OPCODE_EXQPI            0xFF

/* Status Register bits. */
#define SR_WIP                  1       /* Write in progress */
#define SR_WEL                  2       /* Write enable latch */
#define SR_QE                   0x40    /* Quad Enable for MXIC */
/* Status Register #2 bits. */
#define SR2_QE                  2       /* Quad Enable for Winbond */

#define MFGID_WINBOND           0xEF
#define MFGID_MXIC              0xC2
#define MFGID_EON               0x1C

void SPIMUtil_BulkWriteTx(SPIM_T *spim, const uint8_t *pu8TxBuf, uint32_t u32Len);
void SPIMUtil_BulkReadRx(SPIM_T *spim, uint8_t *pu8RxBuf, uint32_t u32Len);
void SPIFlash_Init(void);
uint32_t SPIFlash_ReadJedecID(void);
uint8_t SPIFlash_ReadStatusRegister(void);
void SPIFlash_WriteStatusRegister(uint8_t u8Status);
uint8_t SPIFlash_W25Q_ReadStatusRegister2(void);
void SPIFlash_W25Q_WriteStatusRegister2(uint8_t u8Status);
void SPIFlash_EnableWrite(void);
void SPIFlash_WaitWriteDone(void);
void SPIFlash_W25Q_SetQuadEnable(int isEn);
void SPIFlash_MX25_SetQuadEnable(int isEn);
void SPIFlash_EN25Q_SetQPIMode(int isEn);
void SPIFlash_EraseBlock(uint32_t u32Addr);
void SPIFlash_EraseAddrRange(uint32_t u32Addr, uint32_t u32Len);
void SPIFlash_WriteInPageData(uint32_t u32Addr, uint32_t u32NTx, uint8_t *pu8TxBuf);
void SPIFlash_WriteData(uint32_t u32Addr, uint32_t u32NTx, uint8_t *pu8TxBuf);
void SPIFlash_ReadData(uint32_t u32Addr, uint32_t u32NRx, uint8_t *pu8RxBuf);
void SPIFlash_DMAWrite(SPIM_T *spim, uint32_t u32FlashAddr, int is4ByteAddr, uint32_t u32Len, uint8_t *pu8TxBuf, 
    uint32_t u32WriteCmdCode);
void SPIFlash_DMARead(SPIM_T *spim, uint32_t u32FlashAddr, int is4ByteAddr, uint32_t u32Len, uint8_t *pu8RxBuf, 
    uint32_t u32ReadCmdCode);
    
#endif /* #ifndef __SPIFLASH_DRV_H__ */

#ifdef __cplusplus
}
#endif
