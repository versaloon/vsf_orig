#include "app_cfg.h"
#include "app_type.h"
#include "interfaces.h"

#if IFS_I2C_EN

#include "../core.h"
#define IIC_NUM				2

struct i2c_ctrl_t
{
	uint8_t chip_addr;
	uint8_t msg_len;
	uint8_t msg_prt;
	uint16_t msg_buf_prt;
	struct interface_i2c_msg_t *msg;
	void *param;
	void (*callback)(void *, vsf_err_t);
} static gd32f1x0_i2c[IIC_NUM];

struct i2c_param_t
{
	I2C_TypeDef *reg;
	struct gd32f1x0_afio_pin_t sda[3];
	struct gd32f1x0_afio_pin_t scl[3];
} static const gd32f1x0_i2c_param[IIC_NUM] = 
{
#if IIC_NUM >= 1
	{
		I2C1,
		{
			{0, 10, 4},		// PA10, AF4
			{1, 7, 1},		// PB7, AF1
			{1, 9, 1},		// PB9, AF1
		},
		{
			{0, 9, 4},		// PA9, AF4
			{1, 6, 1},		// PB6, AF1
			{1, 8, 1},		// PB8, AF1
		},
	},
#endif
#if IIC_NUM >= 2
	{
		I2C2,
		{
			{0, 1, 4},		// PA1, AF4
			{1, 11, 1},		// PB11, AF1
			{6, 7, -1},		// PF7, default
		},
		{
			{0, 0, 4},		// PA0, AF4
			{1, 10, 1},		// PB10, AF1
			{6, 6, -1},		// PF6, default
		},
	},
#endif
};

