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

vsf_err_t stm32_sdio_init(uint8_t index);
vsf_err_t stm32_sdio_config(uint8_t index, uint16_t kHz, uint8_t buswide);
vsf_err_t stm32_sdio_fini(uint8_t index);
vsf_err_t stm32_sdio_start(uint8_t index);
vsf_err_t stm32_sdio_stop(uint8_t index);
vsf_err_t stm32_sdio_send_cmd(uint8_t index, uint8_t cmd, uint32_t arg,
						uint8_t resp);
vsf_err_t stm32_sdio_send_cmd_isready(uint8_t index, uint8_t resp);
vsf_err_t stm32_sdio_get_resp(uint8_t index, uint8_t *cresp, uint32_t *resp,
						uint8_t resp_num);
vsf_err_t stm32_sdio_data_tx(uint8_t index, uint32_t to_ms,
						uint32_t size, uint32_t block_size);
vsf_err_t stm32_sdio_data_tx_isready(uint8_t index, uint32_t size,
						uint8_t *buffer);
vsf_err_t stm32_sdio_data_rx(uint8_t index, uint32_t to_ms,
						uint32_t size, uint32_t block_size);
vsf_err_t stm32_sdio_data_rx_isready(uint8_t index, uint32_t size,
						uint8_t *buffer);
