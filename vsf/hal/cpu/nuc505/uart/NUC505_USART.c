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

#if VSFHAL_USART_EN

#include "core.h"
#include "NUC505Series.h"

#define UART_IS_RX_READY(uart)		((uart->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) >> UART_FIFOSTS_RXEMPTY_Pos)
#define UART_IS_TX_EMPTY(uart)		((uart->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk) >> UART_FIFOSTS_TXEMPTYF_Pos)
#define UART_IS_TX_FIFO_FULL(uart)	((uart->FIFOSTS & UART_FIFOSTS_TXFULL_Msk) >> UART_FIFOSTS_TXFULL_Pos)

static void (*nuc505_usart_ontx[USART_NUM])(void *);
static void (*nuc505_usart_onrx[USART_NUM])(void *, uint16_t data);
static void *nuc505_usart_callback_param[USART_NUM];

vsf_err_t nuc505_usart_init(uint8_t index)
{
	uint8_t usart_idx = index & 0x0F;
	uint8_t remap_idx = (index >> 4) & 0x0F;

	switch (usart_idx)
	{
	#if USART00_ENABLE
	case 0:
		switch (remap_idx)
		{
		case 0:
			#if USART00_TX_ENABLE
			SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk);
			SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB0MFP_Pos;
			#endif
			#if USART00_RX_ENABLE
			SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB1MFP_Msk);
			SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB1MFP_Pos;
			#endif
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->CLKDIV3 &= ~(CLK_CLKDIV3_UART0SEL_Msk | CLK_CLKDIV3_UART0DIV_Msk);
		CLK->APBCLK |= CLK_APBCLK_UART0CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_UART0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;
		break;
	#endif // USART00_ENABLE
	#if USART01_ENABLE || USART11_ENABLE
	case 1:
		switch (remap_idx)
		{
		#if USART01_ENABLE
		case 0:
			#if USART01_TX_ENABLE
			SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA8MFP_Msk);
			SYS->GPA_MFPH |= 3 << SYS_GPA_MFPH_PA8MFP_Pos;
			#endif
			#if USART01_RX_ENABLE
			SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA9MFP_Msk);
			SYS->GPA_MFPH |= 3 << SYS_GPA_MFPH_PA9MFP_Pos;
			#endif
			break;
		#endif // USART01_ENABLE
		#if USART11_ENABLE
		case 0:
			#if USART11_CTS_ENABLE
			SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk);
			SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB8MFP_Pos;
			#endif
			#if USART11_RTS_ENABLE
			SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk);
			SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB8MFP_Pos;
			#endif
			#if USART11_TX_ENABLE
			SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB6MFP_Msk);
			SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB6MFP_Pos;
			#endif
			#if USART11_RX_ENABLE
			SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB7MFP_Msk);
			SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB7MFP_Msk;
			#endif
			break;
		#endif // USART11_ENABLE
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->CLKDIV3 &= ~(CLK_CLKDIV3_UART1SEL_Msk | CLK_CLKDIV3_UART1DIV_Msk);
		CLK->APBCLK |= CLK_APBCLK_UART1CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_UART1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART1RST_Msk;
		break;
	#endif // USART01_ENABLE || USART11_ENABLE
	#if USART02_ENABLE
	case 2:
		switch (remap_idx)
		{
		case 0:
			#if USART02_CTS_ENABLE
			SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk);
			SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB12MFP_Pos;
			#endif
			#if USART02_RTS_ENABLE
			SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB13MFP_Msk);
			SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB13MFP_Pos;
			#endif
			#if USART02_TX_ENABLE
			SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB10MFP_Msk);
			SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB10MFP_Pos;
			#endif
			#if USART02_RX_ENABLE
			SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB11MFP_Msk);
			SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB11MFP_Pos;
			#endif
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->CLKDIV3 &= ~(CLK_CLKDIV3_UART2SEL_Msk | CLK_CLKDIV3_UART2DIV_Msk);
		CLK->APBCLK |= CLK_APBCLK_UART2CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_UART2RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART2RST_Msk;
		break;
	#endif // USART02_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_usart_fini(uint8_t index)
{
	uint8_t usart_idx = index & 0x0F;

	switch (usart_idx)
	{
	#if USART00_ENABLE
	case 0:
		CLK->APBCLK &= ~CLK_APBCLK_UART0CKEN_Msk;
		break;
	#endif // USART00_ENABLE
	#if USART01_ENABLE || USART11_ENABLE
	case 1:
		CLK->APBCLK &= ~CLK_APBCLK_UART1CKEN_Msk;
		break;
	#endif // USART01_ENABLE || USART11_ENABLE
	#if USART02_ENABLE
	case 2:
		CLK->APBCLK &= ~CLK_APBCLK_UART2CKEN_Msk;
		break;
	#endif // USART02_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode)
{
	UART_T *usart;
	uint32_t usart_idx = index & 0x0F;
	struct nuc505_info_t *info;
	uint32_t baud_div = 0, reg_line = 0;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

/*	switch (datalength)
	{
	case 5:
		reg_line = 0;
		break;
	case 6:
		reg_line = 1;
		break;
	case 7:
		reg_line = 2;
		break;
	case 8:
		reg_line = 3;
		break;
	default:
		return VSFERR_INVALID_PARAMETER;
	}
*/
	// mode:
	// bit0 - bit1: parity
	// ------------------------------------- bit2 - bit3: mode [nothing]
	// bit4       : stopbits
	reg_line |= (mode << 3) & 0x18;	//parity
	reg_line |= (mode >> 2) & 0x04;	//stopbits

	usart->FUNCSEL = 0;
	usart->LINE = reg_line;
	usart->FIFO = 0x5ul << 4; // 46/14 (64/16)
	usart->TOUT = 60;

	if (nuc505_get_info(&info))
	{
		return VSFERR_FAIL;
	}

	if(baudrate != 0)
	{
		int32_t error;
		baud_div = info->osc_freq_hz / baudrate;
		if ((baud_div < 11) || (baud_div > (0xffff + 2)))
			return VSFERR_INVALID_PARAMETER;
		error = (info->osc_freq_hz / baud_div) * 1000 / baudrate;
		error -= 1000;
		if ((error > 20) || ((error < -20)))
			return VSFERR_INVALID_PARAMETER;
		if (info->osc_freq_hz * 1000 / baud_div / baudrate)
		usart->BAUD = UART_BAUD_BAUDM0_Msk | UART_BAUD_BAUDM1_Msk |
						(baud_div - 2);
	}


	switch (usart_idx)
	{
	#if USART0_INT_EN
	case 0:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART0_IRQn);
		break;
	#endif
	#if USART1_INT_EN
	case 1:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART1_IRQn);
		break;
	#endif
	#if USART2_INT_EN
	case 2:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART2_IRQn);
		break;
	#endif
	default:
		break;
	}

	return VSFERR_NONE;
}

