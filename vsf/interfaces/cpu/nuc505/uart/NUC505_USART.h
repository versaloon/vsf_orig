vsf_err_t nuc505_usart_init(uint8_t index);
vsf_err_t nuc505_usart_fini(uint8_t index);
vsf_err_t nuc505_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode);
vsf_err_t nuc505_usart_config_cb(uint8_t index, uint32_t int_priority,
				void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t));
vsf_err_t nuc505_usart_tx(uint8_t index, uint16_t data);
vsf_err_t nuc505_usart_tx_isready(uint8_t index);
uint16_t nuc505_usart_rx(uint8_t index);
vsf_err_t nuc505_usart_rx_isready(uint8_t index);
uint16_t nuc505_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t nuc505_usart_tx_get_free_size(uint8_t index);
uint16_t nuc505_usart_rx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t nuc505_usart_rx_get_data_size(uint8_t index);

