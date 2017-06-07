#ifndef __VSFUSBH_H_INCLUDED__
#define __VSFUSBH_H_INCLUDED__

#ifndef USB_MAXCHILDREN
#define USB_MAXCHILDREN					4
#endif
#ifndef USB_MAXENDPOINTS
#define USB_MAXENDPOINTS				16
#endif
#ifndef USB_ALTSETTINGALLOC
#define USB_ALTSETTINGALLOC				4
#endif

#define USBH_INTERFACE_RAM_OPTIMIZE		1

#include "stack/usb/common/usb_common.h"
#include "stack/usb/common/usb_ch11.h"
#include "hcd/hcd.h"

#define VSFSM_EVT_URB_COMPLETE	(VSFSM_EVT_USER_LOCAL + 1)
#define VSFSM_EVT_NEW_DEVICE	(VSFSM_EVT_USER_LOCAL + 2)
#define VSFSM_EVT_EP0_CRIT		(VSFSM_EVT_USER_LOCAL + 3)

#define DEFAULT_TIMEOUT			50	// 50ms

struct vsfusbh_device_t
{
	uint8_t devnum;
	uint8_t devnum_temp;
	uint8_t speed;		/* full/low/high */
	uint8_t slow;

	struct usb_device_descriptor_t descriptor;
	struct usb_config_t *config;
	struct usb_config_t *actconfig;

	uint32_t toggle[2];	// one bit per endpoint

	uint16_t epmaxpacketin[USB_MAXENDPOINTS];
	uint16_t epmaxpacketout[USB_MAXENDPOINTS];

	struct vsfusbh_device_t *parent;
	struct vsfusbh_device_t *children[USB_MAXCHILDREN];

	struct vsfsm_crit_t ep0_crit;

	uint8_t num_config;
	uint8_t maxchild;
	uint8_t temp_u8;
	uint8_t dummy;

	// save priv device pointer
	void *priv;
};

#define USB_DEVICE_ID_MATCH_VENDOR		0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT		0x0002
#define USB_DEVICE_ID_MATCH_DEV_LO		0x0004
#define USB_DEVICE_ID_MATCH_DEV_HI		0x0008
#define USB_DEVICE_ID_MATCH_DEV_CLASS		0x0010
#define USB_DEVICE_ID_MATCH_DEV_SUBCLASS	0x0020
#define USB_DEVICE_ID_MATCH_DEV_PROTOCOL	0x0040
#define USB_DEVICE_ID_MATCH_INT_CLASS		0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS	0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL	0x0200
struct vsfusbh_device_id_t
{
	uint16_t match_flags;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice_lo, bcdDevice_hi;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	//uint32_t driver_info;
};

struct iso_packet_descriptor_t
{
	uint32_t offset;				/*!< Start offset in transfer buffer	*/
	uint32_t length;				/*!< Length in transfer buffer			*/
	uint32_t actual_length;			/*!< Actual transfer length				*/
	int32_t status;					/*!< Transfer status					*/
};


/* The following flags are used internally by usbcore and HCDs */
#define URB_DIR_IN		0x0200	/* Transfer from device to host */
#define URB_DIR_OUT		0
#define URB_DIR_MASK		URB_DIR_IN

#define URB_DMA_MAP_SINGLE	0x00010000	/* Non-scatter-gather mapping */
#define URB_DMA_MAP_PAGE	0x00020000	/* HCD-unsupported S-G */
#define URB_DMA_MAP_SG		0x00040000	/* HCD-supported S-G */
#define URB_MAP_LOCAL		0x00080000	/* HCD-local-memory mapping */
#define URB_SETUP_MAP_SINGLE	0x00100000	/* Setup packet DMA mapped */
#define URB_SETUP_MAP_LOCAL	0x00200000	/* HCD-local setup packet */
#define URB_DMA_SG_COMBINED	0x00400000	/* S-G entries were combined */
#define URB_ALIGNED_TEMP_BUFFER	0x00800000	/* Temp buffer was alloc'd */