vsf_err_t nuc505_usart_config_cb(uint8_t index, uint32_t int_priority,
				void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t))
{
	uint32_t usart_idx = index & 0x0F;

	nuc505_usart_ontx[usart_idx] = ontx;
	nuc505_usart_onrx[usart_idx] = onrx;
	nuc505_usart_callback_param[usart_idx] = p;

	return VSFERR_NONE;
}

vsf_err_t nuc505_usart_tx(uint8_t index, uint16_t data)
{
	UART_T *usart;
	uint32_t usart_idx = index & 0x0F;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	usart->DAT = (uint8_t)data;
	usart->INTEN |= UART_INTEN_THREIEN_Msk;

	return VSFERR_NONE;
}

uint16_t nuc505_usart_rx(uint8_t index)
{
	UART_T *usart;
	uint32_t usart_idx = index & 0x0F;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	return usart->DAT;
}

uint16_t nuc505_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size)
{
	UART_T *usart;
	uint32_t usart_idx = index & 0x0F;
	uint16_t i;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	for (i = 0; i < size; i++)
	{
		usart->DAT = data[i];
	}
	usart->INTEN |= UART_INTEN_THREIEN_Msk;

	return 0;
}

uint16_t nuc505_usart_tx_get_free_size(uint8_t index)
{
	UART_T *usart;
	uint32_t usart_idx = index & 0x0F, fifo_len;

	fifo_len = usart_idx ? 64 : 16;
	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	if (usart->FIFOSTS & UART_FIFOSTS_TXFULL_Msk)
	{
		return 0;
	}
	else
	{
		return fifo_len - ((usart->FIFOSTS & UART_FIFOSTS_TXPTR_Msk) >>
							UART_FIFOSTS_TXPTR_Pos);
	}
}

