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

#ifndef __VSFHAL_H_INCLUDED__
#define __VSFHAL_H_INCLUDED__

#include "app_type.h"
#include "vsfhal_cfg.h"
#include "vsfhal_const.h"

#define VSFHAL_DUMMY_PORT				0xFF

#define VSFHAL_CORE_SLEEP_WFI(m)		__CONNECT(m, _SLEEP_WFI)
#define VSFHAL_CORE_SLEEP_PWRDOWN(m)	__CONNECT(m, _SLEEP_PWRDOWN)

struct vsfhal_core_t
{
	vsf_err_t (*init)(void *p);
	vsf_err_t (*fini)(void *p);
	vsf_err_t (*reset)(void *p);
	uint32_t (*get_stack)(void);
	vsf_err_t (*set_stack)(uint32_t sp);
	void (*sleep)(uint32_t mode);
	vsf_err_t (*pendsv_config)(void (*on_pendsv)(void *), void *param);
	vsf_err_t (*pendsv_trigger)(void);
};

#define VSFHAL_CORE_INIT(m)				__CONNECT(m, _init)
#define VSFHAL_CORE_FINI(m)				__CONNECT(m, _fini)
#define VSFHAL_CORE_RESET(m)			__CONNECT(m, _reset)
#define VSFHAL_CORE_GET_STACK(m)		__CONNECT(m, _get_stack)
#define VSFHAL_CORE_SET_STACK(m)		__CONNECT(m, _set_stack)
#define VSFHAL_CORE_SLEEP(m)			__CONNECT(m, _sleep)
#define VSFHAL_CORE_PENDSV_CONFIG(m)	__CONNECT(m, _pendsv_config)
#define VSFHAL_CORE_PENDSV_TRIGGER(m)	__CONNECT(m, _pendsv_trigger)

vsf_err_t VSFHAL_CORE_INIT(__TARGET_CHIP__)(void *p);
vsf_err_t VSFHAL_CORE_FINI(__TARGET_CHIP__)(void *p);
vsf_err_t VSFHAL_CORE_RESET(__TARGET_CHIP__)(void *p);
uint32_t VSFHAL_CORE_GET_STACK(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_CORE_SET_STACK(__TARGET_CHIP__)(uint32_t sp);
void VSFHAL_CORE_SLEEP(__TARGET_CHIP__)(uint32_t mode);
vsf_err_t VSFHAL_CORE_PENDSV_CONFIG(__TARGET_CHIP__)(void (*on_pendsv)(void *), void *param);
vsf_err_t VSFHAL_CORE_PENDSV_TRIGGER(__TARGET_CHIP__)(void);

#ifndef VSFCFG_STANDALONE_MODULE
#define SLEEP_WFI						VSFHAL_CORE_SLEEP_WFI(__TARGET_CHIP__)
#define SLEEP_PWRDOWN					VSFHAL_CORE_SLEEP_PWRDOWN(__TARGET_CHIP__)

#define vsfhal_core_init				VSFHAL_CORE_INIT(__TARGET_CHIP__)
#define vsfhal_core_fini				VSFHAL_CORE_FINI(__TARGET_CHIP__)
#define vsfhal_core_reset				VSFHAL_CORE_RESET(__TARGET_CHIP__)
#define vsfhal_core_get_stack			VSFHAL_CORE_GET_STACK(__TARGET_CHIP__)
#define vsfhal_core_set_stack			VSFHAL_CORE_SET_STACK(__TARGET_CHIP__)
#define vsfhal_core_sleep				VSFHAL_CORE_SLEEP(__TARGET_CHIP__)
#define vsfhal_core_pendsv_config		VSFHAL_CORE_PENDSV_CONFIG(__TARGET_CHIP__)
#define vsfhal_core_pendsv_trigger		VSFHAL_CORE_PENDSV_TRIGGER(__TARGET_CHIP__)
#endif

#if VSFHAL_UNIQUEID_EN

struct vsfhal_uid_t
{
	uint32_t (*get)(uint8_t *buffer, uint32_t size);
};

#define VSFHAL_UID_GET(m)				__CONNECT(m, _uid_get)

uint32_t VSFHAL_UID_GET(__TARGET_CHIP__)(uint8_t *buffer, uint32_t size);

#ifndef VSFCFG_STANDALONE_MODULE
#define vsfhal_uid_get					VSFHAL_UID_GET(__TARGET_CHIP)
#endif

#endif

#if VSFHAL_FLASH_EN

struct vsfhal_flash_t
{
	const bool *direct_read;

	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);

	vsf_err_t (*capacity)(uint8_t index, uint32_t *pagesize, uint32_t *pagenum);
	uint32_t (*baseaddr)(uint8_t index);
	uint32_t (*blocksize)(uint8_t index, uint32_t addr, uint32_t size, int op);
	vsf_err_t (*config_cb)(uint8_t index, uint32_t int_priority, void *param, void (*onfinish)(void*, vsf_err_t));

	vsf_err_t (*erase)(uint8_t index, uint32_t addr);
	vsf_err_t (*read)(uint8_t index, uint32_t addr, uint8_t *buff);
	vsf_err_t (*write)(uint8_t index, uint32_t addr, uint8_t *buff);
};

#define VSFHAL_FLASH_DIRECT_READ(m)		__CONNECT(m, _flash_direct_read)
#define VSFHAL_FLASH_SECURITY_SET(m)	__CONNECT(m, _flash_security_set)
#define VSFHAL_FLASH_INIT(m)			__CONNECT(m, _flash_init)
#define VSFHAL_FLASH_FINI(m)			__CONNECT(m, _flash_fini)
#define VSFHAL_FLASH_CAPACITY(m)		__CONNECT(m, _flash_capacity)
#define VSFHAL_FLASH_BASEADDR(m)		__CONNECT(m, _flash_baseaddr)
#define VSFHAL_FLASH_BLOCKSIZE(m)		__CONNECT(m, _flash_blocksize)
#define VSFHAL_FLASH_CONFIG_CB(m)		__CONNECT(m, _flash_config_cb)
#define VSFHAL_FLASH_ERASE(m)			__CONNECT(m, _flash_erase)
#define VSFHAL_FLASH_READ(m)			__CONNECT(m, _flash_read)
#define VSFHAL_FLASH_WRITE(m)			__CONNECT(m, _flash_write)

extern const bool VSFHAL_FLASH_DIRECT_READ(__TARGET_CHIP__);
void VSFHAL_FLASH_SECURITY_SET(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_FLASH_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_FLASH_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_FLASH_CAPACITY(__TARGET_CHIP__)(uint8_t index, uint32_t *pagesize, uint32_t *pagenum);
uint32_t VSFHAL_FLASH_BASEADDR(__TARGET_CHIP__)(uint8_t index);
uint32_t VSFHAL_FLASH_BLOCKSIZE(__TARGET_CHIP__)(uint8_t index, uint32_t addr, uint32_t size, int op);
vsf_err_t VSFHAL_FLASH_CONFIG_CB(__TARGET_CHIP__)(uint8_t index, int32_t int_priority, void *param, void (*onfinish)(void*, vsf_err_t));
vsf_err_t VSFHAL_FLASH_ERASE(__TARGET_CHIP__)(uint8_t index, uint32_t addr);
vsf_err_t VSFHAL_FLASH_READ(__TARGET_CHIP__)(uint8_t index, uint32_t addr, uint8_t *buff);
vsf_err_t VSFHAL_FLASH_WRITE(__TARGET_CHIP__)(uint8_t index, uint32_t addr, uint8_t *buff);

#ifndef VSFCFG_STANDALONE_MODULE
#define vsfhal_flash_direct_read		VSFHAL_FLASH_DIRECT_READ(__TARGET_CHIP__)
#define vsfhal_flash_security_set		VSFHAL_FLASH_SECURITY_SET(__TARGET_CHIP__)
#define vsfhal_flash_init				VSFHAL_FLASH_INIT(__TARGET_CHIP__)
#define vsfhal_flash_fini				VSFHAL_FLASH_FINI(__TARGET_CHIP__)
#define vsfhal_flash_capacity			VSFHAL_FLASH_CAPACITY(__TARGET_CHIP__)
#define vsfhal_flash_baseaddr			VSFHAL_FLASH_BASEADDR(__TARGET_CHIP__)
#define vsfhal_flash_blocksize			VSFHAL_FLASH_BLOCKSIZE(__TARGET_CHIP__)
#define vsfhal_flash_config_cb			VSFHAL_FLASH_CONFIG_CB(__TARGET_CHIP__)
#define vsfhal_flash_erase				VSFHAL_FLASH_ERASE(__TARGET_CHIP__)
#define vsfhal_flash_read				VSFHAL_FLASH_READ(__TARGET_CHIP__)
#define vsfhal_flash_write				VSFHAL_FLASH_WRITE(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_CLKO_EN

struct vsfhal_clko_t
{
	vsf_err_t (*init)(uint32_t index);
	vsf_err_t (*fini)(uint32_t index);
	vsf_err_t (*config)(uint32_t index, uint32_t kHz);
	vsf_err_t (*enable)(uint32_t index);
	vsf_err_t (*disable)(uint32_t index);
};

#define VSFHAL_CLKO_INIT(m)				__CONNECT(m, _clko_init)
#define VSFHAL_CLKO_FINI(m)				__CONNECT(m, _clko_fini)
#define VSFHAL_CLKO_CONFIG(m)			__CONNECT(m, _clko_config)
#define VSFHAL_CLKO_ENABLE(m)			__CONNECT(m, _clko_enable)
#define VSFHAL_CLKO_DISABLE(m)			__CONNECT(m, _clko_disable)

vsf_err_t VSFHAL_CLKO_INIT(__TARGET_CHIP__)(uint32_t index);
vsf_err_t VSFHAL_CLKO_FINI(__TARGET_CHIP__)(uint32_t index);
vsf_err_t VSFHAL_CLKO_CONFIG(__TARGET_CHIP__)(uint32_t index, uint32_t kHz);
vsf_err_t VSFHAL_CLKO_ENABLE(__TARGET_CHIP__)(uint32_t index);
vsf_err_t VSFHAL_CLKO_DISABLE(__TARGET_CHIP__)(uint32_t index);

#endif

#if VSFHAL_USART_EN

#define VSFHAL_USART_STOPBITS_0P5(m)	__CONNECT(m, _USART_STOPBITS_0P5)
#define VSFHAL_USART_STOPBITS_1(m)		__CONNECT(m, _USART_STOPBITS_1)
#define VSFHAL_USART_STOPBITS_1P5(m)	__CONNECT(m, _USART_STOPBITS_1P5)
#define VSFHAL_USART_STOPBITS_2(m)		__CONNECT(m, _USART_STOPBITS_2)
#define VSFHAL_USART_PARITY_NONE(m)		__CONNECT(m, _USART_PARITY_NONE)
#define VSFHAL_USART_PARITY_ODD(m)		__CONNECT(m, _USART_PARITY_ODD)
#define VSFHAL_USART_PARITY_EVEN(m)		__CONNECT(m, _USART_PARITY_EVEN)
struct vsfhal_usart_t
{
#if VSFHAL_CONST_EN
	struct
	{
		uint32_t STOPBITS_1;
		uint32_t STOPBITS_1P5;
		uint32_t STOPBITS_2;
		uint32_t PARITY_NONE;
		uint32_t PARITY_ODD;
		uint32_t PARITY_EVEN;
	} constants;
#endif
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, uint32_t baudrate, uint32_t mode);
	vsf_err_t (*config_cb)(uint8_t index, uint32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t));
	uint16_t (*tx_bytes)(uint8_t index, uint8_t *data, uint16_t size);
	uint16_t (*tx_get_free_size)(uint8_t index);
	uint16_t (*rx_bytes)(uint8_t index, uint8_t *data, uint16_t size);
	uint16_t (*rx_get_data_size)(uint8_t index);
};

