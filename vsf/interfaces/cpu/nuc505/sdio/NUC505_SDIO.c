#include "compiler.h"
#include "app_cfg.h"
#include "interfaces.h"

#if IFS_SDIO_EN

#include "NUC505Series.h"
#include "core.h"

static void (*sdio_callback)(void *) = NULL;
static void *sdio_callback_param;
static struct sdio_info_t *sdio_info;
static volatile uint8_t sdio_busy = 0;


vsf_err_t nuc505_sdio_init(uint8_t index)
{
	// IO config
	PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL0_Msk) | (0x2ul << GPIO_PUEN_PULLSEL0_Pos);
	PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL1_Msk) | (0x2ul << GPIO_PUEN_PULLSEL1_Pos);
	PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL2_Msk) | (0x2ul << GPIO_PUEN_PULLSEL2_Pos);
	PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL4_Msk) | (0x1ul << GPIO_PUEN_PULLSEL4_Pos);
	PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL5_Msk) | (0x1ul << GPIO_PUEN_PULLSEL5_Pos);
	PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL6_Msk) | (0x1ul << GPIO_PUEN_PULLSEL6_Pos);
	PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL7_Msk) | (0x1ul << GPIO_PUEN_PULLSEL7_Pos);

	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC0MFP_Msk) |
			(1 << SYS_GPC_MFPL_PC0MFP_Pos);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC1MFP_Msk) |
			(1 << SYS_GPC_MFPL_PC1MFP_Pos);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC4MFP_Msk) |
			(1 << SYS_GPC_MFPL_PC4MFP_Pos);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC5MFP_Msk) |
			(1 << SYS_GPC_MFPL_PC5MFP_Pos);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC6MFP_Msk) |
			(1 << SYS_GPC_MFPL_PC6MFP_Pos);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC7MFP_Msk) |
			(1 << SYS_GPC_MFPL_PC7MFP_Pos);

	CLK->CLKDIV1 &= ~CLK_CLKDIV1_SDHSEL_Msk;
	CLK->CLKDIV1 &= ~CLK_CLKDIV1_SDHDIV_Msk;
	CLK->AHBCLK |= CLK_AHBCLK_SDHCKEN_Msk;

	SYS->IPRST1 |= SYS_IPRST1_SDHRST_Msk;
	SYS->IPRST1 &= ~SYS_IPRST1_SDHRST_Msk;

	SD->DMACTL = SDH_DMACTL_DMARST_Msk;
	while(SD->DMACTL & SDH_DMACTL_DMARST_Msk);
	SD->DMACTL = SDH_DMACTL_DMAEN_Msk;

	SD->GCTL |= SDH_GCTL_GCTLRST_Msk;
	while(SD->GCTL & SDH_GCTL_GCTLRST_Msk);
	SD->GCTL |= SDH_GCTL_SDEN_Msk;

	SD->GINTEN |= SDH_GINTEN_DTAIEN_Msk;
	SD->INTEN |= SDH_INTEN_DITOIEN_Msk | SDH_INTEN_RTOIEN_Msk |
			SDH_INTEN_CRCIEN_Msk | SDH_INTEN_BLKDIEN_Msk;
	NVIC_EnableIRQ(SDH_IRQn);

	sdio_busy = 0;
	return VSFERR_NONE;
}

vsf_err_t nuc505_sdio_fini(uint8_t index)
{
	SD->DMACTL &= ~SDH_DMACTL_DMAEN_Msk;
	SD->GCTL &= ~SDH_GCTL_SDEN_Msk;
	CLK->AHBCLK &= ~CLK_AHBCLK_SDHCKEN_Msk;
	return VSFERR_NONE;
}