// struct vsfusbh_urb_t.transfer_flags
#define USB_DISABLE_SPD			0x0001
#define URB_SHORT_NOT_OK		USB_DISABLE_SPD
#define USB_ISO_ASAP			0x0002
#define USB_ASYNC_UNLINK		0x0008
#define USB_QUEUE_BULK			0x0010
#define USB_NO_FSBR				0x0020
#define USB_ZERO_PACKET			0x0040		// Finish bulk OUTs always with zero length packet
#define URB_NO_INTERRUPT		0x0080		/* HINT: no non-error interrupt needed */
/* ... less overhead for QUEUE_BULK */
#define USB_TIMEOUT_KILLED		0x1000		// only set by HCD!
#define URB_ZERO_PACKET			USB_ZERO_PACKET
#define URB_ISO_ASAP			USB_ISO_ASAP

struct vsfusbh_urb_t
{
	struct vsfusbh_device_t *vsfdev;
	uint32_t pipe;					/*!< pipe information						*/

	uint16_t packet_size;
	uint16_t transfer_flags;		/*!< USB_DISABLE_SPD | USB_ISO_ASAP | etc.	*/
	void *transfer_buffer;
	uint32_t transfer_length;
	uint32_t actual_length;

	struct usb_ctrlrequest_t setup_packet;

	uint32_t start_frame;			/*!< start frame (iso/irq only)		*/
	uint16_t interval;				/*!< polling interval (iso/irq only)*/
	int16_t status;					/*!< returned status				*/
	uint32_t timeout;
	struct vsfsm_t *sm;

	uint32_t urb_priv[1];
};

struct vsfusbh_t;
struct vsfusbh_class_drv_t
{
	const char *name;
	const struct vsfusbh_device_id_t *id_table;
	void * (*probe)(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev,
			struct usb_interface_t *interface,
			const struct vsfusbh_device_id_t *id);
	void (*disconnect)(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev,
			void *priv);
	vsf_err_t (*ioctl)(struct vsfusbh_device_t *dev, uint32_t code, void *buf);
};

struct vsfusbh_hcddrv_t
{
	vsf_err_t (*init_thread)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*fini)(void* param);
	vsf_err_t (*suspend)(void* param);
	vsf_err_t (*resume)(void* param);
	vsf_err_t (*alloc_device)(void *param, struct vsfusbh_device_t *dev);
	vsf_err_t (*free_device)(void *param, struct vsfusbh_device_t *dev);
	struct vsfusbh_urb_t * (*alloc_urb)(void);
	vsf_err_t (*free_urb)(void *param, struct vsfusbh_urb_t **vsfurbp);
	vsf_err_t (*submit_urb)(void *param, struct vsfusbh_urb_t *vsfurb);
	vsf_err_t (*relink_urb)(void *param, struct vsfusbh_urb_t *vsfurb);
	vsf_err_t (*rh_control)(void *param, struct vsfusbh_urb_t *vsfurb);
};

struct vsfusbh_t
{
	const struct vsfusbh_hcddrv_t *hcd;
	void *hcd_param;

	// private
	uint8_t hcd_rh_speed;
	void *hcd_data;
	uint32_t device_bitmap[4];
	struct vsfusbh_device_t *rh_dev;
	struct vsfusbh_device_t *new_dev;
	struct sllist drv_list;

	struct vsfsm_t sm;
	struct vsfsm_pt_t dev_probe_pt;
	struct vsfsm_pt_t hcd_init_pt;

	struct vsfusbh_urb_t *probe_urb;
};

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFUSBH_MODNAME						"vsf.stack.usb.host"

struct vsfusbh_modifs_t
{
	vsf_err_t (*init)(struct vsfusbh_t*);
	vsf_err_t (*fini)(struct vsfusbh_t*);
	vsf_err_t (*register_driver)(struct vsfusbh_t*,
					const struct vsfusbh_class_drv_t*);

	vsf_err_t (*submit_urb)(struct vsfusbh_t*, struct vsfusbh_urb_t*);
	vsf_err_t (*relink_urb)(struct vsfusbh_t*, struct vsfusbh_urb_t*);

	struct vsfusbh_device_t* (*alloc_device)(struct vsfusbh_t*);
	void (*free_device)(struct vsfusbh_t*, struct vsfusbh_device_t*);
	vsf_err_t (*add_device)(struct vsfusbh_t*, struct vsfusbh_device_t*);
	void (*disconnect_device)(struct vsfusbh_t*, struct vsfusbh_device_t**);
	void (*remove_interface)(struct vsfusbh_t*, struct vsfusbh_device_t*,
					struct usb_interface_t*);

