vsf_err_t stm32f7_hcd_init(uint32_t index, vsf_err_t (*irq)(void *), void *param);
vsf_err_t stm32f7_hcd_fini(uint32_t index);
void* stm32f7_hcd_regbase(uint32_t index);