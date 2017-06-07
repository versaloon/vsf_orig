struct usrapp_t
{
	const struct app_hwcfg_t *hwcfg;

#if defined(APPCFG_BUFMGR_SIZE) && (APPCFG_BUFMGR_SIZE > 0)
	struct vsfshell_t shell;
#endif

	struct
	{
		struct vsfscsi_device_t scsi_dev;
		struct vsfscsi_lun_t lun[1];

		struct vsf_scsistream_t scsistream;
		struct vsf_mal2scsi_t mal2scsi;
		struct vsfmal_t mal;
		struct fakefat32_param_t fakefat32;
		uint8_t *pbuffer[1];
		uint8_t buffer[1][512];
	} mal;

	struct
	{
		struct
		{
			struct vsfusbd_RNDIS_param_t param;
			struct vsfip_dhcpd_t dhcpd;
		} rndis;
		struct
		{
			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[65];
			uint8_t rxbuff[65];
		} cdc;
		struct
		{
			struct vsfusbd_MSCBOT_param_t param;
		} msc;
		struct vsfusbd_iface_t ifaces[5];
		struct vsfusbd_config_t config[1];
		struct vsfusbd_device_t device;
	} usbd;

	struct
	{
		VSFPOOL_DEFINE(buffer_pool, struct vsfip_buffer_t, APPCFG_VSFIP_BUFFER_NUM);
		VSFPOOL_DEFINE(socket_pool, struct vsfip_socket_t, APPCFG_VSFIP_SOCKET_NUM);
#if APPCFG_VSFIP_TCPPCB_NUM > 0
		VSFPOOL_DEFINE(tcppcb_pool, struct vsfip_tcppcb_t, APPCFG_VSFIP_TCPPCB_NUM);
#endif
		uint8_t buffer_mem[APPCFG_VSFIP_BUFFER_NUM][VSFIP_BUFFER_SIZE];

#if APPCFG_TELNETD_EN
		struct
		{
			struct vsfip_telnetd_t telnetd;
			struct vsfip_telnetd_session_t sessions[1];

			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[65];
			uint8_t rxbuff[65];
		} telnetd;
#endif
	} tcpip;
};

extern struct usrapp_t usrapp;

void usrapp_nrt_init(struct usrapp_t *app);
void usrapp_poll(struct usrapp_t *app);
