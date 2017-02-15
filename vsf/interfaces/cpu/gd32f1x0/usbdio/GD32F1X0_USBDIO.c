/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       BDM.c                                                     *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    BDM interface implementation file                         *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2011-05-09:     created(by SimonQian)                             *
 **************************************************************************/

#include "app_cfg.h"
#include "app_type.h"
#include "interfaces.h"
#include "compiler.h"

#if IFS_USBDIO_EN
// NOTE PB12 is used for debug if enabled
#define IFS_USBDIO_DBG_PORT			1
#define IFS_USBDIO_DBG_PIN			12

//#define IFS_USBDIO_RX_LOOP
#define IFS_USBDIO_TX_LOOP

// IFS_USBDIO_PORT
// IFS_USBDIO_DP, IFS_USBDIO_DM
static void (*gd32f1x0_usbdio_onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len);

static void usbdio_set_mode(uint8_t in0_out1)
{
	GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (IFS_USBDIO_PORT << 10));
	if (in0_out1)
	{
		// disable EXTI before set to output
		EXTI->IER &= ~(1 << IFS_USBDIO_DP);

		gpio->BOR = (1 << (0 + IFS_USBDIO_DP)) | (1 << (16 + IFS_USBDIO_DM));
		gpio->CTLR = (gpio->CTLR & ~((0x3 << (IFS_USBDIO_DP << 1)) | (0x3 << (IFS_USBDIO_DM << 1)))) |
						((GPIO_OUTPP & 0x3) << (IFS_USBDIO_DP << 1)) |
						((GPIO_OUTPP & 0x3) << (IFS_USBDIO_DM << 1));
	}
	else
	{
		gpio->CTLR = (gpio->CTLR & ~((0x3 << (IFS_USBDIO_DP << 1)) | (0x3 << (IFS_USBDIO_DM << 1)))) |
						((GPIO_INFLOAT & 0x3) << (IFS_USBDIO_DP << 1)) |
						((GPIO_INFLOAT & 0x3) << (IFS_USBDIO_DM << 1));

		// enable EXTI after set to input
		EXTI->IER |= 1 << IFS_USBDIO_DP;
	}
}

vsf_err_t gd32f1x0_usbdio_init(void (*onrx)(enum usbdio_evt_t evt, uint8_t *buf, uint16_t len))
{
	GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (IFS_USBDIO_PORT << 10));
	uint8_t eint_idx = IFS_USBDIO_DP;
	uint8_t port_idx = IFS_USBDIO_PORT;
#if (IFS_USBDIO_DP == 0) || (IFS_USBDIO_DP == 1)
	uint8_t nvic_idx = EXTI0_1_IRQn;
#elif (IFS_USBDIO_DP == 2) || (IFS_USBDIO_DP == 2)
	uint8_t nvic_idx = EXTI2_3_IRQn;
#else
	uint8_t nvic_idx = EXTI4_15_IRQn;
#endif

	gd32f1x0_usbdio_onrx = onrx;
	vsfhal_gpio_init(port_idx);
	gpio->OSPD = (gpio->OSPD & ~((0x3 << (IFS_USBDIO_DP << 1)) | (0x3 << (IFS_USBDIO_DM << 1)))) |
						(0x3 << (IFS_USBDIO_DP << 1)) | (0x3 << (IFS_USBDIO_DM << 1));

#ifdef IFS_USBDIO_DBG_PORT
	vsfhal_gpio_init(IFS_USBDIO_DBG_PORT);
	GPIOB->BOR = 1 << (0 + IFS_USBDIO_DBG_PIN);
	vsfhal_gpio_config_pin(IFS_USBDIO_DBG_PORT, IFS_USBDIO_DBG_PIN, GPIO_OUTPP);
#endif

	EXTI->FTE |= 1 << eint_idx;
	SYSCFG->EXTISS[eint_idx >> 2] &= ~(0xF << ((eint_idx & 3) << 2));
	SYSCFG->EXTISS[eint_idx >> 2] |= port_idx << ((eint_idx & 3) << 2);
	NVIC->ISER[nvic_idx >> 0x05] = 1UL << (nvic_idx & 0x1F);

	usbdio_set_mode(0);
	vsf_leave_critical();
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_usbdio_fini(void)
{
	return VSFERR_NONE;
}

