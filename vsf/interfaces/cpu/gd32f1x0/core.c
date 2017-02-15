#include "app_cfg.h"
#include "app_type.h"
#include "interfaces.h"

#include "core.h"

static struct gd32f1x0_info_t gd32f1x0_info = 
{
	0, CORE_VECTOR_TABLE, 
	CORE_CLKEN,
	CORE_CLKSRC, GD32F1X0_PLLSRC_HSE,
	HSE_FREQ_HZ, HSI_FREQ_HZ, CORE_PLL_FREQ_HZ,
	CORE_MCLK_FREQ_HZ, CORE_APB_FREQ_HZ, CORE_MCLK_FREQ_HZ,
};

// Pendsv
struct gd32f1x0_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static gd32f1x0_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (gd32f1x0_pendsv.on_pendsv != NULL)
	{
		gd32f1x0_pendsv.on_pendsv(gd32f1x0_pendsv.param);
	}
}

vsf_err_t gd32f1x0_interface_pendsv_config(void (*on_pendsv)(void *), void *param)
{
	gd32f1x0_pendsv.on_pendsv = on_pendsv;
	gd32f1x0_pendsv.param = param;

	if (gd32f1x0_pendsv.on_pendsv != NULL)
	{
		SCB->SHP[10] = 0xFF;
	}
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_interface_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_interface_get_info(struct gd32f1x0_info_t **info)
{
	*info = &gd32f1x0_info;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_interface_fini(void *p)
{
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_interface_reset(void *p)
{
	NVIC_SystemReset();
	return VSFERR_NONE;
}

uint32_t gd32f1x0_interface_get_stack(void)
{
	return __get_MSP();
}

vsf_err_t gd32f1x0_interface_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void gd32f1x0_interface_sleep(uint32_t mode)
{
	vsf_leave_critical();
	__WFI();
}

static uint32_t __log2__(uint32_t n)
{
	uint32_t i, value = 1;
	
	for (i = 0; i < 31; i++)
	{
		if (value == n)
		{
			return i;
		}
		value <<= 1;
	}
	return 0;
}

#define RCC_GCFGR_FREQ_DIVIDE_RESET		(uint32_t)0x083F000C	/* Reset SCS[1:0], AHBPS[3:0], APB1PS[2:0],APB2PS[2:0],
																	ADCPS[1:0],USBPS[1:0],CKOTUSRC[2:0], CKOUTDIV, and PLLDV bits*/
#define RCC_GCCR_CLK_ENABLE_RESET		(uint32_t)0xFEF6FFFF	/* Reset HSEEN, CKMEN and PLLEN bits */
#define RCC_GCCR_HSEBPS_RESET			(uint32_t)0xFFFBFFFF	/* Reset HSEBPS bit */
#define RCC_GCFGR_CLK_SELECT_RESET		(uint32_t)0xF7C0FFFF	/* Reset PLLSEL, PLLPREDV and PLLMF[3:0] bits */
#define RCC_GCFGR2_HSEPREDV1_RESET		(uint32_t)0xFFFFFFF0	/* Reset HSEPREDV1[3:0] bits */
#define RCC_GCFGR3_RESET				(uint32_t)0xFFFFFEBC	/* Reset USARTSEL[1:0], I2CSEL, CECSEL and ADCSEL bits */
#define RCC_GCCR2_HSI14_RESET			(uint32_t)0xFFFFFFFE	/* Reset HSI14 bit */
#define RCC_GCIR_DISABLE				(uint32_t)0x00000000	/* Disable all interrupts */
#define RCC_GCFGR_PLLMF3_0				(uint32_t)0x003C0000	/* PLLMF[3:0] Bits */
vsf_err_t gd32f1x0_interface_init(void *p)
{
	uint32_t tmp32;
	
	if (p != NULL)
	{
		gd32f1x0_info = *(struct gd32f1x0_info_t *)p;
	}

	// move from System_Init
	RCC->GCCR |= RCC_GCCR_HSIEN;
	RCC->GCFGR &= RCC_GCFGR_FREQ_DIVIDE_RESET;
	RCC->GCCR &= RCC_GCCR_CLK_ENABLE_RESET;
	RCC->GCCR &= RCC_GCCR_HSEBPS_RESET;
	RCC->GCFGR &= RCC_GCFGR_CLK_SELECT_RESET;
	RCC->GCFGR2 &= RCC_GCFGR2_HSEPREDV1_RESET;
	RCC->GCFGR3 &= RCC_GCFGR3_RESET;
	RCC->GCCR2 &= RCC_GCCR2_HSI14_RESET;
	RCC->GCIR = RCC_GCIR_DISABLE;
	
	if (gd32f1x0_info.clk_enable & GD32F1X0_CLK_HSI)
	{
		RCC->GCCR |= RCC_GCCR_HSIEN;
		while (!(RCC->GCCR & RCC_GCCR_HSISTB));
		RCC->GCFGR &= ~RCC_GCFGR_SCS;
		while (RCC->GCFGR & RCC_GCFGR_SCSS);
	}
	
	if (gd32f1x0_info.clk_enable & GD32F1X0_CLK_HSE)
	{
		RCC->GCCR |= RCC_GCCR_HSEEN;
		while (!(RCC->GCCR & RCC_GCCR_HSESTB));
		RCC->GCFGR = (RCC->GCFGR & ~RCC_GCFGR_SCS) | RCC_GCFGR_SCS_HSE;
		while ((RCC->GCFGR & RCC_GCFGR_SCSS) != RCC_GCFGR_SCSS_HSE);
	}
	
	RCC->GCCR &= ~RCC_GCCR_PLLEN;
	RCC->GCFGR &= ~(RCC_GCFGR_AHBPS | RCC_GCFGR_APB1PS | RCC_GCFGR_APB2PS);
	
	if (gd32f1x0_info.clk_enable & GD32F1X0_CLK_PLL)
	{
		if (gd32f1x0_info.pllsrc == GD32F1X0_PLLSRC_HSID2)
		{
			RCC->GCFGR &= ~RCC_GCFGR_PLLSEL;
			tmp32 = gd32f1x0_info.hsi_freq_hz / 2;
		}
		else if (gd32f1x0_info.pllsrc == GD32F1X0_PLLSRC_HSE)
		{
			RCC->GCFGR |= RCC_GCFGR_PLLSEL_HSEPREDIV;
			RCC->GCFGR &= ~RCC_GCFGR_PLLPREDV;
			tmp32 = gd32f1x0_info.hse_freq_hz;
		}
		else if (gd32f1x0_info.pllsrc == GD32F1X0_PLLSRC_HSED2)
		{
			RCC->GCFGR |= RCC_GCFGR_PLLSEL_HSEPREDIV;
			RCC->GCFGR |= RCC_GCFGR_PLLPREDV_PREDIV1_DIV2;
			tmp32 = gd32f1x0_info.hse_freq_hz / 2;
		}
		else
			return VSFERR_FAIL;
		
		RCC->GCFGR &= ~(RCC_GCFGR_PLLMF | RCC_GCFGR_USBPS);
		tmp32 = gd32f1x0_info.pll_freq_hz / tmp32;
		tmp32 -= 2;
		if (tmp32 & 0xf)
			RCC->GCFGR |= (tmp32 & 0xf) << 18;
		if (tmp32 & 0x10)
			RCC->GCFGR |= RCC_GCFGR_PLLMF_4;
		
		RCC->GCCR |= RCC_GCCR_PLLEN;
		while (!(RCC->GCCR & RCC_GCCR_PLLSTB));
		
		if (gd32f1x0_info.pll_freq_hz == 48000000)
			RCC->GCFGR |= RCC_GCFGR_USBPS_Div1;
		else if (gd32f1x0_info.pll_freq_hz == 72000000)
			RCC->GCFGR |= RCC_GCFGR_USBPS_Div1_5;
		else if (gd32f1x0_info.pll_freq_hz == 96000000)
			RCC->GCFGR |= RCC_GCFGR_USBPS_Div2;
		else if (gd32f1x0_info.pll_freq_hz == 120000000)
			RCC->GCFGR |= RCC_GCFGR_USBPS_Div2_5;
	}

	if (gd32f1x0_info.clksrc == GD32F1X0_CLKSRC_PLL)
	{
		tmp32 = gd32f1x0_info.pll_freq_hz / gd32f1x0_info.sys_freq_hz;

		RCC->GCFGR &= ~RCC_GCFGR_AHBPS;
		tmp32 = __log2__(tmp32);
		if (tmp32 > 0)
			RCC->GCFGR |= ((0x08 | (tmp32 - 1)) << 4);
		tmp32 = RCC_GCFGR_SCS_PLL;
	}
	else
		return VSFERR_FAIL;
	RCC->GCFGR = (RCC->GCFGR & ~RCC_GCFGR_SCS) | tmp32;
	while ((RCC->GCFGR & RCC_GCFGR_SCSS) != (tmp32 << 2));

	RCC->APB2CCR |= RCC_APB2CCR_CFGEN;

	SCB->VTOR = gd32f1x0_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | gd32f1x0_info.priority_group;
	return VSFERR_NONE;
}

uint32_t gd32f1x0_uid_get(uint8_t *buffer, uint32_t size)
{
	if (NULL == buffer)
	{
		return 0;
	}
	
	if (size > 12)
	{
		size = 12;
	}
	
	memcpy(buffer, (uint8_t *)0x1ffff7ac, size);
	return size;
}

// tickclk
#define CM3_SYSTICK_ENABLE				(1 << 0)
#define CM3_SYSTICK_INT					(1 << 1)
#define CM3_SYSTICK_CLKSOURCE			(1 << 2)
#define CM3_SYSTICK_COUNTFLAG			(1 << 16)

static void (*gd32f1x0_tickclk_cb)(void *param) = NULL;
static void *gd32f1x0_tickclk_param = NULL;
static uint32_t gd32f1x0_tickcnt = 0;
vsf_err_t gd32f1x0_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= CM3_SYSTICK_ENABLE;
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_tickclk_stop(void)
{
	SysTick->CTRL &= ~CM3_SYSTICK_ENABLE;
	return VSFERR_NONE;
}

static uint32_t gd32f1x0_tickclk_get_count_local(void)
{
	return gd32f1x0_tickcnt;
}

uint32_t gd32f1x0_tickclk_get_count(void)
{
	uint32_t count1, count2;
	
	do {
		count1 = gd32f1x0_tickclk_get_count_local();
		count2 = gd32f1x0_tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

ROOTFUNC void SysTick_Handler(void)
{
	gd32f1x0_tickcnt++;
	if (gd32f1x0_tickclk_cb != NULL)
	{
		gd32f1x0_tickclk_cb(gd32f1x0_tickclk_param);
	}
}

vsf_err_t gd32f1x0_tickclk_config_cb(void (*callback)(void*), void *param)
{
	uint32_t tmp = SysTick->CTRL;
	
	SysTick->CTRL &= ~CM3_SYSTICK_INT;
	gd32f1x0_tickclk_cb = callback;
	gd32f1x0_tickclk_param = param;
	SysTick->CTRL = tmp;
	return VSFERR_NONE;
}

void gd32f1x0_tickclk_poll()
{
	if (SysTick->CTRL & CM3_SYSTICK_COUNTFLAG)
		SysTick_Handler();
}

vsf_err_t gd32f1x0_tickclk_init()
{
	gd32f1x0_tickcnt = 0;
	SysTick->LOAD = gd32f1x0_info.sys_freq_hz / 1000;
#ifdef IFS_TICKCLK_NOINT
	SysTick->CTRL = CM3_SYSTICK_CLKSOURCE;
#else
	SysTick->CTRL = CM3_SYSTICK_INT | CM3_SYSTICK_CLKSOURCE;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
#endif
	return VSFERR_NONE;
}

vsf_err_t gd32f1x0_tickclk_fini(void)
{
	return gd32f1x0_tickclk_stop();
}

void HardFault_Handler(void)
{
	while(1);
}

// AFIO
vsf_err_t gd32f1x0_afio_config(const struct gd32f1x0_afio_pin_t *pin, uint32_t mode)
{
	GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)(GPIOA_BASE + (pin->port << 10));
	RCC->AHBCCR |= RCC_AHBCCR_PAEN << pin->port;

	if (pin->af >= 0)
	{
		GPIOx->AFS[pin->pin >> 3] &= ~((uint32_t)0xF << ((pin->pin & 7) << 2));
		GPIOx->AFS[pin->pin >> 3] |= (uint32_t)pin->af << ((pin->pin & 7) << 2);
	}
	gd32f1x0_gpio_config_pin(pin->port, pin->pin, mode);
	return VSFERR_NONE;
}

