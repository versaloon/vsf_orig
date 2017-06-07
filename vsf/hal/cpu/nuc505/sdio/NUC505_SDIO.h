vsf_err_t nuc505_sdio_init(uint8_t index);
vsf_err_t nuc505_sdio_fini(uint8_t index);
vsf_err_t nuc505_sdio_config(uint8_t index, uint32_t kHz, uint8_t buswidth,
		void (*callback)(void *), void *param);
vsf_err_t nuc505_sdio_start(uint8_t index, uint8_t cmd, uint32_t arg,
		struct sdio_info_t *extra_param);
vsf_err_t nuc505_sdio_stop(uint8_t index);
vsf_err_t nuc505_sdio_config_int(uint8_t index, void (*callback)(void *param),
		void *param);
vsf_err_t nuc505_sdio_enable_int(uint8_t index);