	vsf_err_t (*control_msg)(struct vsfusbh_t*, struct vsfusbh_urb_t*,
					uint8_t, uint8_t, uint16_t, uint16_t);
	vsf_err_t (*get_descriptor)(struct vsfusbh_t*, struct vsfusbh_urb_t*,
					uint8_t, uint8_t);
	vsf_err_t (*get_class_descriptor)(struct vsfusbh_t*, struct vsfusbh_urb_t*,
					uint16_t, uint8_t, uint8_t);
	vsf_err_t (*set_configuration)(struct vsfusbh_t*, struct vsfusbh_urb_t*,
					uint8_t);
	vsf_err_t (*set_interface)(struct vsfusbh_t*, struct vsfusbh_urb_t*,
					uint16_t, uint16_t);

	vsf_err_t (*get_extra_descriptor)(uint8_t*, uint16_t, uint8_t, void**);
};

vsf_err_t vsfusbh_modexit(struct vsf_module_t*);
vsf_err_t vsfusbh_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFUSBHMOD						\
	((struct vsfusbh_modifs_t *)vsf_module_load(VSFUSBH_MODNAME, true))
#define vsfusbh_init						VSFUSBHMOD->init
#define vsfusbh_fini						VSFUSBHMOD->fini
#define vsfusbh_register_driver				VSFUSBHMOD->register_driver
#define vsfusbh_submit_urb					VSFUSBHMOD->submit_urb
#define vsfusbh_relink_urb					VSFUSBHMOD->relink_urb
#define vsfusbh_alloc_device				VSFUSBHMOD->alloc_device
#define vsfusbh_free_device					VSFUSBHMOD->free_device
#define vsfusbh_add_device					VSFUSBHMOD->add_device
#define vsfusbh_disconnect_device			VSFUSBHMOD->disconnect_device
#define vsfusbh_remove_interface			VSFUSBHMOD->remove_interface
#define vsfusbh_control_msg					VSFUSBHMOD->control_msg
#define vsfusbh_get_descriptor				VSFUSBHMOD->get_descriptor
#define vsfusbh_get_class_descriptor		VSFUSBHMOD->get_class_descriptor
#define vsfusbh_set_configuration			VSFUSBHMOD->set_configuration
#define vsfusbh_set_interface				VSFUSBHMOD->set_interface
#define vsfusbh_get_extra_descriptor		VSFUSBHMOD->get_extra_descriptor

#else
vsf_err_t vsfusbh_submit_urb(struct vsfusbh_t *usbh, struct vsfusbh_urb_t *vsfurb);
vsf_err_t vsfusbh_relink_urb(struct vsfusbh_t *usbh, struct vsfusbh_urb_t *vsfurb);

vsf_err_t vsfusbh_init(struct vsfusbh_t *usbh);
vsf_err_t vsfusbh_fini(struct vsfusbh_t *usbh);
vsf_err_t vsfusbh_register_driver(struct vsfusbh_t *usbh,
		const struct vsfusbh_class_drv_t *drv);

struct vsfusbh_device_t *vsfusbh_alloc_device(struct vsfusbh_t *usbh);
void vsfusbh_free_device(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev);
vsf_err_t vsfusbh_add_device(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev);
void vsfusbh_disconnect_device(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t **pdev);
void vsfusbh_remove_interface(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct usb_interface_t *interface);

vsf_err_t vsfusbh_control_msg(struct vsfusbh_t *usbh, struct vsfusbh_urb_t *vsfurb,
		uint8_t bRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex);

vsf_err_t vsfusbh_get_descriptor(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint8_t type, uint8_t index);
vsf_err_t vsfusbh_get_class_descriptor(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint16_t ifnum, uint8_t type, uint8_t id);
vsf_err_t vsfusbh_set_configuration(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint8_t configuration);
vsf_err_t vsfusbh_set_interface(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint16_t interface, uint16_t alternate);

vsf_err_t vsfusbh_get_extra_descriptor(uint8_t *buf, uint16_t size,
		uint8_t type, void **ptr);
#endif

#endif	// __VSFUSBH_H_INCLUDED__
