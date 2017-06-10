struct usrapp_t
{
	const struct app_hwcfg_t *hwcfg;

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
			struct vsfusbd_MSCBOT_param_t param;
		} msc;
		struct vsfusbd_iface_t ifaces[1];
		struct vsfusbd_config_t config[1];
		struct vsfusbd_device_t device;
	} usbd;
};

extern struct usrapp_t usrapp;

void usrapp_srt_init(struct usrapp_t *app);
#if defined(APPCFG_USR_POLL)
void usrapp_poll(struct usrapp_t *app);
#endif