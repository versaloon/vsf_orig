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
#include "vsfhal.h"

#if VSFHAL_I2C_EN

#include "NUC505Series.h"
#include "core.h"

#define NUC505_I2C_NUM				2

#define I2C_CON_I2C_STS		I2C_CTL_SI_Msk
#define I2C_CON_START		I2C_CTL_STA_Msk
#define I2C_CON_STOP		I2C_CTL_STO_Msk
#define I2C_CON_ACK			I2C_CTL_AA_Msk
#define I2C_SET_CONTROL_REG(i2c, CTL_Msk) ( (i2c)->CTL = ((i2c)->CTL & ~0x3cul) | (CTL_Msk) )

static void *nuc505_i2c_param[NUC505_I2C_NUM];
static void (*nuc505_i2c_callback[NUC505_I2C_NUM])(void *, vsf_err_t);

struct i2c_ctrl_t
{
	uint16_t enable : 1;
	uint16_t :15;
	uint16_t chip_addr;
	uint16_t msg_len;
	uint16_t msg_prt;
	uint16_t msg_buf_prt;
	struct vsfi2c_msg_t *msg;
} static i2c_ctrl[NUC505_I2C_NUM];

vsf_err_t nuc505_i2c_init(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= NUC505_I2C_NUM)
		return VSFERR_NOT_SUPPORT;
#endif

	switch (index)
	{
	#if I2C0_ENABLE
	case 0:
		#if I2C00_SCL_ENABLE
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA14MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA14MFP_Pos;
		#endif
		#if I2C00_SDA_ENABLE
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA14MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA14MFP_Pos;
		#endif
		#if I2C10_SCL_ENABLE
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk);
		SYS->GPB_MFPL |= 2 << SYS_GPB_MFPL_PB0MFP_Pos;
		#endif
		#if I2C10_SDA_ENABLE
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB1MFP_Msk);
		SYS->GPB_MFPL |= 2 << SYS_GPB_MFPL_PB1MFP_Pos;
		#endif
		#if I2C20_SCL_ENABLE
		SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD0MFP_Msk);
		SYS->GPD_MFPL |= 2 << SYS_GPD_MFPL_PD0MFP_Pos;
		#endif
		#if I2C20_SDA_ENABLE
		SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD1MFP_Msk);
		SYS->GPD_MFPL |= 2 << SYS_GPD_MFPL_PD1MFP_Pos;
		#endif
		CLK->APBCLK |= CLK_APBCLK_I2C0CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_I2C0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_I2C0RST_Msk;
		break:
	#endif // I2C0_ENABLE
	#if I2C1_ENABLE
	case 1:
		#if I2C01_SCL_ENABLE
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA10MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA10MFP_Pos;
		#endif
		#if I2C01_SDA_ENABLE
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA11MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA11MFP_Pos;
		#endif
		#if I2C11_SCL_ENABLE
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB10MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB10MFP_Pos;
		#endif
		#if I2C11_SDA_ENABLE
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB11MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB11MFP_Pos;
		#endif
		#if I2C21_SCL_ENABLE
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB14MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB14MFP_Pos;
		#endif
		#if I2C21_SDA_ENABLE
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB15MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB15MFP_Pos;
		#endif
		CLK->APBCLK |= CLK_APBCLK_I2C1CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_I2C1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_I2C1RST_Msk;
		break:
	#endif // I2C1_ENABLE
	default:
		return VSFERR_FAIL;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_i2c_fini(uint8_t index)
{
#if __VSF_DEBUG__
	if (index >= NUC505_I2C_NUM)
		return VSFERR_NOT_SUPPORT;
#endif

	if (index == 0)
	{
		I2C0->CTL = 0;
		CLK->APBCLK &= ~CLK_APBCLK_I2C0CKEN_Msk;
	}
	else if (index == 1)
	{
		I2C1->CTL = 0;
		CLK->APBCLK &= ~CLK_APBCLK_I2C1CKEN_Msk;
	}
	return VSFERR_NONE;
}

