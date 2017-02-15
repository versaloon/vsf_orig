#ifndef __NV32_CORE_H_INCLUDED__
#define __NV32_CORE_H_INCLUDED__

#define NV32_CLK_HSI				(1UL << 0)
#define NV32_CLK_HSE				(1UL << 1)
#define NV32_CLK_PLL				(1UL << 2)

struct nv32_afio_pin_t
{
	uint8_t port;
	uint8_t pin;
	int8_t af;
};
vsf_err_t nv32_afio_config(const struct nv32_afio_pin_t *pin, uint32_t mode);

enum nv32_clksrc_t
{
	NV32_CLKSRC_FLL = 0,
	NV32_CLKSRC_IRC = 1,
	NV32_CLKSRC_OSC = 2,
};
enum nv32_fllsrc_t
{
	NV32_FLLSRC_IRC,
	NV32_FLLSRC_OSC,
};
struct nv32_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;
	
	enum nv32_clksrc_t clksrc;
	enum nv32_fllsrc_t fllsrc;
	
	uint32_t osc_freq_hz;
	uint32_t irc_freq_hz;
	uint32_t fll_freq_hz;
	uint32_t sys_freq_hz;
};

vsf_err_t nv32_interface_init(void *p);
vsf_err_t nv32_interface_fini(void *p);
vsf_err_t nv32_interface_reset(void *p);
uint32_t nv32_interface_get_stack(void);
vsf_err_t nv32_interface_set_stack(uint32_t sp);
void nv32_interface_sleep(uint32_t mode);
vsf_err_t nv32_interface_get_info(struct nv32_info_t **info);
vsf_err_t nv32_interface_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t nv32_interface_pendsv_trigger(void);

uint32_t nv32_uid_get(uint8_t *buffer, uint32_t size);

vsf_err_t nv32_tickclk_init(void);
vsf_err_t nv32_tickclk_fini(void);
void nv32_tickclk_poll(void);
vsf_err_t nv32_tickclk_start(void);
vsf_err_t nv32_tickclk_stop(void);
uint32_t nv32_tickclk_get_count(void);
vsf_err_t nv32_tickclk_config_cb(void (*callback)(void*), void *param);

#endif	// __NV32_CORE_H_INCLUDED__