// configure delay parameter for porting
#define VSFSDCD_NOP(n)						do{REPEAT_CODE((n), asm("nop");)}while (0)
// different optimization level will require different delay
// configuration below is adjusted for sysclk 48MHz with
// IAR EWARM 7.60 kickstart high size/speed optimization
// Steps to configure the delay timing
// 1. adjust VSFSDCD_RX_NOP_BEFORE_SAMPLE, make the first sample point locate at the center of the 2nd SYNC bit
// 2. adjust VSFSDCD_RX0_NOP_BIT1 and VSFSDCD_RX0_NOP_BIT0, make the "if (sample1 & (1 << IFS_USBDIO_DP))" balance at both side
// 3. adjust VSFSDCD_RX0_NOP_STUFF_SAMPLE, and force a stuff condition at the corresponding bit, make the stuff sample at the center
// 4. set VSFSDCD_RX0_NOP_END to 0, adjust VSFSDCD_RX0_NOP_NOSTUFF, make the sample point of the next bit at the center
// 5. set VSFSDCD_RX0_NOP_END to 0, adjust VSFSDCD_RX0_NOP_STUFF, force a stuff condition, make the sample point of the next bit at the center
// 6. optimize using VSFSDCD_RX0_NOP_END for the smallest code size
// 7. goto 2. for the next bit until timing is OK for all 8 bits
#ifdef IFS_USBDIO_RX_LOOP
#define VSFSDCD_RX_NOP_BEFORE_SAMPLE()		VSFSDCD_NOP(1)
#define VSFSDCD_RX_NOP_BALANCE_IF()			VSFSDCD_NOP(3)
#define VSFSDCD_RX_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(16)
#define VSFSDCD_RX_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_RX_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX_NOP_END()				VSFSDCD_NOP(1)
#define VSFSDCD_RX_NOP_EVT()				VSFSDCD_NOP(8)
#else
#define VSFSDCD_RX_NOP_BEFORE_SAMPLE()		VSFSDCD_NOP(5)
#define VSFSDCD_RX0_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX0_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX0_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(16)
#define VSFSDCD_RX0_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_RX0_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX0_NOP_END()				VSFSDCD_NOP(14)
#define VSFSDCD_RX1_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX1_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX1_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(17)
#define VSFSDCD_RX1_NOP_STUFF()				VSFSDCD_NOP(11)
#define VSFSDCD_RX1_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX1_NOP_END()				VSFSDCD_NOP(14)
#define VSFSDCD_RX2_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX2_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX2_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(16)
#define VSFSDCD_RX2_NOP_STUFF()				VSFSDCD_NOP(11)
#define VSFSDCD_RX2_NOP_NOSTUFF()			VSFSDCD_NOP(1)
#define VSFSDCD_RX2_NOP_END()				VSFSDCD_NOP(14)
#define VSFSDCD_RX3_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX3_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX3_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(17)
#define VSFSDCD_RX3_NOP_STUFF()				VSFSDCD_NOP(11)
#define VSFSDCD_RX3_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX3_NOP_END()				VSFSDCD_NOP(14)
#define VSFSDCD_RX4_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX4_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX4_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(16)
#define VSFSDCD_RX4_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_RX4_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX4_NOP_END()				VSFSDCD_NOP(13)
#define VSFSDCD_RX5_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX5_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX5_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(16)
#define VSFSDCD_RX5_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_RX5_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX5_NOP_END()				VSFSDCD_NOP(13)
#define VSFSDCD_RX6_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX6_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX6_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(16)
#define VSFSDCD_RX6_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_RX6_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX6_NOP_END()				VSFSDCD_NOP(13)
#define VSFSDCD_RX7_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_RX7_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_RX7_NOP_STUFF_SAMPLE()		VSFSDCD_NOP(17)
#define VSFSDCD_RX7_NOP_STUFF()				VSFSDCD_NOP(11)
#define VSFSDCD_RX7_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_RX7_NOP_END()				VSFSDCD_NOP(8)
#define VSFSDCD_RX_NOP_EVT()				VSFSDCD_NOP(8)
#endif

