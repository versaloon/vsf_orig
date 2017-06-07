vsf_err_t cmem7_usart_init(uint8_t index);
vsf_err_t cmem7_usart_fini(uint8_t index);
vsf_err_t cmem7_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode);
vsf_err_t cmem7_usart_config_callback(uint8_t index, uint32_t int_priority,
		void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t));
uint16_t cmem7_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t cmem7_usart_tx_get_free_size(uint8_t index);
uint16_t cmem7_usart_rx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t cmem7_usart_rx_get_data_size(uint8_t index);

