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

#if VSFHAL_PWM_EN

#define GD32F1X0_DEF_TIMER(index, TIMER, APB, CLKMASK)	\
	[index] = {.timer = (TIMER), .apb = (APB), .clkmask = (CLKMASK)}
struct gd32f1x0_pwm_t
{
	TIMER_TypeDef *timer;
	uint8_t apb;
	uint32_t clkmask;
} const gd32f1x0_pwm[17] =
{
	GD32F1X0_DEF_TIMER(0,	TIMER1,		2,	RCC_APB2CCR_TIMER1EN),
	GD32F1X0_DEF_TIMER(1,	TIMER2,		1,	RCC_APB1CCR_TIMER2EN),
	GD32F1X0_DEF_TIMER(2,	TIMER3,		1,	RCC_APB1CCR_TIMER3EN),
	GD32F1X0_DEF_TIMER(5,	TIMER6,		1,	RCC_APB1CCR_TIMER6EN),
	GD32F1X0_DEF_TIMER(13,	TIMER14,	1,	RCC_APB1CCR_TIMER14EN),
	GD32F1X0_DEF_TIMER(14,	TIMER15,	2,	RCC_APB2CCR_TIMER15EN),
	GD32F1X0_DEF_TIMER(15,	TIMER16,	2,	RCC_APB2CCR_TIMER16EN),
	GD32F1X0_DEF_TIMER(16,	TIMER17,	2,	RCC_APB2CCR_TIMER17EN),
};

// index:
//		bit0 - 2	: channel(even: P channel, odd: N channel)
//		bit3 - 7	: timer_index
//		bit8 - 11	: port
//		bit12- 15	: pin
//		bit16- 18	: af

static const struct gd32f1x0_pwm_t* gd32f1x0_pwm_get_timer(uint32_t index)
{
	return &gd32f1x0_pwm[(index >> 3) & 0x1F];
}

vsf_err_t gd32f1x0_pwm_init(uint32_t index)
{
	const struct gd32f1x0_pwm_t *pwm = gd32f1x0_pwm_get_timer(index);
	TIMER_TypeDef *timer = pwm->timer;
	struct gd32f1x0_afio_pin_t afio;

	afio.port = (index >> 8) & 0x0F;
	afio.pin = (index >> 12) & 0x0F;
	afio.af = (index >> 16) & 0x07;
	RCC->AHBCCR |= RCC_AHBCCR_PAEN << afio.port;
	gd32f1x0_afio_config(&afio, gd32f1x0_GPIO_AF);

	switch (pwm->apb)
	{
	case 1: RCC->APB1CCR |= pwm->clkmask; break;
	case 2: RCC->APB2CCR |= pwm->clkmask; break;
	}
	timer->CTLR1 |= TIMER_CTLR1_CNTE;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_pwm_fini(uint32_t index)
{
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_pwm_config_mode(uint32_t index, uint8_t mode)
{
	TIMER_TypeDef *timer = gd32f1x0_pwm_get_timer(index)->timer;
	uint8_t timer_idx = (index >> 3) & 0x1F;
	uint8_t channel = index & 0x07;

	timer->CHE &= ~(0x03 << (channel << 1));
	timer->CHE |= (mode & 0x03) << (channel << 1);
	channel >>= 1;
	// PWM_MODE_1
	*((&timer->CHCTLR1) + (channel >> 1)) =
		(TIMER_CHCTLR1_CH1OM_1 | TIMER_CHCTLR1_CH1OM_2) << ((channel & 1) << 3);
	if ((timer_idx == 0) || (timer_idx == 14) ||
		(timer_idx == 15) || (timer_idx == 16))
		timer->BKDT |= TIMER_BKDT_POE;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_pwm_config_freq(uint32_t index, uint16_t kHz)
{
	const struct gd32f1x0_pwm_t *pwm = gd32f1x0_pwm_get_timer(index);
	TIMER_TypeDef *timer = pwm->timer;
	struct gd32f1x0_info_t *info;
	uint32_t clk;

	gd32f1x0_get_info(&info);
	switch (pwm->apb)
	{
	case 1: clk = info->apb1_freq_hz; break;
	case 2: clk = info->apb2_freq_hz; break;
	}

	clk = (clk / 1000) / kHz;
	timer->CARL = clk;
	timer->EVG = TIMER_EVG_UPG;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_pwm_out(uint32_t index, uint16_t rate)
{
	TIMER_TypeDef *timer = gd32f1x0_pwm_get_timer(index)->timer;
	uint8_t channel = (index & 0x07) >> 1;

	rate = rate * (timer->CARL + 1) / 0xFFFF;
	(&timer->CHCC1)[channel] = rate;
	return VSFERR_NONE;
}

#endif