#define VSFHAL_USART_INIT(m)			__CONNECT(m, _usart_init)
#define VSFHAL_USART_FINI(m)			__CONNECT(m, _usart_fini)
#define VSFHAL_USART_CONFIG(m)			__CONNECT(m, _usart_config)
#define VSFHAL_USART_CONFIG_CB(m)		__CONNECT(m, _usart_config_cb)
#define VSFHAL_USART_TX_BYTES(m)		__CONNECT(m, _usart_tx_bytes)
#define VSFHAL_USART_TX_GET_FREE_SIZE(m)__CONNECT(m, _usart_tx_get_free_size)
#define VSFHAL_USART_RX_BYTES(m)		__CONNECT(m, _usart_rx_bytes)
#define VSFHAL_USART_RX_GET_DATA_SIZE(m)__CONNECT(m, _usart_rx_get_data_size)

vsf_err_t VSFHAL_USART_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_USART_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_USART_CONFIG(__TARGET_CHIP__)(uint8_t index, uint32_t baudrate, uint32_t mode);
vsf_err_t VSFHAL_USART_CONFIG_CB(__TARGET_CHIP__)(uint8_t index, uint32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t));
uint16_t VSFHAL_USART_TX_BYTES(__TARGET_CHIP__)(uint8_t index, uint8_t *data, uint16_t size);
uint16_t VSFHAL_USART_TX_GET_FREE_SIZE(__TARGET_CHIP__)(uint8_t index);
uint16_t VSFHAL_USART_RX_BYTES(__TARGET_CHIP__)(uint8_t index, uint8_t *data, uint16_t size);
uint16_t VSFHAL_USART_RX_GET_DATA_SIZE(__TARGET_CHIP__)(uint8_t index);

#ifndef VSFCFG_STANDALONE_MODULE
#define USART_STOPBITS_1				VSFHAL_USART_STOPBITS_1(__TARGET_CHIP__)
#define USART_STOPBITS_1P5				VSFHAL_USART_STOPBITS_1P5(__TARGET_CHIP__)
#define USART_STOPBITS_2				VSFHAL_USART_STOPBITS_2(__TARGET_CHIP__)
#define USART_PARITY_NONE				VSFHAL_USART_PARITY_NONE(__TARGET_CHIP__)
#define USART_PARITY_ODD				VSFHAL_USART_PARITY_ODD(__TARGET_CHIP__)
#define USART_PARITY_EVEN				VSFHAL_USART_PARITY_EVEN(__TARGET_CHIP__)

#define vsfhal_usart_init				VSFHAL_USART_INIT(__TARGET_CHIP__)
#define vsfhal_usart_fini				VSFHAL_USART_FINI(__TARGET_CHIP__)
#define vsfhal_usart_config				VSFHAL_USART_CONFIG(__TARGET_CHIP__)
#define vsfhal_usart_config_cb			VSFHAL_USART_CONFIG_CB(__TARGET_CHIP__)
#define vsfhal_usart_tx_bytes			VSFHAL_USART_TX_BYTES(__TARGET_CHIP__)
#define vsfhal_usart_tx_get_free_size	VSFHAL_USART_TX_GET_FREE_SIZE(__TARGET_CHIP__)
#define vsfhal_usart_rx_bytes			VSFHAL_USART_RX_BYTES(__TARGET_CHIP__)
#define vsfhal_usart_rx_get_data_size	VSFHAL_USART_RX_GET_DATA_SIZE(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_SPI_EN

#define VSFHAL_SPI_MASTER(m)			__CONNECT(m, _SPI_MASTER)
#define VSFHAL_SPI_SLAVE(m)				__CONNECT(m, _SPI_SLAVE)
#define VSFHAL_SPI_MODE0(m)				__CONNECT(m, _SPI_MODE0)
#define VSFHAL_SPI_MODE1(m)				__CONNECT(m, _SPI_MODE1)
#define VSFHAL_SPI_MODE2(m)				__CONNECT(m, _SPI_MODE2)
#define VSFHAL_SPI_MODE3(m)				__CONNECT(m, _SPI_MODE3)
#define VSFHAL_SPI_MSB_FIRST(m)			__CONNECT(m, _SPI_MSB_FIRST)
#define VSFHAL_SPI_LSB_FIRST(m)			__CONNECT(m, _SPI_LSB_FIRST)
struct spi_ability_t
{
	uint32_t max_freq_hz;
	uint32_t min_freq_hz;
};
struct vsfhal_spi_t
{
#if VSFHAL_CONST_EN
	struct
	{
		uint32_t MASTER;
		uint32_t SLAVE;
		uint32_t MODE0;
		uint32_t MODE1;
		uint32_t MODE2;
		uint32_t MODE3;
		uint32_t MSB_FIRST;
		uint32_t LSB_FIRST;
	} constants;
#endif
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*get_ability)(uint8_t index, struct spi_ability_t *ability);
	vsf_err_t (*enable)(uint8_t index);
	vsf_err_t (*disable)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, uint32_t kHz, uint32_t mode);
	vsf_err_t (*config_cb)(uint8_t index, uint32_t int_priority, void *p, void (*onready)(void *));

	vsf_err_t (*select)(uint8_t index, uint8_t cs);
	vsf_err_t (*deselect)(uint8_t index, uint8_t cs);

	vsf_err_t (*start)(uint8_t index, uint8_t *out, uint8_t *in, uint32_t len);
	uint32_t (*stop)(uint8_t index);
};

#define VSFHAL_SPI_INIT(m)				__CONNECT(m, _spi_init)
#define VSFHAL_SPI_FINI(m)				__CONNECT(m, _spi_fini)
#define VSFHAL_SPI_GET_ABILITY(m)		__CONNECT(m, _spi_get_ability)
#define VSFHAL_SPI_ENABLE(m)			__CONNECT(m, _spi_enable)
#define VSFHAL_SPI_DISABLE(m)			__CONNECT(m, _spi_disable)
#define VSFHAL_SPI_CONFIG(m)			__CONNECT(m, _spi_config)
#define VSFHAL_SPI_CONFIG_CB(m)			__CONNECT(m, _spi_config_cb)
#define VSFHAL_SPI_SELECT(m)			__CONNECT(m, _spi_select)
#define VSFHAL_SPI_DESELECT(m)			__CONNECT(m, _spi_deselect)
#define VSFHAL_SPI_START(m)				__CONNECT(m, _spi_start)
#define VSFHAL_SPI_STOP(m)				__CONNECT(m, _spi_stop)

