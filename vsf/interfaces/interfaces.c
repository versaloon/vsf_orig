/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces.c                                              *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    interfaces implementation file                            *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2010-12-05:     created(by SimonQian)                             *
 **************************************************************************/

#include "app_cfg.h"
#include "interfaces.h"

vsf_err_t peripheral_commit(void)
{
	return VSFERR_NONE;
}

const struct interfaces_info_t core_interfaces =
{
	// core
	.core.init = CORE_INIT(__TARGET_CHIP__),
	.core.fini = CORE_FINI(__TARGET_CHIP__),
	.core.reset = CORE_RESET(__TARGET_CHIP__),
	.core.get_stack = CORE_GET_STACK(__TARGET_CHIP__),
	.core.set_stack = CORE_SET_STACK(__TARGET_CHIP__),
	.core.sleep = CORE_SLEEP(__TARGET_CHIP__),
	.core.pendsv_config = CORE_PENDSV_CONFIG(__TARGET_CHIP__),
	.core.pendsv_trigger = CORE_PENDSV_TRIGGER(__TARGET_CHIP__),
#if IFS_UNIQUEID_EN
	.uid.get = CORE_UID_GET(__TARGET_CHIP__),
#endif
#if IFS_FLASH_EN
	.flash.direct_read = &CORE_FLASH_DIRECT_READ(__TARGET_CHIP__),
	.flash.init = CORE_FLASH_INIT(__TARGET_CHIP__),
	.flash.fini = CORE_FLASH_FINI(__TARGET_CHIP__),
	.flash.capacity = CORE_FLASH_CAPACITY(__TARGET_CHIP__),
	.flash.baseaddr = CORE_FLASH_BASEADDR(__TARGET_CHIP__),
	.flash.blocksize = CORE_FLASH_BLOCKSIZE(__TARGET_CHIP__),
	.flash.config_cb = CORE_FLASH_CONFIG_CB(__TARGET_CHIP__),
	.flash.erase = CORE_FLASH_ERASE(__TARGET_CHIP__),
	.flash.read = CORE_FLASH_READ(__TARGET_CHIP__),
	.flash.write = CORE_FLASH_WRITE(__TARGET_CHIP__),
#endif
#if IFS_CLKO_EN
	.clko.init = CORE_CLKO_INIT(__TARGET_CHIP__),
	.clko.fini = CORE_CLKO_FINI(__TARGET_CHIP__),
	.clko.config = CORE_CLKO_CONFIG(__TARGET_CHIP__),
	.clko.enable = CORE_CLKO_ENABLE(__TARGET_CHIP__),
	.clko.disable = CORE_CLKO_DISABLE(__TARGET_CHIP__),
#endif
#if IFS_GPIO_EN
#if IFS_CONST_EN
	.gpio.constants.INFLOAT = GPIO_INFLOAT,
	.gpio.constants.INPU = GPIO_INPU,
	.gpio.constants.INPD = GPIO_INPD,
	.gpio.constants.OUTPP = GPIO_OUTPP,
	.gpio.constants.OUTOD = GPIO_OUTOD,
#endif
	.gpio.init = CORE_GPIO_INIT(__TARGET_CHIP__),
	.gpio.fini = CORE_GPIO_FINI(__TARGET_CHIP__),
	.gpio.config_pin = CORE_GPIO_CONFIG_PIN(__TARGET_CHIP__),
	.gpio.config = CORE_GPIO_CONFIG(__TARGET_CHIP__),
	.gpio.set = CORE_GPIO_SET(__TARGET_CHIP__),
	.gpio.clear = CORE_GPIO_CLEAR(__TARGET_CHIP__),
	.gpio.out = CORE_GPIO_OUT(__TARGET_CHIP__),
	.gpio.in = CORE_GPIO_IN(__TARGET_CHIP__),
	.gpio.get = CORE_GPIO_GET(__TARGET_CHIP__),
#endif
#if IFS_TIMER_EN
	.timer.init = CORE_TIMER_INIT(__TARGET_CHIP__),
	.timer.fini = CORE_TIMER_FINI(__TARGET_CHIP__),
	.timer.config = CORE_TIMER_CONFIG(__TARGET_CHIP__),
	.timer.start = CORE_TIMER_START(__TARGET_CHIP__),
	.timer.stop = CORE_TIMER_STOP(__TARGET_CHIP__),
	.timer.get_count = CORE_TIMER_GET_COUNT(__TARGET_CHIP__),
	.timer.set_count = CORE_TIMER_SET_COUNT(__TARGET_CHIP__),
	.timer.config_channel = CORE_TIMER_CONFIG_CHANNEL(__TARGET_CHIP__),
	.timer.get_channel = CORE_TIMER_GET_CHANNEL(__TARGET_CHIP__),
	.timer.set_channel = CORE_TIMER_SET_CHANNEL(__TARGET_CHIP__),
#endif
#if IFS_EINT_EN
#if IFS_CONST_EN
	.eint.constants.ONFALL = EINT_ONFALL,
	.eint.constants.ONRISE = EINT_ONRISE,
	.eint.constants.ONLOW = EINT_ONLOW,
	.eint.constants.ONHIGH = EINT_ONHIGH,
#endif
	.eint.init = CORE_EINT_INIT(__TARGET_CHIP__),
	.eint.fini = CORE_EINT_FINI(__TARGET_CHIP__),
	.eint.config = CORE_EINT_CONFIG(__TARGET_CHIP__),
	.eint.enable = CORE_EINT_ENABLE(__TARGET_CHIP__),
	.eint.disable = CORE_EINT_DISABLE(__TARGET_CHIP__),
#endif
#if IFS_USART_EN
#if IFS_CONST_EN
	.usart.constants.STOBITS_1 = USART_STOPBITS_1,
	.usart.constants.STOPBITS_1P5 = USART_STOPBITS_1P5,
	.usart.constants.STOPBITS2 = USART_STOPBITS_2,
	.usart.constants.PARITY_NONE = USART_PARITY_NONE,
	.usart.constants.PARITY_ODD = USART_PARITY_ODD,
	.usart.constants.PARITY_EVEN = USART_PARITY_EVEN,
#endif
	.usart.init = CORE_USART_INIT(__TARGET_CHIP__),
	.usart.fini = CORE_USART_FINI(__TARGET_CHIP__),
	.usart.config = CORE_USART_CONFIG(__TARGET_CHIP__),
	.usart.config_cb = CORE_USART_CONFIG_CB(__TARGET_CHIP__),
	.usart.tx_bytes = CORE_USART_TX_BYTES(__TARGET_CHIP__),
	.usart.tx_get_free_size = CORE_USART_TX_GET_FREE_SIZE(__TARGET_CHIP__),
	.usart.rx_bytes = CORE_USART_RX_BYTES(__TARGET_CHIP__),
	.usart.rx_get_data_size = CORE_USART_RX_GET_DATA_SIZE(__TARGET_CHIP__),
#endif
#if IFS_SPI_EN
#if IFS_CONST_EN
	.spi.constants.MASTER = SPI_MASTER,
	.spi.constants.SLAVE = SPI_SLAVE,
	.spi.constants.MODE0 = SPI_MODE0,
	.spi.constants.MODE1 = SPI_MODE1,
	.spi.constants.MODE2 = SPI_MODE2,
	.spi.constants.MODE3 = SPI_MODE3,
	.spi.constants.MSB_FIRST = SPI_MSB_FIRST,
	.spi.constants.LSB_FIRST = SPI_LSB_FIRST,
#endif
	.spi.init = CORE_SPI_INIT(__TARGET_CHIP__),
	.spi.fini = CORE_SPI_FINI(__TARGET_CHIP__),
	.spi.get_ability = CORE_SPI_GET_ABILITY(__TARGET_CHIP__),
	.spi.enable = CORE_SPI_ENABLE(__TARGET_CHIP__),
	.spi.disable = CORE_SPI_DISABLE(__TARGET_CHIP__),
	.spi.config = CORE_SPI_CONFIG(__TARGET_CHIP__),
	.spi.config_cb = CORE_SPI_CONFIG_CB(__TARGET_CHIP__),
	.spi.select = CORE_SPI_SELECT(__TARGET_CHIP__),
	.spi.deselect = CORE_SPI_DESELECT(__TARGET_CHIP__),
	.spi.start = CORE_SPI_START(__TARGET_CHIP__),
	.spi.stop = CORE_SPI_STOP(__TARGET_CHIP__),
#endif
#if IFS_ADC_EN
	.adc.init = CORE_ADC_INIT(__TARGET_CHIP__),
	.adc.fini = CORE_ADC_FINI(__TARGET_CHIP__),
	.adc.config = CORE_ADC_CONFIG(__TARGET_CHIP__),
	.adc.config_channel = CORE_ADC_CONFIG_CHANNEL(__TARGET_CHIP__),
	.adc.get_max_value = CORE_ADC_GET_MAX_VALUE(__TARGET_CHIP__),
	.adc.calibrate = CORE_ADC_CALIBRATE(__TARGET_CHIP__),
	.adc.start = CORE_ADC_START(__TARGET_CHIP__),
#endif
#if IFS_IIC_EN
	.i2c.init = CORE_I2C_INIT(__TARGET_CHIP__),
	.i2c.fini = CORE_I2C_FINI(__TARGET_CHIP__),
	.i2c.config = CORE_I2C_CONFIG(__TARGET_CHIP__),
	.i2c.xfer = CORE_I2C_XFER(__TARGET_CHIP__),
#endif
#if IFS_USBD_EN
	.usbd.init = CORE_USBD_INIT(__TARGET_CHIP__),
	.usbd.fini = CORE_USBD_FINI(__TARGET_CHIP__),
	.usbd.poll = CORE_USBD_POLL(__TARGET_CHIP__),
	.usbd.reset = CORE_USBD_RESET(__TARGET_CHIP__),
	.usbd.connect = CORE_USBD_CONNECT(__TARGET_CHIP__),
	.usbd.disconnect = CORE_USBD_DISCONNECT(__TARGET_CHIP__),
	.usbd.set_address = CORE_USBD_SET_ADDRESS(__TARGET_CHIP__),
	.usbd.get_address = CORE_USBD_GET_ADDRESS(__TARGET_CHIP__),
	.usbd.suspend = CORE_USBD_SUSPEND(__TARGET_CHIP__),
	.usbd.resume = CORE_USBD_RESUME(__TARGET_CHIP__),
	.usbd.lowpower = CORE_USBD_LOWPOWER(__TARGET_CHIP__),
	.usbd.get_frame_number = CORE_USBD_GET_FRAME_NUM(__TARGET_CHIP__),
	.usbd.get_setup = CORE_USBD_GET_SETUP(__TARGET_CHIP__),
	.usbd.prepare_buffer = CORE_USBD_PREPARE_BUFFER(__TARGET_CHIP__),
	.usbd.ep.num_of_ep = &CORE_USBD_EP_NUM(__TARGET_CHIP__),
	.usbd.ep.reset = CORE_USBD_EP_RESET(__TARGET_CHIP__),
	.usbd.ep.set_type = CORE_USBD_EP_SET_TYPE(__TARGET_CHIP__),
	.usbd.ep.set_IN_dbuffer = CORE_USBD_EP_SET_IN_DBUFFER(__TARGET_CHIP__),
	.usbd.ep.is_IN_dbuffer = CORE_USBD_EP_IS_IN_DBUFFER(__TARGET_CHIP__),
	.usbd.ep.switch_IN_buffer = CORE_USBD_EP_SWITCH_IN_BUFFER(__TARGET_CHIP__),
	.usbd.ep.set_IN_epsize = CORE_USBD_EP_SET_IN_EPSIZE(__TARGET_CHIP__),
	.usbd.ep.get_IN_epsize = CORE_USBD_EP_GET_IN_EPSIZE(__TARGET_CHIP__),
	.usbd.ep.set_IN_stall = CORE_USBD_EP_SET_IN_STALL(__TARGET_CHIP__),
	.usbd.ep.clear_IN_stall = CORE_USBD_EP_CLEAR_IN_STALL(__TARGET_CHIP__),
	.usbd.ep.is_IN_stall = CORE_USBD_EP_IS_IN_STALL(__TARGET_CHIP__),
	.usbd.ep.reset_IN_toggle = CORE_USBD_EP_RESET_IN_TOGGLE(__TARGET_CHIP__),
	.usbd.ep.toggle_IN_toggle = CORE_USBD_EP_TOGGLE_IN_TOGGLE(__TARGET_CHIP__),
	.usbd.ep.set_IN_count = CORE_USBD_EP_SET_IN_COUNT(__TARGET_CHIP__),
	.usbd.ep.write_IN_buffer = CORE_USBD_EP_WRITE_IN_BUFFER(__TARGET_CHIP__),
	.usbd.ep.set_OUT_dbuffer = CORE_USBD_EP_SET_OUT_DBUFFER(__TARGET_CHIP__),
	.usbd.ep.is_OUT_dbuffer = CORE_USBD_EP_IS_OUT_DBUFFER(__TARGET_CHIP__),
	.usbd.ep.switch_OUT_buffer = CORE_USBD_EP_SWITCH_OUT_BUFFER(__TARGET_CHIP__),
	.usbd.ep.set_OUT_epsize = CORE_USBD_EP_SET_OUT_EPSIZE(__TARGET_CHIP__),
	.usbd.ep.get_OUT_epsize = CORE_USBD_EP_GET_OUT_EPSIZE(__TARGET_CHIP__),
	.usbd.ep.set_OUT_stall = CORE_USBD_EP_SET_OUT_STALL(__TARGET_CHIP__),
	.usbd.ep.clear_OUT_stall = CORE_USBD_EP_CLEAR_OUT_STALL(__TARGET_CHIP__),
	.usbd.ep.is_OUT_stall = CORE_USBD_EP_IS_OUT_STALL(__TARGET_CHIP__),
	.usbd.ep.reset_OUT_toggle = CORE_USBD_EP_RESET_OUT_TOGGLE(__TARGET_CHIP__),
	.usbd.ep.toggle_OUT_toggle = CORE_USBD_EP_TOGGLE_OUT_TOGGLE(__TARGET_CHIP__),
	.usbd.ep.get_OUT_count = CORE_USBD_EP_GET_OUT_COUNT(__TARGET_CHIP__),
	.usbd.ep.read_OUT_buffer = CORE_USBD_EP_READ_OUT_BUFFER(__TARGET_CHIP__),
	.usbd.ep.enable_OUT = CORE_USBD_EP_ENABLE_OUT(__TARGET_CHIP__),
	.usbd.callback = &CORE_USBD_CALLBACK(__TARGET_CHIP__),
#endif
#if IFS_PWM_EN
	.pwm.init = CORE_PWM_INIT(__TARGET_CHIP__),
	.pwm.fini = CORE_PWM_FINI(__TARGET_CHIP__),
	.pwm.config_mode = CORE_PWM_CONFIG_MODE(__TARGET_CHIP__),
	.pwm.config_freq = CORE_PWM_CONFIG_FREQ(__TARGET_CHIP__),
	.pwm.out = CORE_PWM_OUT(__TARGET_CHIP__),
	.pwm.in = CORE_PWM_IN(__TARGET_CHIP__),
#endif
#if IFS_MICROWIRE_EN
	.microwire.init = NULL,
	.microwire.fini = NULL,
#endif
#if IFS_NAND_EN
	.nand.init = CORE_NAND_INIT(__TARGET_CHIP__),
	.nand.fini = CORE_NAND_FINI(__TARGET_CHIP__),
	.nand.config = CORE_NAND_CONFIG(__TARGET_CHIP__),
	.nand.write_cmd = CORE_NAND_WRITE_CMD(__TARGET_CHIP__),
	.nand.write_addr = CORE_NAND_WRITE_ADDR(__TARGET_CHIP__),
	.nand.write_data = CORE_NAND_WRITE_DATA(__TARGET_CHIP__),
	.nand.read_data = CORE_NAND_READ_DATA(__TARGET_CHIP__),
#endif
#if IFS_EBI_EN
	.ebi.init = CORE_EBI_INIT(__TARGET_CHIP__),
	.ebi.fini = CORE_EBI_FINI(__TARGET_CHIP__),

	.ebi.config = CORE_EBI_CONFIG(__TARGET_CHIP__),
	.ebi.config_sram = CORE_EBI_CONFIG_SRAM(__TARGET_CHIP__),
	.ebi.config_psram = CORE_EBI_CONFIG_PSRAM(__TARGET_CHIP__),
	.ebi.config_nor = CORE_EBI_CONFIG_NOR(__TARGET_CHIP__),
	.ebi.config_nand = CORE_EBI_CONFIG_NAND(__TARGET_CHIP__),
	.ebi.config_sdram = CORE_EBI_CONFIG_SDRAM(__TARGET_CHIP__),
	.ebi.config_ddram = CORE_EBI_CONFIG_DDRAM(__TARGET_CHIP__),
	.ebi.config_pccard = CORE_EBI_CONFIG_PCCARD(__TARGET_CHIP__),

	.ebi.get_base_addr = CORE_EBI_GET_BASE_ADDR(__TARGET_CHIP__),
#endif
#if IFS_SDIO_EN
	.sdio.init = CORE_SDIO_INIT(__TARGET_CHIP__),
	.sdio.fini = CORE_SDIO_FINI(__TARGET_CHIP__),
#endif
#if IFS_HCD_EN
	.hcd.init = CORE_HCD_INIT(__TARGET_CHIP__),
	.hcd.fini = CORE_HCD_FINI(__TARGET_CHIP__),
	.hcd.regbase = CORE_HCD_REGBASE(__TARGET_CHIP__),
#endif
	.tickclk.init = CORE_TICKCLK_INIT(__TARGET_CHIP__),
	.tickclk.fini = CORE_TICKCLK_FINI(__TARGET_CHIP__),
	.tickclk.poll = CORE_TICKCLK_POLL(__TARGET_CHIP__),
	.tickclk.start = CORE_TICKCLK_START(__TARGET_CHIP__),
	.tickclk.stop = CORE_TICKCLK_STOP(__TARGET_CHIP__),
	.tickclk.get_count = CORE_TICKCLK_GET_COUNT(__TARGET_CHIP__),
	.tickclk.config_cb = CORE_TICKCLK_CONFIG_CB(__TARGET_CHIP__),
};

const struct interfaces_info_t *interfaces = &core_interfaces;
