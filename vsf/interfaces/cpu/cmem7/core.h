#ifndef __CMEM7_CORE_H_INCLUDED__
#define __CMEM7_CORE_H_INCLUDED__

struct cmem7_info_t
{
	uint32_t sys_freq_hz;
	uint8_t priority_group;
	uint32_t vector_table;
};

vsf_err_t cmem7_interface_init(void *p);
vsf_err_t cmem7_interface_fini(void *p);
vsf_err_t cmem7_interface_reset(void *p);
uint32_t cmem7_interface_get_stack(void);
vsf_err_t cmem7_interface_set_stack(uint32_t sp);
void cmem7_interface_sleep(uint32_t mode);
vsf_err_t cmem7_interface_pendsv(void (*on_pendsv)(void *), void *param);

vsf_err_t cmem7_interface_get_info(struct cmem7_info_t **info);

vsf_err_t cmem7_tickclk_init(void);
vsf_err_t cmem7_tickclk_fini(void);
vsf_err_t cmem7_tickclk_start(void);
vsf_err_t cmem7_tickclk_stop(void);
uint32_t cmem7_tickclk_get_count(void);
vsf_err_t cmem7_tickclk_set_callback(void (*callback)(void*), void *param);

#endif	// __CMEM7_CORE_H_INCLUDED__