vsf_err_t VSFHAL_SPI_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_SPI_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_SPI_GET_ABILITY(__TARGET_CHIP__)(uint8_t index, struct spi_ability_t *ability);
vsf_err_t VSFHAL_SPI_ENABLE(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_SPI_DISABLE(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_SPI_CONFIG(__TARGET_CHIP__)(uint8_t index, uint32_t kHz, uint32_t mode);
vsf_err_t VSFHAL_SPI_CONFIG_CB(__TARGET_CHIP__)(uint8_t index, uint32_t int_priority, void *p, void (*onready)(void *));
vsf_err_t VSFHAL_SPI_SELECT(__TARGET_CHIP__)(uint8_t index, uint8_t cs);
vsf_err_t VSFHAL_SPI_DESELECT(__TARGET_CHIP__)(uint8_t index, uint8_t cs);
vsf_err_t VSFHAL_SPI_START(__TARGET_CHIP__)(uint8_t index, uint8_t *out, uint8_t *in, uint32_t len);
uint32_t VSFHAL_SPI_STOP(__TARGET_CHIP__)(uint8_t index);

#ifndef VSFCFG_STANDALONE_MODULE
#define SPI_MASTER						VSFHAL_SPI_MASTER(__TARGET_CHIP__)
#define SPI_SLAVE						VSFHAL_SPI_SLAVE(__TARGET_CHIP__)
#define SPI_MODE0						VSFHAL_SPI_MODE0(__TARGET_CHIP__)
#define SPI_MODE1						VSFHAL_SPI_MODE1(__TARGET_CHIP__)
#define SPI_MODE2						VSFHAL_SPI_MODE2(__TARGET_CHIP__)
#define SPI_MODE3						VSFHAL_SPI_MODE3(__TARGET_CHIP__)
#define SPI_MSB_FIRST					VSFHAL_SPI_MSB_FIRST(__TARGET_CHIP__)
#define SPI_LSB_FIRST					VSFHAL_SPI_LSB_FIRST(__TARGET_CHIP__)

#define vsfhal_spi_init					VSFHAL_SPI_INIT(__TARGET_CHIP__)
#define vsfhal_spi_fini					VSFHAL_SPI_FINI(__TARGET_CHIP__)
#define vsfhal_spi_get_ability			VSFHAL_SPI_GET_ABILITY(__TARGET_CHIP__)
#define vsfhal_spi_enable				VSFHAL_SPI_ENABLE(__TARGET_CHIP__)
#define vsfhal_spi_disable				VSFHAL_SPI_DISABLE(__TARGET_CHIP__)
#define vsfhal_spi_config				VSFHAL_SPI_CONFIG(__TARGET_CHIP__)
#define vsfhal_spi_config_cb			VSFHAL_SPI_CONFIG_CB(__TARGET_CHIP__)
#define vsfhal_spi_select				VSFHAL_SPI_SELECT(__TARGET_CHIP__)
#define vsfhal_spi_deselect				VSFHAL_SPI_DESELECT(__TARGET_CHIP__)
#define vsfhal_spi_start				VSFHAL_SPI_START(__TARGET_CHIP__)
#define vsfhal_spi_stop					VSFHAL_SPI_STOP(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_ADC_EN

#define VSFHAL_ADC_ALIGNLEFT(m)			__CONNECT(m, _ADC_ALIGNLEFT)
#define VSFHAL_ADC_ALIGNRIGHT(m)		__CONNECT(m, _ADC_ALIGNRIGHT)
#define ADC_ALIGNLEFT					VSFHAL_ADC_ALIGNLEFT(__TARGET_CHIP__)
#define ADC_ALIGNRIGHT					VSFHAL_ADC_ALIGNRIGHT(__TARGET_CHIP__)
struct vsfhal_adc_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, uint32_t clock_hz, uint32_t mode);
	vsf_err_t (*config_channel)(uint8_t index, uint8_t channel, uint8_t cycles);
	uint32_t (*get_max_value)(uint8_t index);
	vsf_err_t (*calibrate)(uint8_t index, uint8_t channel);
	vsf_err_t (*start)(uint8_t index, uint8_t channel,
				 		void (callback)(void *, uint16_t), void *param);
};

#define VSFHAL_ADC_INIT(m)				__CONNECT(m, _adc_init)
#define VSFHAL_ADC_FINI(m)				__CONNECT(m, _adc_fini)
#define VSFHAL_ADC_CONFIG(m)			__CONNECT(m, _adc_config)
#define VSFHAL_ADC_CONFIG_CHANNEL(m)	__CONNECT(m, _adc_config_channel)
#define VSFHAL_ADC_CALIBRATE(m)			__CONNECT(m, _adc_calibrate)
#define VSFHAL_ADC_GET_MAX_VALUE(m)		__CONNECT(m, _adc_get_max_value)
#define VSFHAL_ADC_START(m)				__CONNECT(m, _adc_start)

vsf_err_t VSFHAL_ADC_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_ADC_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_ADC_CONFIG(__TARGET_CHIP__)(uint8_t index, uint32_t clock_hz, uint32_t mode);
vsf_err_t VSFHAL_ADC_CONFIG_CHANNEL(__TARGET_CHIP__)(uint8_t index,  uint8_t channel, uint8_t cycles);
vsf_err_t VSFHAL_ADC_CALIBRATE(__TARGET_CHIP__)(uint8_t index, uint8_t channel);
uint32_t VSFHAL_ADC_GET_MAX_VALUE(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_ADC_START(__TARGET_CHIP__)(uint8_t index, uint8_t channel,
							void (callback)(void *, uint16_t), void *param);

#ifndef VSFCFG_STANDALONE_MODULE
#define vsfhal_adc_init					VSFHAL_ADC_INIT(__TARGET_CHIP__)
#define vsfhal_adc_fini					VSFHAL_ADC_FINI(__TARGET_CHIP__)
#define vsfhal_adc_config				VSFHAL_ADC_CONFIG(__TARGET_CHIP__)
#define vsfhal_adc_config_channel		VSFHAL_ADC_CONFIG_CHANNEL(__TARGET_CHIP__)
#define vsfhal_adc_calibrate			VSFHAL_ADC_CALIBRATE(__TARGET_CHIP__)
#define vsfhal_adc_get_max_value		VSFHAL_ADC_GET_MAX_VALUE(__TARGET_CHIP__)
#define vsfhal_adc_start				VSFHAL_ADC_START(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_GPIO_EN

#define VSFHAL_GPIO_INFLOAT(m)			__CONNECT(m, _GPIO_INFLOAT)
#define VSFHAL_GPIO_INPU(m)				__CONNECT(m, _GPIO_INPU)
#define VSFHAL_GPIO_INPD(m)				__CONNECT(m, _GPIO_INPD)
#define VSFHAL_GPIO_OUTPP(m)			__CONNECT(m, _GPIO_OUTPP)
#define VSFHAL_GPIO_OUTOD(m)			__CONNECT(m, _GPIO_OUTOD)
struct vsfhal_gpio_t
{
#if VSFHAL_CONST_EN
	struct
	{
		uint32_t INFLOAT;
		uint32_t INPU;
		uint32_t INPD;
		uint32_t OUTPP;
		uint32_t OUTOD;
	} constants;
#endif
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config_pin)(uint8_t index, uint8_t pin_idx, uint32_t mode);
	vsf_err_t (*config)(uint8_t index, uint32_t pin_mask, uint32_t io, uint32_t pull_en_mask, uint32_t input_pull_mask);
	vsf_err_t (*set)(uint8_t index, uint32_t pin_mask);
	vsf_err_t (*clear)(uint8_t index, uint32_t pin_mask);
	vsf_err_t (*out)(uint8_t index, uint32_t pin_mask, uint32_t value);
	vsf_err_t (*in)(uint8_t index, uint32_t pin_mask, uint32_t *value);
	uint32_t (*get)(uint8_t index, uint32_t pin_mask);
};

struct vsfhal_gpio_pin_t
{
	uint8_t port;
	uint8_t pin;
};

#define VSFHAL_GPIO_INIT(m)				__CONNECT(m, _gpio_init)
#define VSFHAL_GPIO_FINI(m)				__CONNECT(m, _gpio_fini)
#define VSFHAL_GPIO_CONFIG_PIN(m)		__CONNECT(m, _gpio_config_pin)
#define VSFHAL_GPIO_CONFIG(m)			__CONNECT(m, _gpio_config)
#define VSFHAL_GPIO_IN(m)				__CONNECT(m, _gpio_in)
#define VSFHAL_GPIO_OUT(m)				__CONNECT(m, _gpio_out)
#define VSFHAL_GPIO_SET(m)				__CONNECT(m, _gpio_set)
#define VSFHAL_GPIO_CLEAR(m)			__CONNECT(m, _gpio_clear)
#define VSFHAL_GPIO_GET(m)				__CONNECT(m, _gpio_get)

vsf_err_t VSFHAL_GPIO_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_GPIO_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_GPIO_CONFIG_PIN(__TARGET_CHIP__)(uint8_t index, uint8_t pin_idx, uint32_t mode);
vsf_err_t VSFHAL_GPIO_CONFIG(__TARGET_CHIP__)(uint8_t index, uint32_t pin_mask, uint32_t io, uint32_t pull_en_mask, uint32_t input_pull_mask);
vsf_err_t VSFHAL_GPIO_IN(__TARGET_CHIP__)(uint8_t index, uint32_t pin_mask, uint32_t *value);
vsf_err_t VSFHAL_GPIO_OUT(__TARGET_CHIP__)(uint8_t index, uint32_t pin_mask, uint32_t value);
vsf_err_t VSFHAL_GPIO_SET(__TARGET_CHIP__)(uint8_t index, uint32_t pin_mask);
vsf_err_t VSFHAL_GPIO_CLEAR(__TARGET_CHIP__)(uint8_t index, uint32_t pin_mask);
uint32_t VSFHAL_GPIO_GET(__TARGET_CHIP__)(uint8_t index, uint32_t pin_mask);

#ifndef VSFCFG_STANDALONE_MODULE
#define GPIO_INFLOAT					VSFHAL_GPIO_INFLOAT(__TARGET_CHIP__)
#define GPIO_INPU						VSFHAL_GPIO_INPU(__TARGET_CHIP__)
#define GPIO_INPD						VSFHAL_GPIO_INPD(__TARGET_CHIP__)
#define GPIO_OUTPP						VSFHAL_GPIO_OUTPP(__TARGET_CHIP__)
#define GPIO_OUTOD						VSFHAL_GPIO_OUTOD(__TARGET_CHIP__)

