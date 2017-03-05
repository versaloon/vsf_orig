#include "app_cfg.h"
#include "app_type.h"
#include "interfaces.h"

#include "core.h"

static struct nv32_info_t nv32_info = 
{
	0, CORE_VECTOR_TABLE,
	CORE_CLKSRC, CORE_FLLSRC,
	OSC_HZ, IRC_FREQ_HZ,
	CORE_FLL_FREQ_HZ, CORE_SYS_FREQ_HZ
};

// Pendsv
struct nv32_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static nv32_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (nv32_pendsv.on_pendsv != NULL)
	{
		nv32_pendsv.on_pendsv(nv32_pendsv.param);
	}
}

vsf_err_t nv32_interface_pendsv_config(void (*on_pendsv)(void *), void *param)
{
	nv32_pendsv.on_pendsv = on_pendsv;
	nv32_pendsv.param = param;

	if (nv32_pendsv.on_pendsv != NULL)
	{
		NVIC_SetPriority(PendSV_IRQn, 0xFF);
	}
	return VSFERR_NONE;
}

vsf_err_t nv32_interface_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

vsf_err_t nv32_interface_get_info(struct nv32_info_t **info)
{
	*info = &nv32_info;
	return VSFERR_NONE;
}

vsf_err_t nv32_interface_fini(void *p)
{
	return VSFERR_NONE;
}

vsf_err_t nv32_interface_reset(void *p)
{
	NVIC_SystemReset();
	return VSFERR_NONE;
}

uint32_t nv32_interface_get_stack(void)
{
	return __get_MSP();
}

vsf_err_t nv32_interface_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void nv32_interface_sleep(uint32_t mode)
{
	vsf_leave_critical();
	__WFI();
}

ROOTFUNC int __low_level_init(void)
{
	uint8_t u8Cs1 = WDOG->CS1;
	uint8_t u8Cs2 = WDOG->CS2;
	uint16_t u16TOVAL = WDOG->TOVAL;
	uint16_t u16WIN = WDOG->WIN;

	u8Cs1 &= ~WDOG_CS1_EN_MASK;
	u8Cs1 |= WDOG_CS1_UPDATE_MASK;

	//WDOG_Unlock();
	WDOG->CS2 = u8Cs2;
	WDOG->TOVAL = u16TOVAL;
	WDOG->WIN = u16WIN;
	WDOG->CS1 = u8Cs1;
	return 1;
}

#include "driver/ics/ics.h"
static void nv32_enable_osc(ICS_ConfigType *config)
{
	config->oscConfig.bStopEnable = 1;
	if (nv32_info.osc_freq_hz >= 4000000)
		config->oscConfig.bRange = 1;
	config->oscConfig.bEnable = 1;
}
vsf_err_t nv32_interface_init(void *p)
{
	uint32_t clksrc;
	ICS_ConfigType sICSConfig = {0};

	if (p != NULL)
		nv32_info = *(struct nv32_info_t *)p;

	// on reset, ICS works in FEI mode
	sICSConfig.u32ClkFreq = nv32_info.osc_freq_hz / 1000;
	switch (nv32_info.clksrc)
	{
	case NV32_CLKSRC_IRC:
		// FBILP mode
		sICSConfig.u8ClkMode = ICS_CLK_MODE_FBILP;
		clksrc = nv32_info.irc_freq_hz;
		break;
	case NV32_CLKSRC_OSC:
		// FBELP mode
		sICSConfig.u8ClkMode = ICS_CLK_MODE_FBELP;
		nv32_enable_osc(&sICSConfig);
		clksrc = nv32_info.osc_freq_hz;
		break;
	case NV32_CLKSRC_FLL:
		clksrc = nv32_info.fll_freq_hz;
		switch (nv32_info.fllsrc)
		{
		case NV32_FLLSRC_IRC:
			// FEI mode
			sICSConfig.u8ClkMode = ICS_CLK_MODE_FEI;
			break;
		case NV32_FLLSRC_OSC:
			// FEE mode
			sICSConfig.u8ClkMode = ICS_CLK_MODE_FEE;
			nv32_enable_osc(&sICSConfig);
			break;
		}
		break;
	}
	ICS_Init(&sICSConfig);
	uint8_t bdiv = clksrc / nv32_info.sys_freq_hz;
	bdiv = ffz(~(uint32_t)bdiv);
	ICS->C2 = (ICS->C2 & ~(ICS_C2_BDIV_MASK)) | ICS_C2_BDIV(bdiv);

	// check the clock on PH2 if enabled
//	SIM->SOPT |= SIM_SOPT_CLKOE_MASK;

	SCB->VTOR = nv32_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | nv32_info.priority_group;

	if (nv32_info.sys_freq_hz > 24000000)
		EFMCR = 1;
	else
		EFMCR = 0;
	return VSFERR_NONE;
}

uint32_t nv32_uid_get(uint8_t *buffer, uint32_t size)
{
	if (NULL == buffer)
		return 0;
	
	size = min(size, 12);
	memcpy(buffer, (uint8_t *)&SIM->UUIDL, size);
	return size;
}

// tickclk
#define CM3_SYSTICK_ENABLE				(1 << 0)
#define CM3_SYSTICK_INT					(1 << 1)
#define CM3_SYSTICK_CLKSOURCE			(1 << 2)
#define CM3_SYSTICK_COUNTFLAG			(1 << 16)

static void (*nv32_tickclk_cb)(void *param) = NULL;
static void *nv32_tickclk_param = NULL;
static uint32_t nv32_tickcnt = 0;
vsf_err_t nv32_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= CM3_SYSTICK_ENABLE;
	return VSFERR_NONE;
}

vsf_err_t nv32_tickclk_stop(void)
{
	SysTick->CTRL &= ~CM3_SYSTICK_ENABLE;
	return VSFERR_NONE;
}

static uint32_t nv32_tickclk_get_count_local(void)
{
	return nv32_tickcnt;
}

uint32_t nv32_tickclk_get_count(void)
{
	uint32_t count1, count2;
	
	do {
		count1 = nv32_tickclk_get_count_local();
		count2 = nv32_tickclk_get_count_local();
	} while (count1 != count2);
	return count1;
}

ROOTFUNC void SysTick_Handler(void)
{
	nv32_tickcnt++;
	if (nv32_tickclk_cb != NULL)
	{
		nv32_tickclk_cb(nv32_tickclk_param);
	}
}

vsf_err_t nv32_tickclk_config_cb(void (*callback)(void*), void *param)
{
	uint32_t tmp = SysTick->CTRL;
	
	SysTick->CTRL &= ~CM3_SYSTICK_INT;
	nv32_tickclk_cb = callback;
	nv32_tickclk_param = param;
	SysTick->CTRL = tmp;
	return VSFERR_NONE;
}

void nv32_tickclk_poll()
{
	if (SysTick->CTRL & CM3_SYSTICK_COUNTFLAG)
		SysTick_Handler();
}

vsf_err_t nv32_tickclk_init()
{
	nv32_tickcnt = 0;
	SysTick->LOAD = nv32_info.sys_freq_hz / 1000;
#ifdef IFS_TICKCLK_NOINT
	SysTick->CTRL = CM3_SYSTICK_CLKSOURCE;
#else
	SysTick->CTRL = CM3_SYSTICK_INT | CM3_SYSTICK_CLKSOURCE;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
#endif
	return VSFERR_NONE;
}

vsf_err_t nv32_tickclk_fini(void)
{
	return nv32_tickclk_stop();
}

void HardFault_Handler(void)
{
	while(1);
}