uint16_t nuc505_usart_rx_bytes(uint8_t index, uint8_t *data, uint16_t size)
{
	UART_T *usart;
	uint32_t usart_idx = index & 0x0F;
	uint16_t i;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	for (i = 0; i < size; i++)
	{
		if (usart->FIFOSTS & (UART_FIFOSTS_RXFULL_Msk | UART_FIFOSTS_RXPTR_Msk))
		{
			data[i] = usart->DAT;
		}
		else
		{
			break;
		}
	}
	return i;
}

uint16_t nuc505_usart_rx_get_data_size(uint8_t index)
{
	UART_T *usart;
	uint32_t usart_idx = index & 0x0F, fifo_len;

	fifo_len = usart_idx ? 64 : 16;
	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	if (usart->FIFOSTS & UART_FIFOSTS_RXFULL_Msk)
	{
		return fifo_len;
	}
	else
	{
		return (usart->FIFOSTS & UART_FIFOSTS_RXPTR_Msk) >>
				UART_FIFOSTS_RXPTR_Pos;
	}
}

#if USART0_INT_EN && USART00_ENABLE
ROOTFUNC void UART0_IRQHandler(void)
{
	if (UART0->INTSTS & UART_INTSTS_RDAIF_Msk)
	{
		nuc505_usart_onrx[0](nuc505_usart_callback_param[0], UART0->DAT);
	}
	else if (UART0->INTSTS & UART_INTSTS_RXTOINT_Msk)
	{
		nuc505_usart_onrx[0](nuc505_usart_callback_param[0], UART0->DAT);
	}
	if (UART0->INTSTS & UART_INTSTS_THREINT_Msk)
	{
		UART0->INTEN &= (~UART_INTEN_THREIEN_Msk);
		nuc505_usart_ontx[0](nuc505_usart_callback_param[0]);
	}
}
#endif // USART0_INT_EN && USART00_ENABLE

#if USART1_INT_EN && (USART01_ENABLE || USART11_ENABLE)
ROOTFUNC void UART1_IRQHandler(void)
{
	if (UART1->INTSTS & UART_INTSTS_RDAIF_Msk)
	{
		nuc505_usart_onrx[1](nuc505_usart_callback_param[1], UART1->DAT);
	}
	else if (UART1->INTSTS & UART_INTSTS_RXTOINT_Msk)
	{
		nuc505_usart_onrx[1](nuc505_usart_callback_param[1], UART1->DAT);
	}
	if (UART1->INTSTS & UART_INTSTS_THREINT_Msk)
	{
		UART1->INTEN &= (~UART_INTEN_THREIEN_Msk);
		nuc505_usart_ontx[1](nuc505_usart_callback_param[1]);
	}
}
#endif // USART1_INT_EN && (USART01_ENABLE || USART11_ENABLE)

#if USART2_INT_EN && USART02_ENABLE
ROOTFUNC void UART2_IRQHandler(void)
{
	if (UART2->INTSTS & UART_INTSTS_RDAIF_Msk)
	{
		nuc505_usart_onrx[2](nuc505_usart_callback_param[2], UART2->DAT);
	}
	else if (UART2->INTSTS & UART_INTSTS_RXTOINT_Msk)
	{
		nuc505_usart_onrx[2](nuc505_usart_callback_param[2], UART2->DAT);
	}
	if (UART2->INTSTS & UART_INTSTS_THREINT_Msk)
	{
		UART2->INTEN &= (~UART_INTEN_THREIEN_Msk);
		nuc505_usart_ontx[2](nuc505_usart_callback_param[2]);
	}
}
#endif // USART2_INT_EN && USART02_ENABLE

#endif