#define vsfhal_gpio_init				VSFHAL_GPIO_INIT(__TARGET_CHIP__)
#define vsfhal_gpio_fini				VSFHAL_GPIO_FINI(__TARGET_CHIP__)
#define vsfhal_gpio_config_pin			VSFHAL_GPIO_CONFIG_PIN(__TARGET_CHIP__)
#define vsfhal_gpio_config				VSFHAL_GPIO_CONFIG(__TARGET_CHIP__)
#define vsfhal_gpio_in					VSFHAL_GPIO_IN(__TARGET_CHIP__)
#define vsfhal_gpio_out					VSFHAL_GPIO_OUT(__TARGET_CHIP__)
#define vsfhal_gpio_set					VSFHAL_GPIO_SET(__TARGET_CHIP__)
#define vsfhal_gpio_clear				VSFHAL_GPIO_CLEAR(__TARGET_CHIP__)
#define vsfhal_gpio_get					VSFHAL_GPIO_GET(__TARGET_CHIP__)
#endif

#endif

struct vsfhal_tickclk_t
{
	vsf_err_t (*init)(int32_t int_priority);
	vsf_err_t (*fini)(void);
	void (*poll)(void);
	vsf_err_t (*start)(void);
	vsf_err_t (*stop)(void);
	uint32_t (*get_count)(void);
	vsf_err_t (*config_cb)(void (*)(void*), void*);
};

#define VSFHAL_TICKCLK_INIT(m)			__CONNECT(m, _tickclk_init)
#define VSFHAL_TICKCLK_FINI(m)			__CONNECT(m, _tickclk_fini)
#define VSFHAL_TICKCLK_POLL(m)			__CONNECT(m, _tickclk_poll)
#define VSFHAL_TICKCLK_START(m)			__CONNECT(m, _tickclk_start)
#define VSFHAL_TICKCLK_STOP(m)			__CONNECT(m, _tickclk_stop)
#define VSFHAL_TICKCLK_GET_COUNT(m)		__CONNECT(m, _tickclk_get_count)
#define VSFHAL_TICKCLK_CONFIG_CB(m)		__CONNECT(m, _tickclk_config_cb)

vsf_err_t VSFHAL_TICKCLK_INIT(__TARGET_CHIP__)(int32_t int_priority);
vsf_err_t VSFHAL_TICKCLK_FINI(__TARGET_CHIP__)(void);
void VSFHAL_TICKCLK_POLL(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_TICKCLK_START(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_TICKCLK_STOP(__TARGET_CHIP__)(void);
uint32_t VSFHAL_TICKCLK_GET_COUNT(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_TICKCLK_CONFIG_CB(__TARGET_CHIP__)(void (*)(void*), void*);

#ifndef VSFCFG_STANDALONE_MODULE
#define vsfhal_tickclk_init				VSFHAL_TICKCLK_INIT(__TARGET_CHIP__)
#define vsfhal_tickclk_fini				VSFHAL_TICKCLK_FINI(__TARGET_CHIP__)
#define vsfhal_tickclk_poll				VSFHAL_TICKCLK_POLL(__TARGET_CHIP__)
#define vsfhal_tickclk_start			VSFHAL_TICKCLK_START(__TARGET_CHIP__)
#define vsfhal_tickclk_stop				VSFHAL_TICKCLK_STOP(__TARGET_CHIP__)
#define vsfhal_tickclk_get_count		VSFHAL_TICKCLK_GET_COUNT(__TARGET_CHIP__)
#define vsfhal_tickclk_config_cb		VSFHAL_TICKCLK_CONFIG_CB(__TARGET_CHIP__)
#endif

#if VSFHAL_I2C_EN

#define I2C_READ						(1 << 0)
#define I2C_WRITE						(0 << 0)
#define I2C_NOSTART						(1 << 4)
#define I2C_ACKLAST						(1 << 5)

struct vsfhal_i2c_msg_t
{
	uint16_t flag;
	uint16_t len;
	uint8_t *buf;
};

struct vsfhal_i2c_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, uint16_t kHz);
	vsf_err_t (*config_cb)(uint8_t index, void *param, void (*cb)(void*, vsf_err_t));
	vsf_err_t (*xfer)(uint8_t index, uint16_t addr, struct vsfhal_i2c_msg_t *msg, uint8_t msglen);
};

#define VSFHAL_I2C_INIT(m)				__CONNECT(m, _i2c_init)
#define VSFHAL_I2C_FINI(m)				__CONNECT(m, _i2c_fini)
#define VSFHAL_I2C_CONFIG(m)			__CONNECT(m, _i2c_config)
#define VSFHAL_I2C_CONFIG_CB(m)			__CONNECT(m, _i2c_config_cb)
#define VSFHAL_I2C_XFER(m)				__CONNECT(m, _i2c_xfer)

vsf_err_t VSFHAL_I2C_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_I2C_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_I2C_CONFIG(__TARGET_CHIP__)(uint8_t index, uint16_t kHz);
vsf_err_t VSFHAL_I2C_CONFIG_CB(__TARGET_CHIP__)(uint8_t index, void *param, void (*cb)(void*, vsf_err_t));
vsf_err_t VSFHAL_I2C_XFER(__TARGET_CHIP__)(uint8_t index, uint16_t addr, struct vsfhal_i2c_msg_t *msg, uint8_t msglen);

#ifndef VSFCFG_STANDALONE_MODULE
#define vsfhal_i2c_init					VSFHAL_I2C_INIT(__TARGET_CHIP__)
#define vsfhal_i2c_fini					VSFHAL_I2C_FINI(__TARGET_CHIP__)
#define vsfhal_i2c_config				VSFHAL_I2C_CONFIG(__TARGET_CHIP__)
#define vsfhal_i2c_config_cb			VSFHAL_I2C_CONFIG_CB(__TARGET_CHIP__)
#define vsfhal_i2c_xfer					VSFHAL_I2C_XFER(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_PWM_EN

#define PWM_OUTPP						0x01
#define PWM_OUTPOLARITY					0x02

struct vsfhal_pwm_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config_mode)(uint8_t index, uint8_t mode);
	vsf_err_t (*config_freq)(uint8_t index, uint16_t kHz);
	vsf_err_t (*out)(uint8_t index, uint16_t count, uint16_t *rate);
	vsf_err_t (*in)(uint8_t index, uint16_t count, uint16_t *rate);
};

#define VSFHAL_PWM_INIT(m)				__CONNECT(m, _pwm_init)
#define VSFHAL_PWM_FINI(m)				__CONNECT(m, _pwm_fini)
#define VSFHAL_PWM_CONFIG_MODE(m)		__CONNECT(m, _pwm_config_mode)
#define VSFHAL_PWM_CONFIG_FREQ(m)		__CONNECT(m, _pwm_config_freq)
#define VSFHAL_PWM_OUT(m)				__CONNECT(m, _pwm_out)
#define VSFHAL_PWM_IN(m)				__CONNECT(m, _pwm_in)

vsf_err_t VSFHAL_PWM_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_PWM_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_PWM_CONFIG_MODE(__TARGET_CHIP__)(uint8_t index, uint8_t mode);
vsf_err_t VSFHAL_PWM_CONFIG_FREQ(__TARGET_CHIP__)(uint8_t index, uint16_t kHz);
vsf_err_t VSFHAL_PWM_OUT(__TARGET_CHIP__)(uint8_t index, uint16_t count, uint16_t *rate);
vsf_err_t VSFHAL_PWM_IN(__TARGET_CHIP__)(uint8_t index, uint16_t count, uint16_t *rate);

#ifndef VSFCFG_STANDALONE_MODULE
#define vsfhal_pwm_init					VSFHAL_PWM_INIT(__TARGET_CHIP__)
#define vsfhal_pwm_fini					VSFHAL_PWM_FINI(__TARGET_CHIP__)
#define vsfhal_pwm_config_mode			VSFHAL_PWM_CONFIG_MODE(__TARGET_CHIP__)
#define vsfhal_pwm_config_freq			VSFHAL_PWM_CONFIG_FREQ(__TARGET_CHIP__)
#define vsfhal_pwm_out					VSFHAL_PWM_OUT(__TARGET_CHIP__)
#define vsfhal_pwm_in					VSFHAL_PWM_IN(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_MICROWIRE_EN

struct vsfhal_microwire_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, uint16_t kHz, uint8_t sel_polarity);
	vsf_err_t (*transport)(uint8_t index,
							uint32_t opcode, uint8_t opcode_bitlen,
							uint32_t addr, uint8_t addr_bitlen,
							uint32_t data, uint8_t data_bitlen,
							uint8_t *reply, uint8_t reply_bitlen);
	vsf_err_t (*poll)(uint8_t index, uint16_t interval_us, uint16_t retry_cnt);
};

#endif

#if VSFHAL_TIMER_EN

struct vsfhal_timer_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, uint32_t kHz, uint32_t mode, void (*overflow)(void));
	vsf_err_t (*start)(uint8_t index);
	vsf_err_t (*stop)(uint8_t index);
	vsf_err_t (*get_count)(uint8_t index, uint32_t *count);
	vsf_err_t (*set_count)(uint8_t index, uint32_t count);

	vsf_err_t (*config_channel)(uint8_t index, uint8_t channel, uint32_t mode, void (*callback)(void));
	vsf_err_t (*get_channel)(uint8_t index, uint8_t channel, uint32_t *count);
	vsf_err_t (*set_channel)(uint8_t index, uint8_t channel, uint32_t count);
};