vsf_err_t nuc505_sdio_config(uint8_t index, uint32_t kHz, uint8_t buswidth,
				void (*callback)(void *), void *param)
{
	uint32_t div;
	struct nuc505_info_t *info;

	sdio_callback = callback;
	sdio_callback_param = param;

	nuc505_interface_get_info(&info);
	CLK->CLKDIV1 &= ~CLK_CLKDIV1_SDHDIV_Msk;
	if (kHz > 4000)
	{
		if (info->hclk_freq_hz <= kHz * 1000)
			kHz = info->hclk_freq_hz;
		div = info->pll_freq_hz / (kHz * 100);
		div = (div + 5) / 10;

		CLK->CLKDIV1 |= div << CLK_CLKDIV1_SDHDIV_Pos;
		CLK->CLKDIV1 |= CLK_CLKDIV1_SDHSEL_Msk;
	}
	else
	{
		div = info->osc_freq_hz / (kHz * 100);
		div = (div + 5) / 10;

		CLK->CLKDIV1 &= ~CLK_CLKDIV1_SDHSEL_Msk;
		CLK->CLKDIV1 |= div << CLK_CLKDIV1_SDHDIV_Pos;
	}
	CLK->AHBCLK |= CLK_AHBCLK_SDHCKEN_Msk;

	if (buswidth == 1)
	{
		SD->CTL &= ~SDH_CTL_DBW_Msk;
	}
	else
	{
		SD->CTL |= SDH_CTL_DBW_Msk;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_sdio_start(uint8_t index, uint8_t cmd, uint32_t arg,
		struct sdio_info_t *extra_param)
{
	uint32_t ctl;

	if (sdio_busy)
		return VSFERR_NOT_READY;

	sdio_busy = 1;

	SD->CMDARG = arg;
	ctl = SD->CTL & ~(SDH_CTL_CMDCODE_Msk | SDH_CTL_BLKCNT_Msk);
	sdio_info = extra_param;

	if (extra_param == NULL)
	{
		goto noblk;
	}

	if (extra_param->need_resp)
	{
		if (extra_param->long_resp)
			ctl |= SDH_CTL_R2EN_Msk;
		else
			ctl |= SDH_CTL_RIEN_Msk;
	}

	if (extra_param->data_align4 != NULL)
	{
		ctl |= ((uint32_t)cmd << SDH_CTL_CMDCODE_Pos) | SDH_CTL_COEN_Msk |
				((uint32_t)extra_param->block_cnt << SDH_CTL_BLKCNT_Pos);
		ctl |= extra_param->read0_write1 ? SDH_CTL_DOEN_Msk : SDH_CTL_DIEN_Msk;
		SD->BLEN = extra_param->block_len - 1;
		SD->DMASA = (uint32_t)extra_param->data_align4;

		if (extra_param->read0_write1 == 0)
		{
			SD->TOUT = (extra_param->block_len * extra_param->block_cnt) * 2 +
					0x100000;
		}

		SD->CTL = ctl;
		return VSFERR_NONE;
	}

noblk:
	SD->TOUT = 0x400;
	ctl |= ((uint32_t)cmd << SDH_CTL_CMDCODE_Pos) | SDH_CTL_COEN_Msk |
			(1ul << SDH_CTL_BLKCNT_Pos);
	SD->CTL = ctl;
	if (ctl & SDH_CTL_R2EN_Msk)
	{
		while ((SD->CTL & SDH_CTL_R2EN_Msk) && sdio_busy);
	}
	else if (ctl & SDH_CTL_RIEN_Msk)
	{
		while ((SD->CTL & SDH_CTL_RIEN_Msk) && sdio_busy);
	}
	else
	{
		while ((SD->CTL & SDH_CTL_COEN_Msk) && sdio_busy);
	}
	if ((extra_param) && (extra_param->need_resp))
	{
		if (extra_param->long_resp)
		{
			// TODO
		}
		else
		{
			extra_param->resp[0] = SD->RESP1 & 0xfful;
			extra_param->resp[0] |= SD->RESP0 << 8;
			extra_param->resp[1] = SD->RESP0 >> 24;
		}
	}

	if (sdio_busy)
	{
		sdio_busy = 0;
		sdio_callback(sdio_callback_param);
	}
	//else // error
	//{
	//}

	return VSFERR_NONE;
}

vsf_err_t nuc505_sdio_stop(uint8_t index)
{
	SD->CTL |= SDH_CTL_CTLRST_Msk;
	while (SD->CTL & SDH_CTL_CTLRST_Msk);
	SD->DMACTL |= SDH_DMACTL_DMARST_Msk;
	vsf_enter_critical();
	if (sdio_busy)
	{
		if (sdio_info)
			sdio_info->manual_stop = 1;

		sdio_busy = 0;
		sdio_callback(sdio_callback_param);
	}
	vsf_leave_critical();

	return VSFERR_NONE;
}

static void (*d1_int_callback)(void *) = NULL;
static void *d1_int_param = NULL;
vsf_err_t nuc505_sdio_config_int(uint8_t index,  void (*callback)(void *param),
		void *param)
{
	if (callback != NULL)
	{
		d1_int_callback = callback;
		d1_int_param = param;
	}
	else
	{
		SD->INTEN &= ~SDH_INTEN_SDHIEN0_Msk;
		SD->INTSTS = SDH_INTSTS_SDHIF0_Msk;
	}
	
	return VSFERR_NONE;
}

vsf_err_t nuc505_sdio_enable_int(uint8_t index)
{
	if (d1_int_callback != NULL)
	{
		SD->INTSTS = SDH_INTSTS_SDHIF0_Msk;
		SD->INTEN |= SDH_INTEN_SDHIEN0_Msk;
		
		return VSFERR_NONE;
	}
	
	return VSFERR_FAIL;
}

ROOTFUNC void SDH_IRQHandler(void)
{
	uint32_t intsts = SD->INTSTS;

	if (intsts & SDH_INTSTS_SDHIF0_Msk)
	{
		if (d1_int_callback != NULL)
		{
			d1_int_callback(d1_int_param);
		}

		SD->INTEN &= ~SDH_INTEN_SDHIEN0_Msk;
		SD->INTSTS = SDH_INTSTS_SDHIF0_Msk;
	}

	if (intsts & (SDH_INTSTS_DITOIF_Msk | SDH_INTSTS_RTOIF_Msk))
	{
		SD->CTL |= SDH_CTL_CTLRST_Msk;
		while (SD->CTL & SDH_CTL_CTLRST_Msk);
		SD->INTSTS = SDH_INTSTS_DITOIF_Msk | SDH_INTSTS_RTOIF_Msk;
		if (sdio_info)
		{
			sdio_info->overtime_error = 1;
		}
		if (sdio_busy)
		{
			sdio_busy = 0;
			sdio_callback(sdio_callback_param);
		}
	}
	if (intsts & SDH_INTSTS_CRCIF_Msk)
	{
		SD->CTL |= SDH_CTL_CTLRST_Msk;
		while (SD->CTL & SDH_CTL_CTLRST_Msk);
		SD->INTSTS = SDH_INTSTS_CRCIF_Msk;
		if (sdio_info)
		{
			sdio_info->crc7_error = (intsts & SDH_INTSTS_CRC7_Msk) ? 0 : 1;
			sdio_info->crc16_error = (intsts & SDH_INTSTS_CRC16_Msk) ? 0 : 1;

			if (sdio_info->data_align4 == NULL)
			{
				if (sdio_busy)
				{
					sdio_busy = 0;
					sdio_callback(sdio_callback_param);
				}
			}
		}
	}
	if (intsts & SDH_INTSTS_BLKDIF_Msk)
	{
		SD->INTSTS = SDH_INTSTS_BLKDIF_Msk;
		if (sdio_busy)
		{
			sdio_busy = 0;
			sdio_callback(sdio_callback_param);
		}
	}
	SD->TOUT = 0;
}

#endif