// configure the delay timing of tx is similar to rx
#ifdef IFS_USBDIO_TX_LOOP
#define VSFSDCD_TX_NOP_BEFORE_SAMPLE()		VSFSDCD_NOP(0)
#define VSFSDCD_TX_NOP_BALANCE_IF()			VSFSDCD_NOP(3)
#define VSFSDCD_TX_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX_NOP_STUFF_SEND()			VSFSDCD_NOP(23)
#define VSFSDCD_TX_NOP_STUFF()				VSFSDCD_NOP(2)
#define VSFSDCD_TX_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX_NOP_END()				VSFSDCD_NOP(5)
#define VSFSDCD_TX_NOP_BEFORE_SE0()			VSFSDCD_NOP(8)
#define VSFSDCD_TX_NOP_SE0()				VSFSDCD_NOP(59)
#define VSFSDCD_TX_NOP_AFTER_SE0()			VSFSDCD_NOP(28)
#else
#define VSFSDCD_TX_NOP_BEFORE_SAMPLE()		VSFSDCD_NOP(0)
#define VSFSDCD_TX0_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX0_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX0_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX0_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX0_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX0_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX1_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX1_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX1_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX1_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX1_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX1_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX2_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX2_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX2_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX2_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX2_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX2_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX3_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX3_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX3_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX3_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX3_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX3_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX4_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX4_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX4_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX4_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX4_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX4_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX5_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX5_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX5_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX5_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX5_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX5_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX6_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX6_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX6_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX6_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX6_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX6_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX7_NOP_BIT1()				VSFSDCD_NOP(0)
#define VSFSDCD_TX7_NOP_BIT0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX7_NOP_STUFF_SEND()		VSFSDCD_NOP(22)
#define VSFSDCD_TX7_NOP_STUFF()				VSFSDCD_NOP(12)
#define VSFSDCD_TX7_NOP_NOSTUFF()			VSFSDCD_NOP(2)
#define VSFSDCD_TX7_NOP_END()				VSFSDCD_NOP(16)
#define VSFSDCD_TX_NOP_BEFORE_SE0()			VSFSDCD_NOP(0)
#define VSFSDCD_TX_NOP_SE0()				VSFSDCD_NOP(0)
#define VSFSDCD_TX_NOP_AFTER_SE0()			VSFSDCD_NOP(0)
#endif

// DP fall edge interrupt may be served by asm(not C)
// gd32f1x0_usbdio_rx is the non-timing-critical part of the handler
void gd32f1x0_usbdio_rx(uint8_t *buff, uint8_t len)
{
	uint32_t *input = (uint32_t *)&GPIOA->DIR + (IFS_USBDIO_PORT << 10);
	uint32_t *debug_reg = (uint32_t *)&GPIOA->BOR + (IFS_USBDIO_DBG_PORT << 8);
	uint32_t debug_value = 1 << (16 + IFS_USBDIO_DBG_PIN);
	enum usbdio_evt_t evt;

	if (!len)
	{
		// for SOF, the sample point MUST be j
		// for RST, MUST be k
		VSFSDCD_RX_NOP_EVT();
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (*input & (1 << IFS_USBDIO_DP))
			evt = USBDIO_EVT_SOF;
		else
			evt = USBDIO_EVT_RST;
	}
	else
		evt = USBDIO_EVT_DAT;

	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	if (gd32f1x0_usbdio_onrx != NULL)
		gd32f1x0_usbdio_onrx(evt, buff, len);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);

	*debug_reg = 1 << (0 + IFS_USBDIO_DBG_PIN);
	EXTI->PD |= (1 << IFS_USBDIO_DP);
}

