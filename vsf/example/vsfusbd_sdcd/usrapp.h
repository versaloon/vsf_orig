struct usrapp_t
{
	const struct app_hwcfg_t *hwcfg;
	struct
	{
		struct usbd_cdc_t
		{
			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[65];
			uint8_t rxbuff[65];
		} cdc;

		struct vsfusbd_iface_t ifaces[2];
		struct vsfusbd_config_t config[1];
		struct vsfusbd_device_t device;
	} usbd;
};

extern struct usrapp_t usrapp;

void usrapp_init(struct usrapp_t *app);
void usrapp_poll(struct usrapp_t *app);
