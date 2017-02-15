vsf_err_t cmem7_hcd_init(uint32_t index, vsf_err_t (*irq)(void *), void *param);
vsf_err_t cmem7_hcd_fini(uint32_t index);
void* cmem7_hcd_regbase(uint32_t index);