#ifdef IFS_USBDIO_RX_LOOP
static const uint8_t usbdio_stuffmask[] = {0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3F, 0x7E, 0xFC};
#endif
#pragma optimize=speed
#if (IFS_USBDIO_DP == 0) || (IFS_USBDIO_DP == 1)
ROOTFUNC void EXTI0_1_IRQHandler(void)
#elif (IFS_USBDIO_DP == 2) || (IFS_USBDIO_DP == 3)
ROOTFUNC void EXTI2_3_IRQHandler(void)
#else
ROOTFUNC void EXTI4_15_IRQHandler(void)
#endif
{
	uint8_t buff[16], *ptr = &buff[0], *end = &buff[16];
	uint32_t *input = (uint32_t *)&GPIOA->DIR + (IFS_USBDIO_PORT << 10);
	uint32_t *debug_reg = (uint32_t *)&GPIOA->BOR + (IFS_USBDIO_DBG_PORT << 8);
	uint32_t debug_value = 1 << (16 + IFS_USBDIO_DBG_PIN);
	uint32_t mask = ((1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM));
	uint8_t byte = 0xFF, stuff = 0xFF;
	uint32_t sample0 = (1 << IFS_USBDIO_DM), sample1;

	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	// some delay to center sample point to the second bit of SYNC
	VSFSDCD_RX_NOP_BEFORE_SAMPLE();

#ifdef IFS_USBDIO_RX_LOOP
	uint8_t bitpos = 1, bitmask = 1 << bitpos;
	while (ptr < end)
	{
		sample1 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample1)
			break;
		sample0 ^= sample1;
		if (sample0 & (1 << IFS_USBDIO_DP))
		{
			byte |= bitmask;
			VSFSDCD_RX_NOP_BIT1();
		}
		else
		{
			byte &= ~bitmask;
			VSFSDCD_RX_NOP_BIT0();
		}
		sample0 = sample1;
		if (!(byte & usbdio_stuffmask[bitpos]))
		{
			stuff &= ~bitmask;
			byte |= bitmask;
			VSFSDCD_RX_NOP_STUFF_SAMPLE();
			sample1 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX_NOP_NOSTUFF();
		}
		VSFSDCD_RX_NOP_END();
		bitpos++;
		bitpos &= 7;
		if (!bitpos)
		{
			*ptr++ = byte ^ stuff;
			stuff = 0xFF;
		}
		else
			VSFSDCD_RX_NOP_BALANCE_IF();
	}
