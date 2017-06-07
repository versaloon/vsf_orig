vsf_err_t stm32f4_hcd_init(uint32_t index, vsf_err_t (*irq)(void *), void *param);
vsf_err_t stm32f4_hcd_fini(uint32_t index);
void* stm32f4_hcd_regbase(uint32_t index);