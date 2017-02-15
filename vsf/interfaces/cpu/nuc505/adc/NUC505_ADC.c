#include "app_type.h"
#include "compiler.h"
#include "interfaces.h"

#if IFS_ADC_EN

#include "NUC505Series.h"

#define NUC505_ADC_NUM				1

static void (*nuc505_adc_callback)(void *, uint16_t) = NULL;
static void *nuc505_param = NULL;

vsf_err_t nuc505_adc_init(uint8_t index)
{
	CLK->CLKDIV1 &= ~(CLK_CLKDIV1_ADCSEL_Msk | CLK_CLKDIV1_ADCDIV_Msk);
	//CLK->APBCLK |= CLK_APBCLK_ADCCKEN_Msk;

	NVIC_EnableIRQ(ADC_IRQn);
	return VSFERR_NONE;
}

vsf_err_t nuc505_adc_fini(uint8_t index)
{
	if (index >= NUC505_ADC_NUM)
		return VSFERR_NOT_SUPPORT;

	ADC->CTL |= ADC_CTL_PD_Msk;
	CLK->APBCLK &= ~CLK_APBCLK_ADCCKEN_Msk;

	return VSFERR_NONE;
}

vsf_err_t nuc505_adc_config_channel(uint8_t index, uint8_t channel,
									uint8_t cycles)
{
	if ((index >= NUC505_ADC_NUM) || (channel >= 8))
		return VSFERR_NOT_SUPPORT;

	SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk << 4 * channel);
	SYS->GPA_MFPL |= 0x1ul << 4 * channel;

	return VSFERR_NONE;
}

vsf_err_t nuc505_adc_start(uint8_t index, uint8_t channel, void (*callback)(void *, uint16_t), void *param)
{
	if ((index >= NUC505_ADC_NUM) || (channel >= 8))
		return VSFERR_NOT_SUPPORT;
	if (callback == NULL)
		return VSFERR_FAIL;

	nuc505_adc_callback = callback;
	nuc505_param = param;

	CLK->APBCLK |= CLK_APBCLK_ADCCKEN_Msk;
	ADC->CTL = (60ul << ADC_CTL_EXTSMPT_Pos) |
				(((uint32_t)channel) << ADC_CTL_CHSEL_Pos);
	ADC->INTCTL = ADC_INTCTL_ADCIEN_Msk;
	ADC->CTL |= ADC_CTL_SWTRG_Msk;

	return VSFERR_NONE;
}

ROOTFUNC void ADC_IRQHandler(void)
{
	nuc505_adc_callback(nuc505_param, ADC->DAT);

	ADC->INTCTL = ADC_INTCTL_ADCIF_Msk;
	ADC->CTL = ADC_CTL_CHSEL_Msk | ADC_CTL_PD_Msk;
	CLK->APBCLK &= ~CLK_APBCLK_ADCCKEN_Msk;
}

/*
vsf_err_t nuc505_adc_start(uint8_t index, uint8_t channel);
vsf_err_t nuc505_adc_config(uint8_t index, uint32_t clock_hz, uint32_t mode);
vsf_err_t nuc505_adc_calibrate(uint8_t index, uint8_t channel);
uint32_t nuc505_adc_get_max_value(uint8_t index);
vsf_err_t nuc505_adc_start(uint8_t index, uint8_t channel);
vsf_err_t nuc505_adc_isready(uint8_t index, uint8_t channel);
uint32_t nuc505_adc_get(uint8_t index, uint8_t channel);
*/

#endif