#define VSFHAL_TIMER_INIT(m)			__CONNECT(m, _timer_init)
#define VSFHAL_TIMER_FINI(m)			__CONNECT(m, _timer_fini)
#define VSFHAL_TIMER_CONFIG(m)			__CONNECT(m, _timer_config)
#define VSFHAL_TIMER_START(m)			__CONNECT(m, _timer_start)
#define VSFHAL_TIMER_STOP(m)			__CONNECT(m, _timer_stop)
#define VSFHAL_TIMER_GET_COUNT(m)		__CONNECT(m, _timer_get_count)
#define VSFHAL_TIMER_SET_COUNT(m)		__CONNECT(m, _timer_set_count)
#define VSFHAL_TIMER_CONFIG_CHANNEL(m)	__CONNECT(m, _timer_config_channel)
#define VSFHAL_TIMER_GET_CHANNEL(m)		__CONNECT(m, _timer_get_channel)
#define VSFHAL_TIMER_SET_CHANNEL(m)		__CONNECT(m, _timer_set_channel)

vsf_err_t VSFHAL_TIMER_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_TIMER_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_TIMER_CONFIG(__TARGET_CHIP__)(uint8_t index, uint32_t kHz, uint32_t mode, void (*overflow)(void));
vsf_err_t VSFHAL_TIMER_START(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_TIMER_STOP(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_TIMER_GET_COUNT(__TARGET_CHIP__)(uint8_t index, uint32_t *count);
vsf_err_t VSFHAL_TIMER_SET_COUNT(__TARGET_CHIP__)(uint8_t index, uint32_t count);
vsf_err_t VSFHAL_TIMER_CONFIG_CHANNEL(__TARGET_CHIP__)(uint8_t index, uint8_t channel, uint32_t mode, void (*callback)(void));
vsf_err_t VSFHAL_TIMER_GET_CHANNEL(__TARGET_CHIP__)(uint8_t index, uint8_t channel, uint32_t *count);
vsf_err_t VSFHAL_TIMER_SET_CHANNEL(__TARGET_CHIP__)(uint8_t index, uint8_t channel, uint32_t count);

#endif

#if VSFHAL_EINT_EN

#define VSFHAL_EINT_ONFALL(m)			__CONNECT(m, _EINT_ONFALL)
#define VSFHAL_EINT_ONRISE(m)			__CONNECT(m, _EINT_ONRISE)
#define VSFHAL_EINT_ONLOW(m)			__CONNECT(m, _EINT_ONLOW)
#define VSFHAL_EINT_ONHIGH(m)			__CONNECT(m, _EINT_ONHIGH)
struct vsfhal_eint_t
{
#if VSFHAL_CONST_EN
	struct
	{
		uint32_t ONFALL;
		uint32_t ONRISE;
		uint32_t ONLOW;
		uint32_t ONHIGH;
	} constants;
#endif
	vsf_err_t (*init)(uint32_t index);
	vsf_err_t (*fini)(uint32_t index);
	vsf_err_t (*config)(uint32_t index, uint32_t type, uint32_t int_priority, void *param, void (*callback)(void *param));
	vsf_err_t (*enable)(uint32_t index);
	vsf_err_t (*disable)(uint32_t index);
};

#define VSFHAL_EINT_INIT(m)				__CONNECT(m, _eint_init)
#define VSFHAL_EINT_FINI(m)				__CONNECT(m, _eint_fini)
#define VSFHAL_EINT_CONFIG(m)			__CONNECT(m, _eint_config)
#define VSFHAL_EINT_ENABLE(m)			__CONNECT(m, _eint_enable)
#define VSFHAL_EINT_DISABLE(m)			__CONNECT(m, _eint_disable)

vsf_err_t VSFHAL_EINT_INIT(__TARGET_CHIP__)(uint32_t index);
vsf_err_t VSFHAL_EINT_FINI(__TARGET_CHIP__)(uint32_t index);
vsf_err_t VSFHAL_EINT_CONFIG(__TARGET_CHIP__)(uint32_t index, uint32_t type, uint32_t int_priority, void *param, void (*callback)(void *param));
vsf_err_t VSFHAL_EINT_ENABLE(__TARGET_CHIP__)(uint32_t index);
vsf_err_t VSFHAL_EINT_DISABLE(__TARGET_CHIP__)(uint32_t index);

#ifndef VSFCFG_STANDALONE_MODULE
#define EINT_ONFALL						VSFHAL_EINT_ONFALL(__TARGET_CHIP__)
#define EINT_ONRISE						VSFHAL_EINT_ONRISE(__TARGET_CHIP__)
#define EINT_ONLOW						VSFHAL_EINT_ONLOW(__TARGET_CHIP__)
#define EINT_ONHIGH						VSFHAL_EINT_ONHIGH(__TARGET_CHIP__)

#define vsfhal_eint_init				VSFHAL_EINT_INIT(__TARGET_CHIP__)
#define vsfhal_eint_fini				VSFHAL_EINT_FINI(__TARGET_CHIP__)
#define vsfhal_eint_config				VSFHAL_EINT_CONFIG(__TARGET_CHIP__)
#define vsfhal_eint_enable				VSFHAL_EINT_ENABLE(__TARGET_CHIP__)
#define vsfhal_eint_disable				VSFHAL_EINT_DISABLE(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_NAND_EN
struct nand_info_t
{
	uint32_t clock_hz;
	struct nand_ecc_t
	{
		bool ecc_enable;
		uint16_t ecc_page_size;
	} ecc;
	struct nand_timing_t
	{
		uint8_t ale_to_re_cycle;
		uint8_t cle_to_re_cycle;
		uint16_t setup_cycle;
		uint16_t wait_cycle;
		uint8_t hold_cycle;
		uint8_t hiz_cycle;
		uint16_t setup_cycle_attr;
		uint16_t wait_cycle_attr;
		uint8_t hold_cycle_attr;
		uint8_t hiz_cycle_attr;
	} timing;
};

struct vsfhal3_nand_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, struct nand_info_t *param);
	vsf_err_t (*write_cmd)(uint8_t index, uint8_t *cmd, uint8_t bytelen);
	vsf_err_t (*write_addr)(uint8_t index, uint8_t *addr, uint8_t bytelen);
	vsf_err_t (*write_data)(uint8_t index, uint8_t *data, uint16_t bytelen);
	vsf_err_t (*read_data)(uint8_t index, uint8_t *data, uint16_t bytelen);
};

#define VSFHAL_NAND_INIT(m)				__CONNECT(m, _nand_init)
#define VSFHAL_NAND_FINI(m)				__CONNECT(m, _nand_fini)
#define VSFHAL_NAND_CONFIG(m)			__CONNECT(m, _nand_config)
#define VSFHAL_NAND_WRITE_CMD(m)		__CONNECT(m, _nand_write_cmd)
#define VSFHAL_NAND_WRITE_ADDR(m)		__CONNECT(m, _nand_write_addr)
#define VSFHAL_NAND_WRITE_DATA(m)		__CONNECT(m, _nand_write_data)
#define VSFHAL_NAND_READ_DATA(m)		__CONNECT(m, _nand_read_data)

vsf_err_t VSFHAL_NAND_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_NAND_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_NAND_CONFIG(__TARGET_CHIP__)(uint8_t index, struct nand_info_t *param);
vsf_err_t VSFHAL_NAND_WRITE_CMD(__TARGET_CHIP__)(uint8_t index, uint8_t *cmd, uint8_t bytelen);
vsf_err_t VSFHAL_NAND_WRITE_ADDR(__TARGET_CHIP__)(uint8_t index, uint8_t *addr, uint8_t bytelen);
vsf_err_t VSFHAL_NAND_WRITE_DATA(__TARGET_CHIP__)(uint8_t index, uint8_t *data, uint16_t bytelen);
vsf_err_t VSFHAL_NAND_READ_DATA(__TARGET_CHIP__)(uint8_t index, uint8_t *data, uint16_t bytelen);

#endif

#if VSFHAL_EBI_EN

#define EBI_TGTTYP_NOR					0x00
#define EBI_TGTTYP_SRAM					0x10
#define EBI_TGTTYP_PSRAM				0x20
#define EBI_TGTTYP_NAND					0x30
struct ebi_info_t
{
	uint8_t data_width;
	enum wait_signal_t
	{
		EBI_WAIT_NONE = 0,
		EBI_WAIT_POLHIGH_VI = 1,
		EBI_WAIT_POLHIGH_VN = 2,
		EBI_WAIT_POLLOW_VI = 3,
		EBI_WAIT_POLLOW_VN = 4
	} wait_signal;

	// mux_addr_mask is used when a multiplexer(eg. 74LS138) is used.
	// If no multiplexer is used, set mux_addr_mask to 0.
	// mux_addr_mask defines the address mask to select current chip.
	uint32_t mux_addr_mask;
};
struct ebi_sram_psram_nor_param_t
{
	// A0-15 == D0-15 with ALE
	bool addr_multiplex;

