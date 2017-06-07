vsf_err_t stm32_flash_init(uint8_t index);
vsf_err_t stm32_flash_fini(uint8_t index);

vsf_err_t stm32_flash_capacity(uint8_t index, uint32_t *pagesize, 
							uint32_t *pagenum);
uint32_t stm32_flash_baseaddr(uint8_t index);
uint32_t stm32_flash_blocksize(uint8_t index, uint32_t addr, uint32_t size,
							int op);
vsf_err_t stm32_flash_config_cb(uint8_t index, uint32_t int_priority,
							void *param, void (*onfinish)(void*, vsf_err_t));

vsf_err_t stm32_flash_erase(uint8_t index, uint32_t offset);
vsf_err_t stm32_flash_read(uint8_t index, uint32_t offset, uint8_t *buff);
vsf_err_t stm32_flash_write(uint8_t index, uint32_t offset, uint8_t *buff);

extern const bool stm32_flash_direct_read;