vsf_err_t nuc505_i2c_config(uint8_t index, uint16_t kHz, void *param,
							void (*callback)(void *, int32_t, int32_t))
{
	vsf_err_t err;
	I2C_T *i2c;
	struct nuc505_info_t *info;
	uint32_t div;

#if __VSF_DEBUG__
	if (index >= NUC505_I2C_NUM)
		return VSFERR_NOT_SUPPORT;
#endif

	if (index == 0)
	{
		i2c = I2C0;
	}
	else if (index == 1)
	{
		i2c = I2C1;
	}

	err = nuc505_get_info(&info);
	if (err != VSFERR_NONE)
	{
		return VSFERR_FAIL;
	}

	nuc505_i2c_param[index] = param;
	nuc505_i2c_callback[index] = callback;

	div = info->pclk_freq_hz / (kHz * 1000 * 4) - 1;
	if (div < 4)
		div = 4;
	else if (div > 255)
		div = 255;
	i2c->CLKDIV = div;
	i2c->CTL = I2C_CTL_INTEN_Msk | I2C_CTL_I2CEN_Msk;

	if (index == 0)
	{
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else if (index == 1)
	{
		NVIC_EnableIRQ(I2C1_IRQn);
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_i2c_xfer(uint8_t index, uint16_t chip_addr,
							struct vsfi2c_msg_t *msg, uint16_t msg_len)
{
#if __VSF_DEBUG__
	if (index >= NUC505_I2C_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	if ((msg == NULL) || (msg_len == 0))
		return VSFERR_INVALID_PARAMETER;

	i2c_ctrl[index].chip_addr = chip_addr;
	i2c_ctrl[index].msg = msg;
	i2c_ctrl[index].msg_len = msg_len;
	i2c_ctrl[index].msg_prt = 0;
	i2c_ctrl[index].msg_buf_prt = 0;
	i2c_ctrl[index].enable = 1;

	if (index == 0)
	{
		I2C0->TOCTL = I2C_TOCTL_TOIF_Msk;
		I2C0->TOCTL = I2C_TOCTL_TOCEN_Msk;
		I2C_SET_CONTROL_REG(I2C0, I2C_CON_START);
	}
	else if (index == 1)
	{
		I2C1->TOCTL = I2C_TOCTL_TOIF_Msk;
		I2C1->TOCTL = I2C_TOCTL_TOCEN_Msk;
		I2C_SET_CONTROL_REG(I2C1, I2C_CON_START);
	}

	return VSFERR_NONE;
}

#if I2C0_ENABLE
void I2C0_IRQHandler(void)
{
	if (i2c_ctrl[0].enable == 0)
		goto end;

	if (I2C0->TOCTL & I2C_TOCTL_TOIF_Msk)
	{
		goto error;
	}
	else if (I2C0->CTL & I2C_CON_I2C_STS)
	{
		uint32_t status = I2C0->STATUS;
		struct vsfi2c_msg_t *msg = &i2c_ctrl[0].msg[i2c_ctrl[0].msg_prt];

		if (msg->flag & VSFI2C_READ)
		{
			if ((status == 0x08) || (status == 0x10))
			{
				I2C0->DAT = (i2c_ctrl[0].chip_addr << 1) + 1;
				I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS);
			}
			else if (status == 0x40)
			{
				if (msg->len > 1)
				{
					// host reply ack
					I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS | I2C_CON_ACK);
				}
				else if (msg->len == 1)
				{
					// host reply nack
					I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS);
				}
				else
				{
					goto error;
				}
			}
			else if (status == 0x50)
			{
				if (i2c_ctrl[0].msg_buf_prt < msg->len)
					msg->buf[i2c_ctrl[0].msg_buf_prt++] = I2C0->DAT;
				if (i2c_ctrl[0].msg_buf_prt < msg->len - 1)
				{
					// host reply ack
					I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS | I2C_CON_ACK);
				}
				else
				{
					// host reply nack
					I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS);
				}
			}
			else if (status == 0x58)
			{
				if (i2c_ctrl[0].msg_buf_prt < msg->len)
					msg->buf[i2c_ctrl[0].msg_buf_prt++] = I2C0->DAT;

				if (++i2c_ctrl[0].msg_prt < i2c_ctrl[0].msg_len)
				{
					i2c_ctrl[0].msg_buf_prt = 0;
					I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS | I2C_CON_START);
				}
				else
				{
					I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS | I2C_CON_STOP);
					I2C0->TOCTL = I2C_TOCTL_TOIF_Msk;
					if (nuc505_i2c_callback[0] != NULL)
					{
						i2c_ctrl[0].enable = 0;
						nuc505_i2c_callback[0](nuc505_i2c_param[0], VSFERR_NONE);
					}
				}
			}
			else
			{
				goto error;
			}
		}
		else
		{
			if ((status == 0x08) || (status == 0x10))	// start send finish
			{
				I2C0->DAT = i2c_ctrl[0].chip_addr << 1;
				I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS);
			}
			else if ((status == 0x18) || (status == 0x28))	// addr/data send finish and ACK received
			{
				if (i2c_ctrl[0].msg_buf_prt < msg->len)
				{
					I2C0->DAT = msg->buf[i2c_ctrl[0].msg_buf_prt++];
					I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS);
				}
				else
				{
					if (++i2c_ctrl[0].msg_prt < i2c_ctrl[0].msg_len)
					{
						i2c_ctrl[0].msg_buf_prt = 0;
						I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS | I2C_CON_START);
					}
					else
					{
						I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS | I2C_CON_STOP);
						I2C0->TOCTL = I2C_TOCTL_TOIF_Msk;
						if (nuc505_i2c_callback[0] != NULL)
						{
							i2c_ctrl[0].enable = 0;
							nuc505_i2c_callback[0](nuc505_i2c_param[0], VSFERR_NONE);
						}
					}
				}
			}
			else
			{
				goto error;
			}
		}
	}

error:
	I2C_SET_CONTROL_REG(I2C0, I2C_CON_I2C_STS | I2C_CON_STOP);
	i2c_ctrl[0].enable = 0;
	if (nuc505_i2c_callback[0] != NULL)
	{
		nuc505_i2c_callback[0](nuc505_i2c_param[0], VSFERR_FAIL);
	}

end:
	I2C0->TOCTL = I2C_TOCTL_TOIF_Msk;
}
#endif

#if I2C1_ENABLE
if used, copy from I2C0_IRQHandler
#endif
#endif

