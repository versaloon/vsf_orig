#include "app_type.h"
#include "compiler.h"
#include "interfaces.h"

#if IFS_USART_EN

#include "cmem7.h"
#include "core.h"

#define USART_NUM						3
#define USART_FIFO_SIZE					16
#define USART_FIFO_HALF_FULL			8


static void (*cmem7_usart_ontx[USART_NUM])(void *);
static void (*cmem7_usart_onrx[USART_NUM])(void *, uint16_t data);
static void *cmem7_usart_callback_param[USART_NUM];

static UART_Type *get_uart(uint8_t index)
{
	if (index == 0)
		return UART0;
	else if (index == 1)
		return UART1;
	else if (index == 2)
		return UART2;
	else
		return NULL;
}

vsf_err_t cmem7_usart_init(uint8_t index)
{
	uint8_t usart_idx = index & 0x0f;
	if (usart_idx == 0)
	{
		SOFT_RESET->SOFTRST_b.UART0_n = 0;
		SOFT_RESET->SOFTRST_b.UART0_n = 1;
		// system clock / 4
		GLOBAL_CTRL->CLK_SEL_0_b.UART0_CLK = 1;
		NVIC_SetPriority(UART0_INT_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
		NVIC_EnableIRQ(UART0_INT_IRQn);
	}
	else if (usart_idx == 1)
	{
		SOFT_RESET->SOFTRST_b.UART1_n = 0;
		SOFT_RESET->SOFTRST_b.UART1_n = 1;
		// system clock / 4
		GLOBAL_CTRL->CLK_SEL_0_b.UART1_CLK = 1;
		NVIC_SetPriority(UART1_INT_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
		NVIC_EnableIRQ(UART1_INT_IRQn);
	}
	else if (usart_idx == 2)
	{
		SOFT_RESET->SOFTRST_b.UART2_n = 0;
		SOFT_RESET->SOFTRST_b.UART2_n = 1;
		// system clock / 4
		GLOBAL_CTRL->CLK_SEL_1_b.UART2_CLK = 1;
		NVIC_SetPriority(UART2_INT_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
		NVIC_EnableIRQ(UART2_INT_IRQn);
	}
	else
	{
		return VSFERR_INVALID_PARAMETER;
	}
	return VSFERR_NONE;
}

vsf_err_t cmem7_usart_fini(uint8_t index)
{
	UART_Type *uart = get_uart(index & 0x0f);
	if (uart == NULL)
		return VSFERR_INVALID_PARAMETER;

	// TODO
	return VSFERR_NONE;
}

vsf_err_t cmem7_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode)
{
	uint32_t clk;
	struct cmem7_info_t *info;
	UART_Type *uart = get_uart(index & 0x0f);
	if (uart == NULL)
		return VSFERR_INVALID_PARAMETER;

	// calc baudrate
	cmem7_interface_get_info(&info);
	clk = info->sys_freq_hz / 4;
	if (baudrate <= 19200)	// mode 0
	{
		uart->CTRL_b.BAUD_MODE = 0;
		uart->BAUDRATE = ((clk << 1) + (baudrate << 4)) / (baudrate << 5);
	}
	else	// mode 1
	{
		uart->CTRL_b.BAUD_MODE = 1;
		uart->BAUDRATE = (((uint64_t)baudrate << 21) + clk) / (clk << 1);
	}

	// mode set
	uart->CTRL_b.STOP = mode & 0x03;
	if (((mode & cmem7_USART_DATALEN_8) == cmem7_USART_DATALEN_8) &&
			((mode & cmem7_USART_PARITY_NONE) == cmem7_USART_PARITY_NONE))
	{
		uart->CTRL_b.MODE = 1;
	}
	else if(((mode & cmem7_USART_DATALEN_8) == cmem7_USART_DATALEN_8) &&
			((mode & cmem7_USART_PARITY_ODD) == cmem7_USART_PARITY_ODD))
	{
		uart->CTRL_b.MODE = 7;
		uart->CTRL_b.PARITY = 1;

	}
	else if(((mode & cmem7_USART_DATALEN_8) == cmem7_USART_DATALEN_8) &&
			((mode & cmem7_USART_PARITY_EVEN) == cmem7_USART_PARITY_EVEN))
	{
		uart->CTRL_b.MODE = 7;
		uart->CTRL_b.PARITY = 0;
	}
	else if(((mode & cmem7_USART_DATALEN_9) == cmem7_USART_DATALEN_9) &&
			((mode & cmem7_USART_PARITY_NONE) == cmem7_USART_PARITY_NONE))
	{
		uart->CTRL_b.MODE = 4;
	}
	else
	{
		return VSFERR_INVALID_PARAMETER;
	}

	// other set
	uart->CTRL_b.LOOPBACK = 0;
	uart->CTRL_b.RX_EN = 1;
	uart->CTRL_b.FIFO_EN = 1;
	uart->CTRL_b.CTS = 0;
	uart->CTRL_b.RX_THRESHOLD = USART_FIFO_SIZE;
	uart->CTRL_b.RX_HALF_FULL = USART_FIFO_HALF_FULL;

	uart->TIMEOUT = 16;

	uart->INT_RAW = 0;
	uart->INT_MASK |= 0x000007FF;
	uart->INT_MASK_b.RHF = 0;
	uart->INT_MASK_b.TONE = 0;

	uart->RUN_b.EN = 1;
	return VSFERR_NONE;
}

vsf_err_t cmem7_usart_config_callback(uint8_t index, uint32_t int_priority,
		void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t))
{
	uint32_t usart_idx = index & 0x0F;

	cmem7_usart_ontx[usart_idx] = ontx;
	cmem7_usart_onrx[usart_idx] = onrx;
	cmem7_usart_callback_param[usart_idx] = p;

	return VSFERR_NONE;
}

uint16_t cmem7_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size)
{
	UART_Type *uart = get_uart(index & 0x0f);
	uint16_t i = 0;

	while (size)
	{
		uart->TX_BUF = data[i];
		size--;
		i++;
	}
	UART2->INT_RAW_b.TE = 0;
	UART2->INT_MASK_b.TE = 0;

	return i;
}

