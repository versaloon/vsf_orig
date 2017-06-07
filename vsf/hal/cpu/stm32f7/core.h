#ifndef __STM32F7_CORE_H_INCLUDED__
#define __STM32F7_CORE_H_INCLUDED__

#define STM32F7_CLK_HSI				(1UL << 0)
#define STM32F7_CLK_HSE				(1UL << 1)
#define STM32F7_CLK_PLL				(1UL << 2)

enum stm32f7_hclksrc_t
{
	STM32F7_HCLKSRC_HSI = 0,
	STM32F7_HCLKSRC_HSE = 1,
	STM32F7_HCLKSRC_PLL = 2
};

enum stm32f7_pllsrc_t
{
	STM32F7_PLLSRC_HSI = 0,
	STM32F7_PLLSRC_HSE = 1,
};

struct stm32f7_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;

	uint32_t clk_enable;

	enum stm32f7_hclksrc_t hclksrc;
	enum stm32f7_pllsrc_t pllsrc;

	uint32_t hsi_freq_hz;
	uint32_t hse_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t hclk_freq_hz;
	uint32_t pclk1_freq_hz;
	uint32_t pclk2_freq_hz;
};

vsf_err_t stm32f7_init(void *p);
vsf_err_t stm32f7_fini(void *p);
vsf_err_t stm32f7_reset(void *p);
uint32_t stm32f7_get_stack(void);
vsf_err_t stm32f7_set_stack(uint32_t sp);
void stm32f7_sleep(uint32_t mode);
vsf_err_t stm32f7_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t stm32f7_pendsv_trigger(void);

vsf_err_t stm32f7_tickclk_init(void);
vsf_err_t stm32f7_tickclk_fini(void);
vsf_err_t stm32f7_tickclk_start(void);
vsf_err_t stm32f7_tickclk_stop(void);
uint32_t stm32f7_tickclk_get_count(void);
vsf_err_t stm32f7_tickclk_config_cb(void (*callback)(void *), void *param);

#endif	// __STM32F7_CORE_H_INCLUDED__