	struct ebi_sram_param_nor_timing_t
	{
		uint16_t address_setup_cycle_r;
		uint16_t address_hold_cycle_r;
		uint16_t data_setup_cycle_r;
		uint32_t clock_hz_r;
		uint16_t address_setup_cycle_w;
		uint16_t address_hold_cycle_w;
		uint16_t data_setup_cycle_w;
		uint32_t clock_hz_w;
	} timing;
};
struct ebi_sram_psram_nor_info_t
{
	struct ebi_info_t common_info;
	struct ebi_sram_psram_nor_param_t param;
};
struct ebi_nand_info_t
{
	struct ebi_info_t common_info;
	struct ebi_nand_param_t
	{
		uint32_t clock_hz;
		struct ebi_nand_ecc_t
		{
			bool ecc_enable;
			uint16_t ecc_page_size;
		} ecc;
		struct ebi_nand_timing_t
		{
			uint8_t ale_to_re_cycle;
			uint8_t cle_to_re_cycle;
			uint16_t setup_cycle;
			uint16_t wait_cycle;
			uint8_t hold_cycle;
			uint8_t hiz_cycle;
			uint16_t setup_cycle_attr;
			uint16_t wait_cycle_attr;
			uint8_t hold_cycle_attr;
			uint8_t hiz_cycle_attr;
		} timing;
		struct ebi_nand_addr_t
		{
			uint32_t cmd;
			uint32_t addr;
			uint32_t data;
		} addr;
	} param;
};
struct ebi_sdram_info_t
{
	struct ebi_info_t common_info;
};
struct ebi_ddram_info_t
{
	struct ebi_info_t common_info;
};
struct ebi_pccard_info_t
{
	struct ebi_info_t common_info;
};
struct vsfhal_ebi_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);

	vsf_err_t (*config)(uint8_t index, uint8_t target_index, void *param);
	vsf_err_t (*config_sram)(uint8_t index, struct ebi_sram_psram_nor_info_t *info);
	vsf_err_t (*config_psram)(uint8_t index, struct ebi_sram_psram_nor_info_t *info);
	vsf_err_t (*config_nor)(uint8_t index, struct ebi_sram_psram_nor_info_t *info);
	vsf_err_t (*config_nand)(uint8_t index, struct ebi_nand_info_t *info);
	vsf_err_t (*config_sdram)(uint8_t index, struct ebi_sdram_info_t *info);
	vsf_err_t (*config_ddram)(uint8_t index, struct ebi_ddram_info_t *info);
	vsf_err_t (*config_pccard)(uint8_t index, struct ebi_pccard_info_t *info);

	void* (*get_base_addr)(uint8_t index, uint8_t target_index);
	vsf_err_t (*isready)(uint8_t index, uint8_t target_index);

	vsf_err_t (*read)(uint8_t index, uint8_t target_index, uint32_t address, uint8_t data_size, uint8_t *buff, uint32_t count);
	vsf_err_t (*write)(uint8_t index, uint8_t target_index, uint32_t address, uint8_t data_size, uint8_t *buff, uint32_t count);

	uint8_t (*read8)(uint8_t index, uint32_t address);
	void (*write8)(uint8_t index, uint32_t address, uint8_t data);
	uint16_t (*read16)(uint8_t index, uint32_t address);
	void (*write16)(uint8_t index, uint32_t address, uint16_t data);
	uint32_t (*read32)(uint8_t index, uint32_t address);
	void (*write32)(uint8_t index, uint32_t address, uint32_t data);

	vsf_err_t (*readp8)(uint8_t index, uint32_t address, uint32_t count, uint8_t *buff);
	vsf_err_t (*readp8_isready)(uint8_t index);
	vsf_err_t (*writep8)(uint8_t index, uint32_t address, uint32_t count, uint8_t *buff);
	vsf_err_t (*writep8_isready)(uint8_t index);
	vsf_err_t (*readp16)(uint8_t index, uint32_t address, uint32_t count, uint16_t *buff);
	vsf_err_t (*readp16_isready)(uint8_t index);
	vsf_err_t (*writep16)(uint8_t index, uint32_t address, uint32_t count, uint16_t *buff);
	vsf_err_t (*writep16_isready)(uint8_t index);
	vsf_err_t (*readp32)(uint8_t index, uint32_t address, uint32_t count, uint32_t *buff);
	vsf_err_t (*readp32_isready)(uint8_t index);
	vsf_err_t (*writep32)(uint8_t index, uint32_t address, uint32_t count, uint32_t *buff);
	vsf_err_t (*writep32_isready)(uint8_t index);
};

#define VSFHAL_EBI_INIT(m)				__CONNECT(m, _ebi_init)
#define VSFHAL_EBI_FINI(m)				__CONNECT(m, _ebi_fini)
#define VSFHAL_EBI_CONFIG(m)			__CONNECT(m, _ebi_config)
#define VSFHAL_EBI_CONFIG_SRAM(m)		__CONNECT(m, _ebi_config_sram)
#define VSFHAL_EBI_CONFIG_PSRAM(m)		__CONNECT(m, _ebi_config_psram)
#define VSFHAL_EBI_CONFIG_NOR(m)		__CONNECT(m, _ebi_config_nor)
#define VSFHAL_EBI_CONFIG_NAND(m)		__CONNECT(m, _ebi_config_nand)
#define VSFHAL_EBI_CONFIG_SDRAM(m)		__CONNECT(m, _ebi_config_sdram)
#define VSFHAL_EBI_CONFIG_DDRAM(m)		__CONNECT(m, _ebi_config_ddram)
#define VSFHAL_EBI_CONFIG_PCCARD(m)		__CONNECT(m, _ebi_config_pccard)
#define VSFHAL_EBI_GET_BASE_ADDR(m)		__CONNECT(m, _ebi_get_base_addr)
#define VSFHAL_EBI_ISREADY(m)			__CONNECT(m, _ebi_isready)
#define VSFHAL_EBI_READ(m)				__CONNECT(m, _ebi_read)
#define VSFHAL_EBI_WRITE(m)				__CONNECT(m, _ebi_write)
#define VSFHAL_EBI_READ8(m)				__CONNECT(m, _ebi_read8)
#define VSFHAL_EBI_WRITE8(m)			__CONNECT(m, _ebi_write8)
#define VSFHAL_EBI_READ16(m)			__CONNECT(m, _ebi_read16)
#define VSFHAL_EBI_WRITE16(m)			__CONNECT(m, _ebi_write16)
#define VSFHAL_EBI_READ32(m)			__CONNECT(m, _ebi_read32)
#define VSFHAL_EBI_WRITE32(m)			__CONNECT(m, _ebi_write32)

vsf_err_t VSFHAL_EBI_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_EBI_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_EBI_CONFIG(__TARGET_CHIP__)(uint8_t index, uint8_t target_index, void *param);
vsf_err_t VSFHAL_EBI_CONFIG_SRAM(__TARGET_CHIP__)(uint8_t index, struct ebi_sram_psram_nor_info_t *info);
vsf_err_t VSFHAL_EBI_CONFIG_PSRAM(__TARGET_CHIP__)(uint8_t index, struct ebi_sram_psram_nor_info_t *info);
vsf_err_t VSFHAL_EBI_CONFIG_NOR(__TARGET_CHIP__)(uint8_t index, struct ebi_sram_psram_nor_info_t *info);
vsf_err_t VSFHAL_EBI_CONFIG_NAND(__TARGET_CHIP__)(uint8_t index, struct ebi_nand_info_t *info);
vsf_err_t VSFHAL_EBI_CONFIG_SDRAM(__TARGET_CHIP__)(uint8_t index, struct ebi_sdram_info_t *info);
vsf_err_t VSFHAL_EBI_CONFIG_DDRAM(__TARGET_CHIP__)(uint8_t index, struct ebi_ddram_info_t *info);
vsf_err_t VSFHAL_EBI_CONFIG_PCCARD(__TARGET_CHIP__)(uint8_t index, struct ebi_pccard_info_t *info);
void* VSFHAL_EBI_GET_BASE_ADDR(__TARGET_CHIP__)(uint8_t index, uint8_t target_index);
vsf_err_t VSFHAL_EBI_ISREADY(__TARGET_CHIP__)(uint8_t index, uint8_t target_index);
vsf_err_t VSFHAL_EBI_READ(__TARGET_CHIP__)(uint8_t index, uint8_t target_index, uint32_t address, uint8_t data_size, uint8_t *buff, uint32_t count);
vsf_err_t VSFHAL_EBI_WRITE(__TARGET_CHIP__)(uint8_t index, uint8_t target_index, uint32_t address, uint8_t data_size, uint8_t *buff, uint32_t count);
uint8_t VSFHAL_EBI_READ8(__TARGET_CHIP__)(uint8_t index, uint32_t address);
void VSFHAL_EBI_WRITE8(__TARGET_CHIP__)(uint8_t index, uint32_t address, uint8_t data);
uint16_t VSFHAL_EBI_READ16(__TARGET_CHIP__)(uint8_t index, uint32_t address);
void VSFHAL_EBI_WRITE16(__TARGET_CHIP__)(uint8_t index, uint32_t address, uint16_t data);
uint32_t VSFHAL_EBI_READ32(__TARGET_CHIP__)(uint8_t index, uint32_t address);
void VSFHAL_EBI_WRITE32(__TARGET_CHIP__)(uint8_t index, uint32_t address, uint32_t data);

#endif

#if VSFHAL_SDIO_EN

#define SDIO_WRITE_NORESP		0
#define SDIO_WRITE_SHORTRESP	0x40
#define SDIO_WRITE_LONGRESP		0x80
#define SDIO_READ_SHORTRESP		0xC0

struct sdio_info_t
{
	uint8_t crc7_error : 1;
	uint8_t crc16_error : 1;
	uint8_t overtime_error : 1;
	uint8_t unknown_error : 1;
	uint8_t manual_stop : 1;

	uint8_t need_resp : 1;
	uint8_t long_resp : 1;
	uint8_t read0_write1 : 1;

	uint8_t block_cnt;
	uint16_t block_len;
	void *data_align4;

	uint32_t resp[4];
};