uint16_t cmem7_usart_tx_get_free_size(uint8_t index)
{
	UART_Type *uart = get_uart(index & 0x0f);

	// cmem7 fake tx free size
	if (uart->STATUS_b.TF)
		return 0;
	else if (uart->STATUS_b.TE)
		return 16;
	else if (uart->STATUS_b.THE)
		return 8;
	else
		return 1;
}

uint16_t cmem7_usart_rx_bytes(uint8_t index, uint8_t *data, uint16_t size)
{
	UART_Type *uart = get_uart(index & 0x0f);
	uint16_t i = 0;

	while (uart->STATUS_b.RNE && size)
	{
		data[i++] = uart->RX_BUF;
		size--;
	}
	return i;
}

uint16_t cmem7_usart_rx_get_data_size(uint8_t index)
{
	UART_Type *uart = get_uart(index & 0x0f);

	// cmem7 fake rx data size
	if (uart->STATUS_b.RHF)			// fifo >= 8
		return 16;
	else if (uart->STATUS_b.RNE)	// fifo == 0
		return 7;
	else							// 0 < fifo < 8
		return 0;
}

static void cmem7_common_irqhandler(UART_Type *uart, uint8_t index)
{
	if (uart->INT_SEEN & (0x1ul << 5))
	{
		uart->INT_SEEN |= 0x1ul << 5;
		cmem7_usart_onrx[index](cmem7_usart_callback_param[index], uart->RX_BUF & 0x1ff);
	}
	else if (uart->INT_SEEN & (0x1ul << 3))
	{
		uart->INT_SEEN |= 0x1ul << 3;
		cmem7_usart_onrx[index](cmem7_usart_callback_param[index], uart->RX_BUF & 0x1ff);
	}
	if (uart->INT_SEEN & (0x1ul << 1))
	{
		uart->INT_SEEN |= 0x1ul << 1;
		uart->INT_MASK_b.TE = 1;
		cmem7_usart_ontx[index](cmem7_usart_callback_param[index]);
	}
}

ROOTFUNC void UART0_IRQHandler(void)
{
	cmem7_common_irqhandler(UART0, 0);
}

ROOTFUNC void UART1_IRQHandler(void)
{
	cmem7_common_irqhandler(UART1, 1);
}

ROOTFUNC void UART2_IRQHandler(void)
{
	cmem7_common_irqhandler(UART2, 2);
}

#endif

