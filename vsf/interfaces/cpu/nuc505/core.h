#ifndef __NUC505_CORE_H_INCLUDED__
#define __NUC505_CORE_H_INCLUDED__

#define NUC505_CLK_HXT				(1UL << 0)
#define NUC505_CLK_LXT				(1UL << 1)
#define NUC505_CLK_LIRC				(1UL << 2)
#define NUC505_CLK_PLL				(1UL << 3)
#define NUC505_CLK_APLL				(1UL << 4)

enum nuc505_hclksrc_t
{
	NUC505_HCLKSRC_PLLFOUT = 1,
	NUC505_HCLKSRC_HXT = 0,
};

struct nuc505_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;

	uint32_t clk_enable;

	enum nuc505_hclksrc_t hclksrc;

	uint32_t osc_freq_hz;
	uint32_t osc32k_freq_hz;
	uint32_t lirc_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t apll_freq_hz;
	uint32_t cpu_freq_hz;
	uint32_t hclk_freq_hz;
	uint32_t pclk_freq_hz;
};

vsf_err_t nuc505_interface_init(void *p);
vsf_err_t nuc505_interface_fini(void *p);
vsf_err_t nuc505_interface_reset(void *p);
uint32_t nuc505_interface_get_stack(void);
vsf_err_t nuc505_interface_set_stack(uint32_t sp);
void nuc505_interface_sleep(uint32_t mode);
vsf_err_t nuc505_interface_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t nuc505_interface_pendsv_trigger(void);
vsf_err_t nuc505_interface_get_info(struct nuc505_info_t **info);

vsf_err_t nuc505_tickclk_init(void);
vsf_err_t nuc505_tickclk_fini(void);
vsf_err_t nuc505_tickclk_set_interval(uint16_t ms);
vsf_err_t nuc505_tickclk_start(void);
vsf_err_t nuc505_tickclk_stop(void);
uint32_t nuc505_tickclk_get_count(void);
vsf_err_t nuc505_tickclk_config_cb(void (*callback)(void*), void *param);

// special
int32_t nuc505_is_running_on_ram(void);
vsf_err_t nuc505_code_map(uint8_t en, uint8_t rst, uint8_t len_kb, uint32_t addr);


#endif	// __NUC505_CORE_H_INCLUDED__