// index spec(LSB to MSB):
// 2-bit iic_idx, 3-bit sda_idx, 3-bit scl_idx
vsf_err_t gd32f1x0_i2c_init(uint8_t index)
{
	uint8_t iic_idx = (index & 0x03) >> 0;
	uint8_t sda_idx = (index & 0x1C) >> 2;
	uint8_t scl_idx = (index & 0xE0) >> 5;
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;

	gd32f1x0_afio_config(&gd32f1x0_i2c_param[iic_idx].sda[sda_idx], gd32f1x0_GPIO_AF | gd32f1x0_GPIO_OD);
	gd32f1x0_afio_config(&gd32f1x0_i2c_param[iic_idx].scl[scl_idx], gd32f1x0_GPIO_AF | gd32f1x0_GPIO_OD);
	
	RCC->APB1CCR |= RCC_APB1CCR_I2C1EN << iic_idx;
	reg->CTLR1 = 0;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_i2c_fini(uint8_t index)
{
	uint8_t iic_idx = (index & 0x03) >> 0;
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;
	reg->CTLR1 = 0;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_i2c_config(uint8_t index, uint16_t kHz)
{
	uint8_t iic_idx = (index & 0x03) >> 0;
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;
	struct gd32f1x0_info_t *info;
	uint16_t iic_clk, iic_freq;
	
	gd32f1x0_interface_get_info(&info);
	iic_clk = info->apb_freq_hz / 1000000;
	iic_clk = min(iic_clk, 36);
	reg->CTLR2 = iic_clk;
	
	if (kHz <= 100)
	{
		// standard mode
		iic_freq = info->apb_freq_hz / (kHz * 2000);
		if (iic_freq < 4)
			iic_freq = 4;
		reg->RTR = (iic_clk >= 36) ? 36 : iic_clk + 1;
	}
	else
	{
		// fast mode
		iic_freq = info->apb_freq_hz / (kHz * 3000);
		if (!iic_freq)
			iic_freq = 1;
		reg->RTR = (uint16_t)(((iic_clk * 300) / 1000) + 1);
	}
	reg->CLKR = iic_freq;
	reg->CTLR1 = I2C_CTLR1_I2CEN;
	
	switch (iic_idx)
	{
#if IIC_NUM >= 1
	case 0: NVIC_EnableIRQ(I2C1_EV_IRQn); NVIC_EnableIRQ(I2C1_ER_IRQn); break;
#endif
#if IIC_NUM >= 2
	case 1: NVIC_EnableIRQ(I2C2_EV_IRQn); NVIC_EnableIRQ(I2C2_ER_IRQn); break;
#endif
	}
	reg->CTLR2 |= I2C_CTLR2_EE | I2C_CTLR2_EIE | I2C_CTLR2_BIE;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_i2c_config_cb(uint8_t index, void *param,
							void (*callback)(void *, vsf_err_t))
{
	uint8_t iic_idx = (index & 0x03) >> 0;
	gd32f1x0_i2c[iic_idx].param = param;
	gd32f1x0_i2c[iic_idx].callback = callback;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_i2c_xfer(uint8_t index, uint16_t chip_addr,
							struct interface_i2c_msg_t *msg, uint8_t msg_len)
{
	uint8_t iic_idx = (index & 0x03) >> 0;
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;

	if ((msg == NULL) || (msg_len == 0))
		return VSFERR_INVALID_PARAMETER;

	gd32f1x0_i2c[iic_idx].chip_addr = chip_addr;
	gd32f1x0_i2c[iic_idx].msg = msg;
	gd32f1x0_i2c[iic_idx].msg_len = msg_len;
	gd32f1x0_i2c[iic_idx].msg_prt = 0;
	gd32f1x0_i2c[iic_idx].msg_buf_prt = 0;

	reg->CTLR1 |= I2C_CTLR1_GENSTA;
	return VSFERR_NONE;
}

static void gd32f1x0_i2c_cb(uint8_t iic_idx, vsf_err_t err)
{
	struct i2c_ctrl_t *ctrl = &gd32f1x0_i2c[iic_idx];
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;
	
	// MUST wait STOP clear, or next START will fail
	reg->CTLR1 |= I2C_CTLR1_GENSTP;
	while (reg->CTLR1 & I2C_CTLR1_GENSTP);
	if (ctrl->callback != NULL)
		ctrl->callback(ctrl->param, err);
}

static void gd32f1x0_i2c_chk_recv_next(uint8_t iic_idx)
{
	struct i2c_ctrl_t *ctrl = &gd32f1x0_i2c[iic_idx];
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;
	struct interface_i2c_msg_t *msg = &ctrl->msg[ctrl->msg_prt];

	if ((msg->flag & I2C_ACKLAST) || (ctrl->msg_buf_prt < msg->len - 1) ||
		((ctrl->msg_len > ctrl->msg_prt + 1) &&
			(ctrl->msg[ctrl->msg_prt + 1].flag & (I2C_READ | I2C_NOSTART))))
	{
		reg->CTLR1 |= I2C_CTLR1_ACKEN;
	}
	else
	{
		reg->CTLR1 &= ~I2C_CTLR1_ACKEN;
	}
}

static void gd32f1x0_i2c_err(uint8_t iic_idx)
{
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;
	reg->STR1 &= ~(I2C_STR1_BE | I2C_STR1_LOSTARB | I2C_STR1_AE);
	gd32f1x0_i2c_cb(iic_idx, VSFERR_FAIL);
}

static void gd32f1x0_i2c_int(uint8_t iic_idx)
{
	struct i2c_ctrl_t *ctrl = &gd32f1x0_i2c[iic_idx];
	I2C_TypeDef *reg = gd32f1x0_i2c_param[iic_idx].reg;
	struct interface_i2c_msg_t *msg = &ctrl->msg[ctrl->msg_prt];
	uint16_t status = reg->STR1;

	if (status & I2C_STR1_SBSEND)
	{
		// I2C_STR1_SBSEND is cleared by reading STR1 and writing DTR
		reg->DTR = (ctrl->chip_addr << 1) | ((msg->flag & I2C_READ) ? 1 : 0);
		// can not put in I2C_STR1_ADDSEND, no idea why
		if (msg->flag & I2C_READ)
		{
			gd32f1x0_i2c_chk_recv_next(iic_idx);
		}
	}
	else if (status & I2C_STR1_ADDSEND)
	{
		// I2C_STR1_ADDSEND is cleared by reading STR1 and reading STR2
		status = reg->STR2;
	}
	else if (status & I2C_STR1_TBE)
	{
		// I2C_STR1_TBE is cleared by writing DTR while sending data
		if (ctrl->msg_buf_prt < msg->len)
		{
		send_data:
			reg->DTR = msg->buf[ctrl->msg_buf_prt++];
		}
		else if (++ctrl->msg_prt < ctrl->msg_len)
		{
			ctrl->msg_buf_prt = 0;
			msg = &ctrl->msg[ctrl->msg_prt];
			if (!(msg->flag & I2C_READ) && (msg->flag & I2C_NOSTART))
				goto send_data;
			else
				reg->CTLR1 |= I2C_CTLR1_GENSTA;
		}
		else
		{
			gd32f1x0_i2c_cb(iic_idx, VSFERR_NONE);
		}
	}
	else if (status & I2C_STR1_RBNE)
	{
		msg->buf[ctrl->msg_buf_prt++] = reg->DTR;
		if (ctrl->msg_buf_prt < msg->len)
		{
		recv_data:
			gd32f1x0_i2c_chk_recv_next(iic_idx);
		}
		else if (++ctrl->msg_prt < ctrl->msg_len)
		{
			ctrl->msg_buf_prt = 0;
			msg = &ctrl->msg[ctrl->msg_prt];
			if ((msg->flag & I2C_READ) && (msg->flag & I2C_NOSTART))
				goto recv_data;
			else
				reg->CTLR1 |= I2C_CTLR1_GENSTA;
		}
		else
		{
			gd32f1x0_i2c_cb(iic_idx, VSFERR_NONE);
		}
	}
}

#if IIC_NUM >= 1
ROOTFUNC void I2C1_EV_IRQHandler(void)
{
	gd32f1x0_i2c_int(0);
}
ROOTFUNC void I2C1_ER_IRQHandler(void)
{
	gd32f1x0_i2c_err(0);
}
#endif

#if IIC_NUM >= 2
ROOTFUNC void I2C2_EV_IRQHandler(void)
{
	gd32f1x0_i2c_int(1);
}
ROOTFUNC void I2C2_ER_IRQHandler(void)
{
	gd32f1x0_i2c_err(1);
}
#endif
#endif