#else
	goto start_sample;
	while (ptr < end)
	{
		// bit0
		sample0 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample0)
			break;
		sample1 ^= sample0;
		if (sample1 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x01;
			VSFSDCD_RX0_NOP_BIT1();
		}
		else
		{
			byte &= ~0x01;
			VSFSDCD_RX0_NOP_BIT0();
		}
		if (!(byte & 0xF9))
		{
			stuff &= ~0x01;
			byte |= 0x01;
			VSFSDCD_RX0_NOP_STUFF_SAMPLE();
			sample0 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX0_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX0_NOP_NOSTUFF();
		}
		VSFSDCD_RX0_NOP_END();

	start_sample:
		// bit1
		sample1 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample1)
			break;
		sample0 ^= sample1;
		if (sample0 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x02;
			VSFSDCD_RX1_NOP_BIT1();
		}
		else
		{
			byte &= ~0x02;
			VSFSDCD_RX1_NOP_BIT0();
		}
		if (!(byte & 0xF3))
		{
			stuff &= ~0x02;
			byte |= 0x02;
			VSFSDCD_RX1_NOP_STUFF_SAMPLE();
			sample1 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX1_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX1_NOP_NOSTUFF();
		}
		VSFSDCD_RX1_NOP_END();

		// bit2
		sample0 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample0)
			break;
		sample1 ^= sample0;
		if (sample1 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x04;
			VSFSDCD_RX2_NOP_BIT1();
		}
		else
		{
			byte &= ~0x04;
			VSFSDCD_RX2_NOP_BIT0();
		}
		if (!(byte & 0xE7))
		{
			stuff &= ~0x04;
			byte |= 0x04;
			VSFSDCD_RX2_NOP_STUFF_SAMPLE();
			sample0 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX2_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX2_NOP_NOSTUFF();
		}
		VSFSDCD_RX2_NOP_END();

		// bit3
		sample1 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample1)
			break;
		sample0 ^= sample1;
		if (sample0 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x08;
			VSFSDCD_RX3_NOP_BIT1();
		}
		else
		{
			byte &= ~0x08;
			VSFSDCD_RX3_NOP_BIT0();
		}
		if (!(byte & 0xCF))
		{
			stuff &= ~0x08;
			byte |= 0x08;
			VSFSDCD_RX3_NOP_STUFF_SAMPLE();
			sample1 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX3_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX3_NOP_NOSTUFF();
		}
		VSFSDCD_RX3_NOP_END();

		// bit4
		sample0 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample0)
			break;
		sample1 ^= sample0;
		if (sample1 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x10;
			VSFSDCD_RX4_NOP_BIT1();
		}
		else
		{
			byte &= ~0x10;
			VSFSDCD_RX4_NOP_BIT0();
		}
		if (!(byte & 0x9F))
		{
			stuff &= ~0x10;
			byte |= 0x10;
			VSFSDCD_RX4_NOP_STUFF_SAMPLE();
			sample0 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX4_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX4_NOP_NOSTUFF();
		}
		VSFSDCD_RX4_NOP_END();

		// bit5
		sample1 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample1)
			break;
		sample0 ^= sample1;
		if (sample0 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x20;
			VSFSDCD_RX5_NOP_BIT1();
		}
		else
		{
			byte &= ~0x20;
			VSFSDCD_RX5_NOP_BIT0();
		}
		if (!(byte & 0x3F))
		{
			stuff &= ~0x20;
			byte |= 0x20;
			VSFSDCD_RX5_NOP_STUFF_SAMPLE();
			sample1 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX5_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX5_NOP_NOSTUFF();
		}
		VSFSDCD_RX5_NOP_END();

		// bit6
		sample0 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample0)
			break;
		sample1 ^= sample0;
		if (sample1 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x40;
			VSFSDCD_RX6_NOP_BIT1();
		}
		else
		{
			byte &= ~0x40;
			VSFSDCD_RX6_NOP_BIT0();
		}
		if (!(byte & 0x7E))
		{
			stuff &= ~0x40;
			byte |= 0x40;
			VSFSDCD_RX6_NOP_STUFF_SAMPLE();
			sample0 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX6_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX6_NOP_NOSTUFF();
		}
		VSFSDCD_RX6_NOP_END();

		// bit7
		sample1 = *input & mask;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (!sample1)
			break;
		sample0 ^= sample1;
		if (sample0 & (1 << IFS_USBDIO_DP))
		{
			byte |= 0x80;
			VSFSDCD_RX7_NOP_BIT1();
		}
		else
		{
			byte &= ~0x80;
			VSFSDCD_RX7_NOP_BIT0();
		}
		if (!(byte & 0xFC))
		{
			stuff &= ~0x80;
			byte |= 0x80;
			VSFSDCD_RX7_NOP_STUFF_SAMPLE();
			sample1 = *input;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_RX7_NOP_STUFF();
		}
		else
		{
			VSFSDCD_RX7_NOP_NOSTUFF();
		}
		VSFSDCD_RX7_NOP_END();

		*ptr++ = byte ^ stuff;
		stuff = 0xFF;
	}
#endif

	uint8_t len = ptr - buff;
#if 0
	buff[0] &= ~0x01;
	gd32f1x0_usbdio_rx(buff, len);
#else
	// rx ready, callback to dcd
	enum usbdio_evt_t evt;
	if (!len)
	{
		// for SOF, the sample point MUST be j
		// for RST, MUST be k
		VSFSDCD_RX_NOP_EVT();
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (*input & (1 << IFS_USBDIO_DP))
			evt = USBDIO_EVT_SOF;
		else
			evt = USBDIO_EVT_RST;
	}
	else
	{
		buff[0] &= ~0x01;
		evt = USBDIO_EVT_DAT;
	}

	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	if (gd32f1x0_usbdio_onrx != NULL)
		gd32f1x0_usbdio_onrx(evt, buff, len);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);

	*debug_reg = 1 << (0 + IFS_USBDIO_DBG_PIN);
	EXTI->PD |= (1 << IFS_USBDIO_DP);
#endif
}

