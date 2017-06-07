vsf_err_t nuc505_i2c_init(uint8_t index);
vsf_err_t nuc505_i2c_fini(uint8_t index);
vsf_err_t nuc505_i2c_config(uint8_t index, uint16_t kHz, void *param,
							void (*callback)(void *, int32_t, int32_t));
vsf_err_t nuc505_i2c_xfer(uint8_t index, uint16_t chip_addr,
							struct vsfi2c_msg_t *msg, uint16_t msg_len);

