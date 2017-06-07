vsf_err_t nuc505_hcd_init(uint32_t index, vsf_err_t (*ohci_irq)(void *), void *param);
vsf_err_t nuc505_hcd_fini(uint32_t index);
void* nuc505_hcd_regbase(uint32_t index);