#pragma optimize=speed
vsf_err_t gd32f1x0_usbdio_tx(uint8_t *txbuf, uint16_t len)
{
	uint8_t *end = txbuf + len;
	uint8_t byte;
	uint8_t stuffing = 0;
	uint32_t *output = (uint32_t *)(&(GPIOA->DOR)) + (IFS_USBDIO_PORT << 10);
	uint32_t level = 1 << IFS_USBDIO_DP;
	uint32_t mask = ((1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM));
	uint32_t orig = *output & ~mask;
	uint32_t *debug_reg = (uint32_t *)&GPIOA->BOR + (IFS_USBDIO_DBG_PORT << 8);
	uint32_t debug_value = 1 << (16 + IFS_USBDIO_DBG_PIN);

	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);

	usbdio_set_mode(1);
	// some delay to make sure at lease 2 bit gap
	VSFSDCD_TX_NOP_BEFORE_SAMPLE();

#ifdef IFS_USBDIO_TX_LOOP
	uint8_t bitpos = 0;
	byte = *txbuf++;
	while (txbuf <= end)
	{
		if (byte & (1 << bitpos))
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX_NOP_NOSTUFF();
		}
		VSFSDCD_TX_NOP_END();

		bitpos++;
		bitpos &= 7;
		if (!bitpos)
			byte = *txbuf++;
		else
			VSFSDCD_TX_NOP_BALANCE_IF();
	}
#else
	while (txbuf < end)
	{
		byte = *txbuf++;

		// bit0
		if (byte & 0x01)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX0_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX0_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX0_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX0_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX0_NOP_NOSTUFF();
		}
		VSFSDCD_TX0_NOP_END();

		// bit1
		if (byte & 0x02)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX1_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX1_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX1_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX1_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX1_NOP_NOSTUFF();
		}
		VSFSDCD_TX1_NOP_END();

		// bit2
		if (byte & 0x04)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX2_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX2_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX2_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX2_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX2_NOP_NOSTUFF();
		}
		VSFSDCD_TX2_NOP_END();

		// bit3
		if (byte & 0x08)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX3_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX3_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX3_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX3_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX3_NOP_NOSTUFF();
		}
		VSFSDCD_TX3_NOP_END();

		// bit4
		if (byte & 0x10)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX4_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX4_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX4_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX4_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX4_NOP_NOSTUFF();
		}
		VSFSDCD_TX4_NOP_END();

		// bit5
		if (byte & 0x20)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX5_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX5_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX5_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX5_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX5_NOP_NOSTUFF();
		}
		VSFSDCD_TX5_NOP_END();

		// bit6
		if (byte & 0x40)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX6_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX6_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX6_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX6_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX6_NOP_NOSTUFF();
		}
		VSFSDCD_TX6_NOP_END();

		// bit7
		if (byte & 0x80)
		{
			level &= mask;
			stuffing++;
			VSFSDCD_TX7_NOP_BIT1();
		}
		else
		{
			level ^= mask;
			stuffing = 0;
			VSFSDCD_TX7_NOP_BIT0();
		}
		*output = orig | level;
		*debug_reg = debug_value;
		asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
		if (stuffing >= 6)
		{
			stuffing = 0;
			level ^= mask;
			VSFSDCD_TX7_NOP_STUFF_SEND();
			*output = orig | level;
			*debug_reg = debug_value;
			asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
			VSFSDCD_TX7_NOP_STUFF();
		}
		else
		{
			VSFSDCD_TX7_NOP_NOSTUFF();
		}
		VSFSDCD_TX7_NOP_END();
	}
#endif

	VSFSDCD_TX_NOP_BEFORE_SE0();
	*output = orig;
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	VSFSDCD_TX_NOP_SE0();
	*output = orig | (1 << IFS_USBDIO_DP);
	*debug_reg = debug_value;
	asm("ROR	%[rd], %[rs], #16" : [rd]"=r"(debug_value) : [rs]"r"(debug_value) :);
	VSFSDCD_TX_NOP_AFTER_SE0();

	usbdio_set_mode(0);
	return VSFERR_NONE;
}

#endif // IFS_USBDIO_EN

