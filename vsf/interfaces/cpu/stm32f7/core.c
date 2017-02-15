#include "app_type.h"
#include "compiler.h"
#include "interfaces.h"

#include "stm32f746xx.h"
#include "core.h"

static struct stm32f7_info_t stm32f7_info =
{
	0, CORE_VECTOR_TABLE,
	CORE_CLKEN,
	CORE_HCLKSRC, CORE_PLLSRC,
	HSI_FREQ_HZ, HSE_FREQ_HZ,
	CORE_PLL_FREQ_HZ, CORE_HCLK_FREQ_HZ, CORE_PCLK1_FREQ_HZ, CORE_PCLK2_FREQ_HZ,
};

vsf_err_t stm32f7_interface_init(void *p)
{
	if (p != NULL)
	{
		stm32f7_info = *(struct stm32f7_info_t *)p;
	}

	// enable cache
	SCB_EnableICache();
	SCB_EnableDCache();
	
	// enable art
	FLASH->ACR |= FLASH_ACR_ARTEN | 6;
	
	// enable hsi and select hclksrc to hsi
	RCC->CR |= RCC_CR_HSION;
	while (!(RCC->CR & RCC_CR_HSIRDY));
	RCC->CFGR &= ~RCC_CFGR_SW;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

    // enable clk
    if (stm32f7_info.clk_enable & STM32F7_CLK_HSE)
    {
		RCC->CR |= RCC_CR_HSEON;
		while (!(RCC->CR & RCC_CR_HSERDY));
    }
    else
	{
		RCC->CR &= ~RCC_CR_HSEON;
	}

	RCC->CR &= ~RCC_CR_PLLON;
    if (stm32f7_info.clk_enable & STM32F7_HCLKSRC_PLL)
	{
		uint32_t n, m, p, input, output, pllcfgr;
		pllcfgr = RCC->PLLCFGR & 0xf0000000;

		pllcfgr |= stm32f7_info.pllsrc == STM32F7_PLLSRC_HSI ? 0 :
				RCC_PLLCFGR_PLLSRC_HSE;
		input = stm32f7_info.pllsrc == STM32F7_PLLSRC_HSI ?
				stm32f7_info.hsi_freq_hz : stm32f7_info.hse_freq_hz;

		if (input % 2000000)
		{
			m = input / 1000000;
			input = 1000000;
		}
		else
		{
			m = input / 2000000;
			input = 2000000;
		}
		pllcfgr |= m;

		for (p = 2; p <= 8; p += 2)
		{
			if ((stm32f7_info.pll_freq_hz * p >= 192000000) &&
				(stm32f7_info.pll_freq_hz * p <= 432000000))
			{
				break;
			}
		}
		if (p > 8)
			return  VSFERR_FAIL;
		pllcfgr |= (p / 2 - 1) << 16;

		output = stm32f7_info.pll_freq_hz * p;
		n = output / input;
		pllcfgr |= n << 6;

		pllcfgr |= (output / 48000000) << 24;
		
		RCC->PLLCFGR = pllcfgr;
		RCC->CR |= RCC_CR_PLLON;
		while (!(RCC->CR & RCC_CR_PLLRDY));
	} 

	// set pclk and hclk
	RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
	// TODO
	RCC->CFGR |= (0x6ul << 10) | (0x6ul << 13) | (0 << 4);

	// select
	RCC->CFGR |= stm32f7_info.hclksrc;
	while (((RCC->CFGR & RCC_CFGR_SWS) >> 2) != stm32f7_info.hclksrc);

	SCB->VTOR = stm32f7_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | stm32f7_info.priority_group;
	return VSFERR_NONE;
}

vsf_err_t stm32f7_interface_fini(void *p)
{
	return VSFERR_NONE;
}
vsf_err_t stm32f7_interface_reset(void *p)
{
	return VSFERR_NONE;
}
uint32_t stm32f7_interface_get_stack(void)
{
	return __get_MSP();
}
vsf_err_t stm32f7_interface_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}
void stm32f7_interface_sleep(uint32_t mode)
{
	// TODO
	return;
}

// Pendsv
struct stm32f7_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static stm32f7_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (stm32f7_pendsv.on_pendsv != NULL)
	{
		stm32f7_pendsv.on_pendsv(stm32f7_pendsv.param);
	}
}

vsf_err_t stm32f7_interface_pendsv_config(void (*on_pendsv)(void *),
		void *param)
{
	stm32f7_pendsv.on_pendsv = on_pendsv;
	stm32f7_pendsv.param = param;

	if (stm32f7_pendsv.on_pendsv != NULL)
	{
		SCB->SHPR[10] = 0xFF;
	}
	return VSFERR_NONE;
}

vsf_err_t stm32f7_interface_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

static void (*stm32f7_tickclk_callback)(void *param) = NULL;
static void *stm32f7_tickclk_param = NULL;
static volatile uint32_t stm32f7_tickcnt = 0;

vsf_err_t stm32f7_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t stm32f7_tickclk_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t stm32f7_tickclk_init(void)
{
	stm32f7_tickcnt = 0;
	SysTick->LOAD = stm32f7_info.hclk_freq_hz / 1000;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
	NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
	return VSFERR_NONE;
}

vsf_err_t stm32f7_tickclk_fini(void)
{
	return stm32f7_tickclk_stop();
}

static uint32_t stm32f7_tickclk_get_count_local(void)
{
	return stm32f7_tickcnt;
}

uint32_t stm32f7_tickclk_get_count(void)
{
	uint32_t count1, count2;

	do {
		count1 = stm32f7_tickclk_get_count_local();
		count2 = stm32f7_tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

ROOTFUNC void SysTick_Handler(void)
{
	stm32f7_tickcnt++;
	if (stm32f7_tickclk_callback != NULL)
	{
		stm32f7_tickclk_callback(stm32f7_tickclk_param);
	}
}

vsf_err_t stm32f7_tickclk_config_cb(void (*callback)(void *), void *param)
{
	uint32_t tmp = SysTick->CTRL;

	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
	stm32f7_tickclk_callback = callback;
	stm32f7_tickclk_param = param;
	SysTick->CTRL = tmp;
	return VSFERR_NONE;
}