struct vsfhal_sdio_t
{
	vsf_err_t (*init)(uint8_t index);
	vsf_err_t (*fini)(uint8_t index);
	vsf_err_t (*config)(uint8_t index, uint32_t kHz, uint8_t buswidth,
			void (*callback)(void *), void *param);
	vsf_err_t (*start)(uint8_t index, uint8_t cmd, uint32_t arg,
			struct sdio_info_t *extra_param);
	vsf_err_t (*stop)(uint8_t index);
	vsf_err_t (*config_int)(uint8_t index,
			void (*callback)(void *param), void *param);
	vsf_err_t (*enable_int)(uint8_t index);
};

#define SDIO_RESP_NONE					0
#define SDIO_RESP_SHORT					1
#define SDIO_RESP_LONG					2

#define VSFHAL_SDIO_INIT(m)				__CONNECT(m, _sdio_init)
#define VSFHAL_SDIO_FINI(m)				__CONNECT(m, _sdio_fini)
#define VSFHAL_SDIO_CONFIG(m)			__CONNECT(m, _sdio_config)
#define VSFHAL_SDIO_START(m)			__CONNECT(m, _sdio_start)
#define VSFHAL_SDIO_STOP(m)				__CONNECT(m, _sdio_stop)
#define VSFHAL_SDIO_CONFIG_INT(m)		__CONNECT(m, _sdio_config_int)
#define VSFHAL_SDIO_ENABLE_INT(m)		__CONNECT(m, _sdio_enable_int)

