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

#if VSFHAL_SPI_EN

#include "NUC505Series.h"
#include "core.h"

struct
{
	void (*onready)(void *);
	void *param;
	void *in;
	void *out;
	uint32_t prtrd;
	uint32_t prtwr;
	uint32_t len;
	uint8_t start;
} static nuc505_spi_param[SPI_NUM];

vsf_err_t nuc505_spi_init(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;
	struct nuc505_info_t *info;

#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif
	
	if (nuc505_get_info(&info) || (NULL == info))
	{
		return VSFERR_BUG;
	}

	switch (spi_idx)
	{
	#if SPI0_ENABLE
	case 0:
		// clk, mosi, miso
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB3MFP_Msk);
		SYS->GPB_MFPL |= 1 << SYS_GPB_MFPL_PB3MFP_Pos;
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB4MFP_Msk);
		SYS->GPB_MFPL |= 1 << SYS_GPB_MFPL_PB4MFP_Pos;
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB5MFP_Msk);
		SYS->GPB_MFPL |= 1 << SYS_GPB_MFPL_PB5MFP_Pos;
		if (info->clk_enable & NUC505_CLK_PLL)
		{
			CLK->CLKDIV2 |= CLK_CLKDIV2_SPI0SEL_Msk;
		}
		else
		{
			CLK->CLKDIV2 &= ~CLK_CLKDIV2_SPI0SEL_Msk;
		}
		CLK->APBCLK |= CLK_APBCLK_SPI0CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_SPI0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI0RST_Msk;
		
		NVIC_EnableIRQ(SPI0_IRQn);
		break;
	#endif // SPI0_ENABLE
	#if SPI1_ENABLE
	case 1:
		// clk, mosi, miso
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB11MFP_Msk);
		SYS->GPB_MFPH |= 1 << SYS_GPB_MFPH_PB11MFP_Pos;
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk);
		SYS->GPB_MFPH |= 1 << SYS_GPB_MFPH_PB12MFP_Pos;
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB13MFP_Msk);
		SYS->GPB_MFPH |= 1 << SYS_GPB_MFPH_PB13MFP_Pos;
		if (info->clk_enable & NUC505_CLK_PLL)
		{
			CLK->CLKDIV2 |= CLK_CLKDIV2_SPI0SEL_Msk;
		}
		else
		{
			CLK->CLKDIV2 &= ~CLK_CLKDIV2_SPI0SEL_Msk;
		}
		CLK->APBCLK |= CLK_APBCLK_SPI1CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_SPI1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI1RST_Msk;
		
		NVIC_EnableIRQ(SPI1_IRQn);
		break;
	#endif // SPI1_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_spi_fini(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;

#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	switch (spi_idx)
	{
	#if SPI0_ENABLE
	case 0:
		SYS->IPRST1 |= SYS_IPRST1_SPI0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI0RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_SPI0CKEN_Msk;
		
		NVIC_DisableIRQ(SPI0_IRQn);
		break;
	#endif // SPI0_ENABLE
	#if SPI1_ENABLE
	case 1:
		SYS->IPRST1 |= SYS_IPRST1_SPI1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI1RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_SPI1CKEN_Msk;
		
		NVIC_DisableIRQ(SPI1_IRQn);
		break;
	#endif // SPI1_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_spi_enable(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;

#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	#if SPI0_ENABLE
	if (spi_idx == 0)
	{
		SPI0->CTL |= SPI_CTL_SPIEN_Msk;
	}
	#endif
	#if SPI1_ENABLE
	if (spi_idx == 1)
	{
		SPI1->CTL |= SPI_CTL_SPIEN_Msk;
	}
	#endif

	return VSFERR_NONE;
}

vsf_err_t nuc505_spi_disable(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;

#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	#if SPI0_ENABLE
	if (spi_idx == 0)
	{
		SPI0->CTL &= ~SPI_CTL_SPIEN_Msk;
	}
	#endif
	#if SPI1_ENABLE
	if (spi_idx == 1)
	{
		SPI1->CTL &= ~SPI_CTL_SPIEN_Msk;
	}
	#endif

	return VSFERR_NONE;
}

vsf_err_t nuc505_spi_get_ability(uint8_t index, struct spi_ability_t *ability)
{
	struct nuc505_info_t *info;
	uint32_t freq;

#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	if (nuc505_get_info(&info) || (NULL == info))
	{
		return VSFERR_BUG;
	}

	if (info->clk_enable & NUC505_CLK_PLL)
	{
		freq = info->pll_freq_hz;
	}
	else
	{
		freq = info->osc_freq_hz;
	}

	ability->max_freq_hz = ability->min_freq_hz = freq;
	ability->min_freq_hz /= 256;

	return VSFERR_NONE;
}