vsf_err_t VSFHAL_SDIO_INIT(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_SDIO_FINI(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_SDIO_CONFIG(__TARGET_CHIP__)(uint8_t index, uint32_t kHz,
		uint8_t buswidth, void (*callback)(void *),
		void *param);
vsf_err_t VSFHAL_SDIO_START(__TARGET_CHIP__)(uint8_t index, uint8_t dir_cmd,
		uint32_t arg, struct sdio_info_t *extra_param);
vsf_err_t VSFHAL_SDIO_STOP(__TARGET_CHIP__)(uint8_t index);
vsf_err_t VSFHAL_SDIO_CONFIG_INT(__TARGET_CHIP__)(uint8_t index,
		void (*callback)(void *param), void *param);
vsf_err_t VSFHAL_SDIO_ENABLE_INT(__TARGET_CHIP__)(uint8_t index);

#endif

enum vsfhal_usbd_eptype_t
{
	USB_EP_TYPE_CONTROL,
	USB_EP_TYPE_INTERRUPT,
	USB_EP_TYPE_BULK,
	USB_EP_TYPE_ISO
};

enum vsfhal_usbd_error_t
{
	USBERR_ERROR,
	USBERR_INVALID_CRC,
	USBERR_SOF_TO,
};

struct vsfhal_usbd_callback_t
{
	void *param;
	vsf_err_t (*on_attach)(void*);
	vsf_err_t (*on_detach)(void*);
	vsf_err_t (*on_reset)(void*);
	vsf_err_t (*on_setup)(void*);
	vsf_err_t (*on_error)(void*, enum vsfhal_usbd_error_t error);
	vsf_err_t (*on_wakeup)(void*);
	vsf_err_t (*on_suspend)(void*);
	vsf_err_t (*on_sof)(void*);
	vsf_err_t (*on_underflow)(void*, uint8_t);
	vsf_err_t (*on_overflow)(void*, uint8_t);
	vsf_err_t (*on_in)(void*, uint8_t);
	vsf_err_t (*on_out)(void*, uint8_t);
};

struct vsfhal_usbd_t
{
	vsf_err_t (*init)(int32_t int_priority);
	vsf_err_t (*fini)(void);
	vsf_err_t (*poll)(void);
	vsf_err_t (*reset)(void);

	vsf_err_t (*connect)(void);
	vsf_err_t (*disconnect)(void);

	vsf_err_t (*set_address)(uint8_t addr);
	uint8_t (*get_address)(void);

	vsf_err_t (*suspend)(void);
	vsf_err_t (*resume)(void);
	vsf_err_t (*lowpower)(uint8_t level);

	uint32_t (*get_frame_number)(void);

	vsf_err_t (*get_setup)(uint8_t *buffer);
	vsf_err_t (*prepare_buffer)(void);
	struct usbd_endpoint_t
	{
		const uint8_t *num_of_ep;

		vsf_err_t (*reset)(uint8_t idx);
		vsf_err_t (*set_type)(uint8_t idx, enum vsfhal_usbd_eptype_t type);

		vsf_err_t (*set_IN_dbuffer)(uint8_t idx);
		bool (*is_IN_dbuffer)(uint8_t idx);
		vsf_err_t (*switch_IN_buffer)(uint8_t idx);
		vsf_err_t (*set_IN_epsize)(uint8_t idx, uint16_t size);
		uint16_t (*get_IN_epsize)(uint8_t idx);
		vsf_err_t (*set_IN_stall)(uint8_t idx);
		vsf_err_t (*clear_IN_stall)(uint8_t idx);
		bool (*is_IN_stall)(uint8_t idx);
		vsf_err_t (*reset_IN_toggle)(uint8_t idx);
		vsf_err_t (*toggle_IN_toggle)(uint8_t idx);
		vsf_err_t (*set_IN_count)(uint8_t idx, uint16_t size);
		vsf_err_t (*write_IN_buffer)(uint8_t idx, uint8_t *buffer, uint16_t size);

		vsf_err_t (*set_OUT_dbuffer)(uint8_t idx);
		bool (*is_OUT_dbuffer)(uint8_t idx);
		vsf_err_t (*switch_OUT_buffer)(uint8_t idx);
		vsf_err_t (*set_OUT_epsize)(uint8_t idx, uint16_t size);
		uint16_t (*get_OUT_epsize)(uint8_t idx);
		vsf_err_t (*set_OUT_stall)(uint8_t idx);
		vsf_err_t (*clear_OUT_stall)(uint8_t idx);
		bool (*is_OUT_stall)(uint8_t idx);
		vsf_err_t (*reset_OUT_toggle)(uint8_t idx);
		vsf_err_t (*toggle_OUT_toggle)(uint8_t idx);
		uint16_t (*get_OUT_count)(uint8_t idx);
		vsf_err_t (*read_OUT_buffer)(uint8_t idx, uint8_t *buffer, uint16_t size);
		vsf_err_t (*enable_OUT)(uint8_t idx);
	} ep;
	struct vsfhal_usbd_callback_t *callback;
};

#if VSFHAL_USBD_EN

#define VSFHAL_USBD_INIT(m)				__CONNECT(m, _usbd_init)
#define VSFHAL_USBD_FINI(m)				__CONNECT(m, _usbd_fini)
#define VSFHAL_USBD_RESET(m)			__CONNECT(m, _usbd_reset)
#define VSFHAL_USBD_POLL(m)				__CONNECT(m, _usbd_poll)
#define VSFHAL_USBD_CONNECT(m)			__CONNECT(m, _usbd_connect)
#define VSFHAL_USBD_DISCONNECT(m)		__CONNECT(m, _usbd_disconnect)
#define VSFHAL_USBD_SET_ADDRESS(m)		__CONNECT(m, _usbd_set_address)
#define VSFHAL_USBD_GET_ADDRESS(m)		__CONNECT(m, _usbd_get_address)
#define VSFHAL_USBD_SUSPEND(m)			__CONNECT(m, _usbd_suspend)
#define VSFHAL_USBD_RESUME(m)			__CONNECT(m, _usbd_resume)
#define VSFHAL_USBD_LOWPOWER(m)			__CONNECT(m, _usbd_lowpower)
#define VSFHAL_USBD_GET_FRAME_NUM(m)	__CONNECT(m, _usbd_get_frame_number)
#define VSFHAL_USBD_GET_SETUP(m)		__CONNECT(m, _usbd_get_setup)
#define VSFHAL_USBD_PREPARE_BUFFER(m)	__CONNECT(m, _usbd_prepare_buffer)
#define VSFHAL_USBD_EP_NUM(m)			__CONNECT(m, _usbd_ep_num)
#define VSFHAL_USBD_EP_RESET(m)			__CONNECT(m, _usbd_ep_reset)
#define VSFHAL_USBD_EP_SET_TYPE(m)		__CONNECT(m, _usbd_ep_set_type)
#define VSFHAL_USBD_EP_SET_IN_DBUFFER(m)__CONNECT(m, _usbd_ep_set_IN_dbuffer)
#define VSFHAL_USBD_EP_IS_IN_DBUFFER(m)	__CONNECT(m, _usbd_ep_is_IN_dbuffer)
#define VSFHAL_USBD_EP_SWITCH_IN_BUFFER(m)\
										__CONNECT(m, _usbd_ep_switch_IN_buffer)
#define VSFHAL_USBD_EP_SET_IN_EPSIZE(m)	__CONNECT(m, _usbd_ep_set_IN_epsize)
#define VSFHAL_USBD_EP_GET_IN_EPSIZE(m)	__CONNECT(m, _usbd_ep_get_IN_epsize)
#define VSFHAL_USBD_EP_SET_IN_STALL(m)	__CONNECT(m, _usbd_ep_set_IN_stall)
#define VSFHAL_USBD_EP_CLEAR_IN_STALL(m)__CONNECT(m, _usbd_ep_clear_IN_stall)
#define VSFHAL_USBD_EP_IS_IN_STALL(m)	__CONNECT(m, _usbd_ep_is_IN_stall)
#define VSFHAL_USBD_EP_RESET_IN_TOGGLE(m)\
										__CONNECT(m, _usbd_ep_reset_IN_toggle)
#define VSFHAL_USBD_EP_TOGGLE_IN_TOGGLE(m)\
										__CONNECT(m, _usbd_ep_toggle_IN_toggle)
#define VSFHAL_USBD_EP_SET_IN_COUNT(m)	__CONNECT(m, _usbd_ep_set_IN_count)
#define VSFHAL_USBD_EP_WRITE_IN_BUFFER(m)\
										__CONNECT(m, _usbd_ep_write_IN_buffer)
#define VSFHAL_USBD_EP_SET_OUT_DBUFFER(m)\
										__CONNECT(m, _usbd_ep_set_OUT_dbuffer)
#define VSFHAL_USBD_EP_IS_OUT_DBUFFER(m)__CONNECT(m, _usbd_ep_is_OUT_dbuffer)
#define VSFHAL_USBD_EP_SWITCH_OUT_BUFFER(m)\
										__CONNECT(m, _usbd_ep_switch_OUT_buffer)
#define VSFHAL_USBD_EP_SET_OUT_EPSIZE(m)__CONNECT(m, _usbd_ep_set_OUT_epsize)
#define VSFHAL_USBD_EP_GET_OUT_EPSIZE(m)__CONNECT(m, _usbd_ep_get_OUT_epsize)
#define VSFHAL_USBD_EP_SET_OUT_STALL(m)	__CONNECT(m, _usbd_ep_set_OUT_stall)
#define VSFHAL_USBD_EP_CLEAR_OUT_STALL(m)\
										__CONNECT(m, _usbd_ep_clear_OUT_stall)
#define VSFHAL_USBD_EP_IS_OUT_STALL(m)	__CONNECT(m, _usbd_ep_is_OUT_stall)
#define VSFHAL_USBD_EP_RESET_OUT_TOGGLE(m)\
										__CONNECT(m, _usbd_ep_reset_OUT_toggle)
#define VSFHAL_USBD_EP_TOGGLE_OUT_TOGGLE(m)\
										__CONNECT(m, _usbd_ep_toggle_OUT_toggle)
#define VSFHAL_USBD_EP_GET_OUT_COUNT(m)	__CONNECT(m, _usbd_ep_get_OUT_count)
#define VSFHAL_USBD_EP_READ_OUT_BUFFER(m)\
										__CONNECT(m, _usbd_ep_read_OUT_buffer)
#define VSFHAL_USBD_EP_ENABLE_OUT(m)	__CONNECT(m, _usbd_ep_enable_OUT)
#define VSFHAL_USBD_CALLBACK(m)			__CONNECT(m, _usbd_callback)

vsf_err_t VSFHAL_USBD_INIT(__TARGET_CHIP__)(int32_t int_priority);
vsf_err_t VSFHAL_USBD_FINI(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_RESET(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_POLL(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_CONNECT(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_DISCONNECT(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_SET_ADDRESS(__TARGET_CHIP__)(uint8_t addr);
uint8_t VSFHAL_USBD_GET_ADDRESS(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_SUSPEND(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_RESUME(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_LOWPOWER(__TARGET_CHIP__)(uint8_t level);
uint32_t VSFHAL_USBD_GET_FRAME_NUM(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBD_GET_SETUP(__TARGET_CHIP__)(uint8_t *buffer);
vsf_err_t VSFHAL_USBD_PREPARE_BUFFER(__TARGET_CHIP__)(void);
extern const uint8_t VSFHAL_USBD_EP_NUM(__TARGET_CHIP__);
vsf_err_t VSFHAL_USBD_EP_RESET(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SET_TYPE(__TARGET_CHIP__)(uint8_t idx, enum vsfhal_usbd_eptype_t type);
vsf_err_t VSFHAL_USBD_EP_SET_IN_DBUFFER(__TARGET_CHIP__)(uint8_t idx);
bool VSFHAL_USBD_EP_IS_IN_DBUFFER(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SWITCH_IN_BUFFER(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SET_IN_EPSIZE(__TARGET_CHIP__)(uint8_t idx, uint16_t size);
uint16_t VSFHAL_USBD_EP_GET_IN_EPSIZE(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SET_IN_STALL(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_CLEAR_IN_STALL(__TARGET_CHIP__)(uint8_t idx);
bool VSFHAL_USBD_EP_IS_IN_STALL(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_RESET_IN_TOGGLE(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_TOGGLE_IN_TOGGLE(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SET_IN_COUNT(__TARGET_CHIP__)(uint8_t idx, uint16_t size);
vsf_err_t VSFHAL_USBD_EP_WRITE_IN_BUFFER(__TARGET_CHIP__)(uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t VSFHAL_USBD_EP_SET_OUT_DBUFFER(__TARGET_CHIP__)(uint8_t idx);
bool VSFHAL_USBD_EP_IS_OUT_DBUFFER(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SWITCH_OUT_BUFFER(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SET_OUT_EPSIZE(__TARGET_CHIP__)(uint8_t idx, uint16_t size);
uint16_t VSFHAL_USBD_EP_GET_OUT_EPSIZE(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_SET_OUT_STALL(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_CLEAR_OUT_STALL(__TARGET_CHIP__)(uint8_t idx);
bool VSFHAL_USBD_EP_IS_OUT_STALL(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_RESET_OUT_TOGGLE(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_TOGGLE_OUT_TOGGLE(__TARGET_CHIP__)(uint8_t idx);
uint16_t VSFHAL_USBD_EP_GET_OUT_COUNT(__TARGET_CHIP__)(uint8_t idx);
vsf_err_t VSFHAL_USBD_EP_READ_OUT_BUFFER(__TARGET_CHIP__)(uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t VSFHAL_USBD_EP_ENABLE_OUT(__TARGET_CHIP__)(uint8_t idx);
extern struct vsfhal_usbd_callback_t VSFHAL_USBD_CALLBACK(__TARGET_CHIP__);

extern const struct vsfhal_usbd_t vsfhal_usbd;

#endif

#if VSFHAL_USBDIO_EN
enum usbdio_evt_t
{
	USBDIO_EVT_RST,
	USBDIO_EVT_SOF,
	USBDIO_EVT_DAT,
};

struct vsfhal_usbdio_t
{
	vsf_err_t (*init)(void (*onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len));
	vsf_err_t (*fini)(void);
	vsf_err_t (*tx)(uint8_t *buf, uint16_t len);
};

#define VSFHAL_USBDIO_INIT(m)			__CONNECT(m, _usbdio_init)
#define VSFHAL_USBDIO_FINI(m)			__CONNECT(m, _usbdio_fini)
#define VSFHAL_USBDIO_TX(m)				__CONNECT(m, _usbdio_tx)

vsf_err_t VSFHAL_USBDIO_INIT(__TARGET_CHIP__)(void (*onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len));
vsf_err_t VSFHAL_USBDIO_FINI(__TARGET_CHIP__)(void);
vsf_err_t VSFHAL_USBDIO_TX(__TARGET_CHIP__)(uint8_t *buf, uint16_t len);

#ifndef VSFCFG_STANDALONE_MODULE
#define vsfhal_usbdio_init				VSFHAL_USBDIO_INIT(__TARGET_CHIP__)
#define vsfhal_usbdio_fini				VSFHAL_USBDIO_FINI(__TARGET_CHIP__)
#define vsfhal_usbdio_tx				VSFHAL_USBDIO_TX(__TARGET_CHIP__)
#endif

#endif

#if VSFHAL_HCD_EN
#define VSFHAL_HCD_PORT1(m)				__CONNECT(m, _HCD_PORT1)
#define VSFHAL_HCD_PORT2(m)				__CONNECT(m, _HCD_PORT2)
#define VSFHAL_HCD_PORT3(m)				__CONNECT(m, _HCD_PORT3)
#define VSFHAL_HCD_PORT4(m)				__CONNECT(m, _HCD_PORT4)

struct vsfhal_hcd_t
{
	vsf_err_t (*init)(uint32_t index, vsf_err_t (*hcd_irq)(void *), void *param);
	vsf_err_t (*fini)(uint32_t index);
	void* (*regbase)(uint32_t index);
};

#define VSFHAL_HCD_INIT(m)				__CONNECT(m, _hcd_init)
#define VSFHAL_HCD_FINI(m)				__CONNECT(m, _hcd_fini)
#define VSFHAL_HCD_REGBASE(m)			__CONNECT(m, _hcd_regbase)

vsf_err_t VSFHAL_HCD_INIT(__TARGET_CHIP__)(uint32_t index,
		vsf_err_t (*hcd_irq)(void *), void *param);
vsf_err_t VSFHAL_HCD_FINI(__TARGET_CHIP__)(uint32_t index);
void* VSFHAL_HCD_REGBASE(__TARGET_CHIP__)(uint32_t index);

#ifndef VSFCFG_STANDALONE_MODULE
#define HCD_PORT1						VSFHAL_HCD_PORT1(__TARGET_CHIP__)
#define HCD_PORT2						VSFHAL_HCD_PORT2(__TARGET_CHIP__)
#define HCD_PORT3						VSFHAL_HCD_PORT3(__TARGET_CHIP__)
#define HCD_PORT4						VSFHAL_HCD_PORT3(__TARGET_CHIP__)

#define vsfhal_hcd_init					VSFHAL_HCD_INIT(__TARGET_CHIP__)
#define vsfhal_hcd_fini					VSFHAL_HCD_FINI(__TARGET_CHIP__)
#define vsfhal_hcd_regbase				VSFHAL_HCD_REGBASE(__TARGET_CHIP__)
#endif

#endif

#endif	// __VSFHAL_H_INCLUDED__