vsf_err_t nuc505_spi_config(uint8_t index, uint32_t kHz, uint32_t mode)
{
	uint8_t spi_idx = index & 0x0F;
	struct nuc505_info_t *info;
	uint32_t freq;

#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	if (nuc505_get_info(&info) || (NULL == info))
	{
		return VSFERR_BUG;
	}

	SPI_T *spi = spi_idx ? SPI1 : SPI0;

	if (info->clk_enable & NUC505_CLK_PLL)
	{
		freq = info->pll_freq_hz;
	}
	else
	{
		freq = info->osc_freq_hz;
	}
	spi->CLKDIV = freq / kHz / 1000 - 1;
	spi->CTL = mode |
				(0x1ul << SPI_CTL_SUSPITV_Pos) | 	// suspend interval = 1.5 spiclk
				(0x8ul << SPI_CTL_DWIDTH_Pos) |		// transmit bit width = 8
				//SPI_CTL_UNITIEN_Msk|
				SPI_CTL_SPIEN_Msk;
	//close SPI wait transfer start
	
	//config thr int
	spi->FIFOCTL = 0;

	return VSFERR_NONE;
}

vsf_err_t nuc505_spi_config_cb(uint8_t index, uint32_t int_priority,
										void *p, void (*onready)(void *))
{
	uint8_t spi_idx = index & 0x0F;

#if __VSF_DEBUG__
	if (spi_idx >= SPI_NUM)
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	nuc505_spi_param[spi_idx].onready = onready;
	nuc505_spi_param[spi_idx].param = p;

	return VSFERR_NONE;
}

vsf_err_t nuc505_spi_select(uint8_t index, uint8_t cs)
{
	return VSFERR_NONE;
}
vsf_err_t nuc505_spi_deselect(uint8_t index, uint8_t cs)
{
	return VSFERR_NONE;
}


static uint8_t nuc505_spi_dummy;
void spi_irq(SPI_T *spi, int i);
vsf_err_t nuc505_spi_start(uint8_t index, uint8_t *out, uint8_t *in,
							uint32_t len)
{
	uint8_t spi_idx = index & 0x0F;

#if __VSF_DEBUG__
	if ((spi_idx >= SPI_NUM) || (len > 0xFFFF))
	{
		return VSFERR_NOT_SUPPORT;
	}
#endif

	if (nuc505_spi_param[spi_idx].start)
		return VSFERR_NOT_READY;

	nuc505_spi_param[spi_idx].start = 1;
	nuc505_spi_param[spi_idx].out = (out == NULL) ? &nuc505_spi_dummy : out;
	nuc505_spi_param[spi_idx].in = in;
	nuc505_spi_param[spi_idx].prtwr = 0;
	nuc505_spi_param[spi_idx].prtrd = 0;
	nuc505_spi_param[spi_idx].len = len;

	SPI_T *spi = spi_idx ? SPI1 : SPI0;

	if(len)
	{
		spi->FIFOCTL = SPI_FIFOCTL_RXTHIEN_Msk;
		spi_irq(spi, spi_idx);
	}

	return VSFERR_NONE;
}

uint32_t nuc505_spi_stop(uint8_t index)
{
	uint8_t spi_idx = index & 0x0F;

#if __VSF_DEBUG__
	if ((spi_idx >= SPI_NUM) || (len > 0xFFFF))
	{
		return 0;
	}
#endif

	SPI_T *spi = spi_idx ? SPI1 : SPI0;
	spi->FIFOCTL = SPI_FIFOCTL_TXRST_Msk | SPI_FIFOCTL_RXRST_Msk;
	nuc505_spi_param[spi_idx].start = 0;

	return nuc505_spi_param[spi_idx].len - nuc505_spi_param[spi_idx].prtwr;
}

void spi_irq(SPI_T *spi, int i)
{	
	if(nuc505_spi_param[i].start != 1)
	{
		return;
	}

	while(!(spi->STATUS&SPI_STATUS_RXEMPTY_Msk))
	{
		if (nuc505_spi_param[i].in != NULL )
		{
			((uint8_t *)(nuc505_spi_param[i].in))[nuc505_spi_param[i].prtrd] = spi->RX;
		}
		else
		{
			volatile uint32_t dummy = spi->RX;
		}
		nuc505_spi_param[i].prtrd++;
	}

	if(nuc505_spi_param[i].prtrd == nuc505_spi_param[i].len)
	{
		nuc505_spi_param[i].start = 0;
		spi->FIFOCTL = 0;

		if (nuc505_spi_param[i].onready != NULL)
		{
			nuc505_spi_param[i].onready(nuc505_spi_param[i].param);
		}
	}
	else
	{
		uint32_t remain = nuc505_spi_param[i].len - nuc505_spi_param[i].prtwr;

		if(remain > 8)
		{
			remain = 8;
		}

		if (remain > 0)
		{
			spi->FIFOCTL &= ~SPI_FIFOCTL_RXTH_Msk;
			spi->FIFOCTL |= (remain - 1) << SPI_FIFOCTL_RXTH_Pos;
			while (remain > 0)
			{
				if (nuc505_spi_param[i].out != NULL)
				{
					spi->TX = ((uint8_t *)(nuc505_spi_param[i].out))[nuc505_spi_param[i].prtwr];
				}
				else
				{
					spi->TX = 0xFF;
				}
				nuc505_spi_param[i].prtwr++;
				remain--;
			}
		}
	}
}

#if SPI0_ENABLE
ROOTFUNC void SPI0_IRQHandler(void)
{
	spi_irq(SPI0, 0);
}
#endif

#if SPI1_ENABLE
ROOTFUNC void SPI1_IRQHandler(void)
{
	spi_irq(SPI1, 1);
}
#endif

#endif

