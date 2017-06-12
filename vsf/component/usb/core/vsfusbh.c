/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "vsf.h"

#undef vsfusbh_init
#undef vsfusbh_fini
#undef vsfusbh_register_driver
#undef vsfusbh_submit_urb
#undef vsfusbh_relink_urb
#undef vsfusbh_alloc_device
#undef vsfusbh_free_device
#undef vsfusbh_add_device
#undef vsfusbh_disconnect_device
#undef vsfusbh_remove_interface
#undef vsfusbh_control_msg
#undef vsfusbh_get_descriptor
#undef vsfusbh_get_class_descriptor
#undef vsfusbh_set_configuration
#undef vsfusbh_set_interface
#undef vsfusbh_get_extra_descriptor

#define USB_MAX_DEVICE				127

struct vsfusbh_class_drv_list
{
	const struct vsfusbh_class_drv_t *drv;
	struct sllist list;
};

struct vsfusbh_device_t *vsfusbh_alloc_device(struct vsfusbh_t *usbh)
{
	vsf_err_t err;
	struct vsfusbh_device_t *dev;

	dev = vsf_bufmgr_malloc(sizeof(struct vsfusbh_device_t));
	if (NULL == dev)
		return NULL;
	memset(dev, 0, sizeof(struct vsfusbh_device_t));

	err = usbh->hcd->alloc_device(usbh->hcd_data, dev);
	if (err != VSFERR_NONE)
	{
		vsf_bufmgr_free(dev);
		return NULL;
	}

	dev->devnum = mskarr_ffz(usbh->device_bitmap, USB_MAX_DEVICE);
	if (dev->devnum == 0)
	{
		vsf_bufmgr_free(dev);
		return NULL;
	}
	mskarr_set(usbh->device_bitmap, dev->devnum);
	return dev;
}

void vsfusbh_free_device(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev)
{
	uint8_t i, j, k;
	struct usb_config_t *config;
	struct usb_interface_t *interface;
	struct usb_interface_desc_t *itd;

	if (dev->devnum != 0)
		mskarr_clr(usbh->device_bitmap, dev->devnum);

	usbh->hcd->free_device(usbh->hcd_data, dev);

	if (dev->config != NULL)
	{
		config = dev->config;
		for (i = 0; i < dev->num_config; i++)
		{
			if (config[i].interface != NULL)
			{
				interface = config[i].interface;
				for (j = 0; j < config[i].bNumInterfaces; j++)
				{
					if (interface[j].altsetting != NULL)
					{
						itd = interface[j].altsetting;
						for (k = 0; k < interface[j].num_altsetting; k++)
						{
							if (itd[k].ep_desc != NULL)
								vsf_bufmgr_free(itd[k].ep_desc);
						}
						vsf_bufmgr_free(itd);
					}
				}
				vsf_bufmgr_free(interface);
			}
			if (dev->config[i].config_buffer != NULL)
				vsf_bufmgr_free(dev->config[i].config_buffer);
		}
		vsf_bufmgr_free(dev->config);
		dev->config = NULL;
	}
}

static const struct vsfusbh_device_id_t *vsfusbh_match_id(
		struct vsfusbh_device_t *dev, struct usb_interface_t *iface,
		const struct vsfusbh_device_id_t *id)
{
	struct usb_interface_desc_t *intf;

	if (id == NULL)
		return NULL;

	for (; id->idVendor || id->bDeviceClass || id->bInterfaceClass; id++)
	{
		if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
				id->idVendor != dev->descriptor.idVendor)
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
				id->idProduct != dev->descriptor.idProduct)
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
				(id->bcdDevice_lo > dev->descriptor.bcdDevice))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
				(id->bcdDevice_hi < dev->descriptor.bcdDevice))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
				(id->bDeviceClass != dev->descriptor.bDeviceClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
				(id->bDeviceSubClass!= dev->descriptor.bDeviceSubClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
				(id->bDeviceProtocol != dev->descriptor.bDeviceProtocol))
			continue;

		intf = &iface->altsetting[iface->act_altsetting];
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
				(id->bInterfaceClass != intf->bInterfaceClass))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
				(id->bInterfaceSubClass != intf->bInterfaceSubClass))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
				(id->bInterfaceProtocol != intf->bInterfaceProtocol))
			continue;
		return id;
	}
	return NULL;
}

static vsf_err_t vsfusbh_match_intrface_driver(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct usb_interface_t *interface)
{
	uint8_t i;
	vsf_err_t err = VSFERR_FAIL;
	struct sllist *list = &usbh->drv_list;
	struct vsfusbh_class_drv_list *drv_list;
	const struct vsfusbh_class_drv_t *drv;
	const struct vsfusbh_device_id_t *id;

	while (list->next)
	{
		list = list->next;
		drv_list = sllist_get_container(list, struct vsfusbh_class_drv_list, list);
		drv = drv_list->drv;

		id = drv->id_table;

		if (id)
		{
			for (i = 0; i < interface->num_altsetting; i++)
			{
				interface->act_altsetting = i;
				id = vsfusbh_match_id(dev, interface, id);
				if (id)
				{
					err = VSFERR_NONE;
					goto end;
				}
			}
		}
	}
	
end:
	interface->act_altsetting = 0;
	return err;
}

static vsf_err_t vsfusbh_find_intrface_driver(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, uint8_t ifnum)
{
	uint8_t i;
	struct sllist *list = &usbh->drv_list;
	struct usb_interface_t *interface = dev->actconfig->interface + ifnum;
	struct vsfusbh_class_drv_list *drv_list;
	const struct vsfusbh_class_drv_t *drv;
	const struct vsfusbh_device_id_t *id;
	void *priv = NULL;

	while (list->next)
	{
		list = list->next;
		drv_list = sllist_get_container(list, struct vsfusbh_class_drv_list, list);
		drv = drv_list->drv;

		id = drv->id_table;

		if (id)
		{
			for (i = 0; i < interface->num_altsetting; i++)
			{
				interface->act_altsetting = i;
				id = vsfusbh_match_id(dev, interface, id);
				if (id)
				{
					priv = drv->probe(usbh, dev, interface, id);
					if (priv)
						break;
				}
			}
			if (priv == NULL)
				interface->act_altsetting = 0;
		}
		else
		{
			priv = drv->probe(usbh, dev, interface, NULL);
		}

		if (priv)
		{
			interface->private_data = priv;
			interface->driver = drv;
			return VSFERR_NONE;
		}
	}
	return VSFERR_FAIL;
}

void vsfusbh_remove_interface(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct usb_interface_t *interface)
{
	const struct vsfusbh_class_drv_t *drv = interface->driver;
	if (drv)
	{
		drv->disconnect(usbh, dev, interface->private_data);
		interface->driver = NULL;
		interface->private_data = NULL;
		interface->act_altsetting = 0;
	}
}

vsf_err_t vsfusbh_add_device(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev)
{
	uint8_t i, j, k, claimed = 0;
#if 0
	uint8_t rejected = 0;
#endif

	vsfsm_crit_init(&dev->ep0_crit, VSFSM_EVT_EP0_CRIT);

	for (i = 0; i < dev->actconfig->bNumInterfaces; i++)
	{
		if ((dev->actconfig->interface[i].altsetting != NULL) &&
				(dev->actconfig->interface[i].driver == NULL))
		{
			if (vsfusbh_find_intrface_driver(usbh, dev, i) == VSFERR_NONE)
				claimed++;
#if 0
			else
				rejected++;
#endif
		}
	}

	// clear all 'extra' pointer
	// free config_buffer
	if (dev->config != NULL)
	{
		for (i = 0; i < dev->num_config; i++)
		{
			//dev->config[i].extra = NULL;
			if (dev->config[i].interface != NULL)
			{
				struct usb_interface_desc_t *altsetting =
							dev->config[i].interface->altsetting;
				if (altsetting != NULL)
				{
					for (j = 0; j < dev->config[i].interface->num_altsetting;
							j++)
					{
						altsetting[j].extra = NULL;
						if (altsetting[j].ep_desc != NULL)
						{
							for (k = 0; k < altsetting[j].bNumEndpoints; k++)
							{
								altsetting[j].ep_desc[k].extra = NULL;
							}
						}
					}
				}
			}

			if (dev->config[i].config_buffer != NULL)
				vsf_bufmgr_free(dev->config[i].config_buffer);
			dev->config[i].config_buffer = NULL;
		}
	}

#if 0
	if (rejected); // unhandled interfaces on device
#endif

	if (claimed == 0)
		return VSFERR_NOT_SUPPORT;
	else
		return VSFERR_NONE;
}

void vsfusbh_disconnect_device(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t **pdev)
{
	struct vsfusbh_device_t *dev = *pdev;
	uint8_t i;

	if (!dev)
		return;

	*pdev = NULL;

	if (dev->actconfig)
	{
		for (i = 0; i < dev->actconfig->bNumInterfaces; i++)
			vsfusbh_remove_interface(usbh, dev, dev->actconfig->interface + i);
	}

	for (i = 0; i < USB_MAXCHILDREN; i++)
	{
		if (dev->children[i])
		{
			vsfusbh_disconnect_device(usbh, &dev->children[i]);
		}
	}

	vsfusbh_free_device(usbh, dev);
	vsf_bufmgr_free(dev);
}


#define KERNEL_REL	0
#define KERNEL_VER	0

/* usb 2.0 root hub device descriptor */
static const uint8_t usb2_rh_dev_descriptor[18] =
{
	0x12,       /*  __u8  bLength; */
	0x01,       /*  __u8  bDescriptorType; Device */
	0x00, 0x02, /*  __le16 bcdUSB; v2.0 */

	0x09,	    /*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*  __u8  bDeviceSubClass; */
	0x00,       /*  __u8  bDeviceProtocol; [ usb 2.0 no TT ] */
	0x40,       /*  __u8  bMaxPacketSize0; 64 Bytes */

	0x6b, 0x1d, /*  __le16 idVendor; Linux Foundation 0x1d6b */
	0x02, 0x00, /*  __le16 idProduct; device 0x0002 */
	KERNEL_VER, KERNEL_REL, /*  __le16 bcdDevice */

	0x03,       /*  __u8  iManufacturer; */
	0x02,       /*  __u8  iProduct; */
	0x01,       /*  __u8  iSerialNumber; */
	0x01        /*  __u8  bNumConfigurations; */
};

/* no usb 2.0 root hub "device qualifier" descriptor: one speed only */

/* usb 1.1 root hub device descriptor */
static const uint8_t usb11_rh_dev_descriptor[18] =
{
	0x12,       /*  __u8  bLength; */
	0x01,       /*  __u8  bDescriptorType; Device */
	0x10, 0x01, /*  __le16 bcdUSB; v1.1 */

	0x09,	    /*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*  __u8  bDeviceSubClass; */
	0x00,       /*  __u8  bDeviceProtocol; [ low/full speeds only ] */
	0x40,       /*  __u8  bMaxPacketSize0; 64 Bytes */

	0x6b, 0x1d, /*  __le16 idVendor; Linux Foundation 0x1d6b */
	0x01, 0x00, /*  __le16 idProduct; device 0x0001 */
	KERNEL_VER, KERNEL_REL, /*  __le16 bcdDevice */

	0x03,       /*  __u8  iManufacturer; */
	0x02,       /*  __u8  iProduct; */
	0x01,       /*  __u8  iSerialNumber; */
	0x01        /*  __u8  bNumConfigurations; */
};

static const uint8_t fs_rh_config_descriptor[] =
{

	/* one configuration */
	0x09,       /*  __u8  bLength; */
	0x02,       /*  __u8  bDescriptorType; Configuration */
	0x19, 0x00, /*  __le16 wTotalLength; */
	0x01,       /*  __u8  bNumInterfaces; (1) */
	0x01,       /*  __u8  bConfigurationValue; */
	0x00,       /*  __u8  iConfiguration; */
	0xc0,       /*  __u8  bmAttributes;
				 Bit 7: must be set,
				     6: Self-powered,
				     5: Remote wakeup,
				     4..0: resvd */
	0x00,       /*  __u8  MaxPower; */

	/* USB 1.1:
	 * USB 2.0, single TT organization (mandatory):
	 *	one interface, protocol 0
	 *
	 * USB 2.0, multiple TT organization (optional):
	 *	two interfaces, protocols 1 (like single TT)
	 *	and 2 (multiple TT mode) ... config is
	 *	sometimes settable
	 *	NOT IMPLEMENTED
	 */

	/* one interface */
	0x09,       /*  __u8  if_bLength; */
	0x04,       /*  __u8  if_bDescriptorType; Interface */
	0x00,       /*  __u8  if_bInterfaceNumber; */
	0x00,       /*  __u8  if_bAlternateSetting; */
	0x01,       /*  __u8  if_bNumEndpoints; */
	0x09,       /*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,       /*  __u8  if_bInterfaceSubClass; */
	0x00,       /*  __u8  if_bInterfaceProtocol; [usb1.1 or single tt] */
	0x00,       /*  __u8  if_iInterface; */

	/* one endpoint (status change endpoint) */
	0x07,       /*  __u8  ep_bLength; */
	0x05,       /*  __u8  ep_bDescriptorType; Endpoint */
	0x81,       /*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,       /*  __u8  ep_bmAttributes; Interrupt */
	0x02, 0x00, /*  __le16 ep_wMaxPacketSize; 1 + (MAX_ROOT_PORTS / 8) */
	0xff        /*  __u8  ep_bInterval; (255ms -- usb 2.0 spec) */
};

static const uint8_t hs_rh_config_descriptor[] =
{

	/* one configuration */
	0x09,       /*  __u8  bLength; */
	0x02,       /*  __u8  bDescriptorType; Configuration */
	0x19, 0x00, /*  __le16 wTotalLength; */
	0x01,       /*  __u8  bNumInterfaces; (1) */
	0x01,       /*  __u8  bConfigurationValue; */
	0x00,       /*  __u8  iConfiguration; */
	0xc0,       /*  __u8  bmAttributes;
				 Bit 7: must be set,
				     6: Self-powered,
				     5: Remote wakeup,
				     4..0: resvd */
	0x00,       /*  __u8  MaxPower; */

	/* USB 1.1:
	 * USB 2.0, single TT organization (mandatory):
	 *	one interface, protocol 0
	 *
	 * USB 2.0, multiple TT organization (optional):
	 *	two interfaces, protocols 1 (like single TT)
	 *	and 2 (multiple TT mode) ... config is
	 *	sometimes settable
	 *	NOT IMPLEMENTED
	 */

	/* one interface */
	0x09,       /*  __u8  if_bLength; */
	0x04,       /*  __u8  if_bDescriptorType; Interface */
	0x00,       /*  __u8  if_bInterfaceNumber; */
	0x00,       /*  __u8  if_bAlternateSetting; */
	0x01,       /*  __u8  if_bNumEndpoints; */
	0x09,       /*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,       /*  __u8  if_bInterfaceSubClass; */
	0x00,       /*  __u8  if_bInterfaceProtocol; [usb1.1 or single tt] */
	0x00,       /*  __u8  if_iInterface; */

	/* one endpoint (status change endpoint) */
	0x07,       /*  __u8  ep_bLength; */
	0x05,       /*  __u8  ep_bDescriptorType; Endpoint */
	0x81,       /*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,       /*  __u8  ep_bmAttributes; Interrupt */
	/* __le16 ep_wMaxPacketSize; 1 + (MAX_ROOT_PORTS / 8)
	 * see hub.c:hub_configure() for details. */
	(USB_MAXCHILDREN + 1 + 7) / 8, 0x00,
	0x0c        /*  __u8  ep_bInterval; (256ms -- usb 2.0 spec) */
};

static void vsfusbh_rh_complete_event(void *param)
{
	vsfsm_post_evt_pending((struct vsfsm_t *)param, VSFSM_EVT_URB_COMPLETE);
}

void vsfusbh_rh_urb_complete(struct vsfusbh_urb_t *vsfurb, int16_t result)
{
	vsfurb->status = result;
	vsftimer_create_cb(1, 1, vsfusbh_rh_complete_event, vsfurb->sm);
}

static vsf_err_t vsfusbh_rh_submit_urb(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb)
{
	uint16_t typeReq, wValue, wLength;
	struct usb_ctrlrequest_t *cmd = &vsfurb->setup_packet;
	uint32_t pipe = vsfurb->pipe;
	uint8_t data[32];
	uint8_t len = 0;

	if (usb_pipeint(pipe))
	{
		// WARNING: not support int transfer for HUB
		return VSFERR_NOT_SUPPORT;
	}

	vsfurb->actual_length = 0;

	typeReq = (cmd->bRequestType << 8) | cmd->bRequest;
	wValue = cmd->wValue;
	wLength = cmd->wLength;

	if (wLength > vsfurb->transfer_length)
		goto error;

	switch (typeReq)
	{
	case DeviceRequest | USB_REQ_GET_STATUS:
		data[0] = 1;
		data[1] = 0;
		len = 2;
		break;
	case DeviceOutRequest | USB_REQ_CLEAR_FEATURE:
	case DeviceOutRequest | USB_REQ_SET_FEATURE:
		break;
	case DeviceRequest | USB_REQ_GET_CONFIGURATION:
		data[0] = 1;
		len = 1;
		break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		break;
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (wValue & 0xff00)
		{
		case USB_DT_DEVICE << 8:
			switch (vsfurb->vsfdev->speed)
			{
			case USB_SPEED_LOW:
			case USB_SPEED_FULL:
				memcpy (data, usb11_rh_dev_descriptor, 18);
				break;
			case USB_SPEED_HIGH:
				memcpy (data, usb2_rh_dev_descriptor, 18);
				break;
			//case USB_SPEED_VARIABLE:
			//	break;
			//case USB_SPEED_SUPER:
			//	break;
			default:
				goto error;
			}
			len = 18;
			break;
		case USB_DT_CONFIG << 8:
			switch (vsfurb->vsfdev->speed)
			{
			case USB_SPEED_LOW:
			case USB_SPEED_FULL:
				len = sizeof(fs_rh_config_descriptor);
				memcpy (data, fs_rh_config_descriptor, len);
				break;
			case USB_SPEED_HIGH:
			case USB_SPEED_VARIABLE:
				\
				len = sizeof(hs_rh_config_descriptor);
				memcpy (data, hs_rh_config_descriptor, len);
				break;
			//case USB_SPEED_SUPER:
			//	break;
			default:
				goto error;
			}
			break;
		case USB_DT_STRING << 8:
			goto nongeneric;
		default:
			goto error;
		}
		break;
	case DeviceRequest | USB_REQ_GET_INTERFACE:
		data[0] = 0;
		len = 1;
		break;
	case DeviceOutRequest | USB_REQ_SET_INTERFACE:
		break;
	case DeviceOutRequest | USB_REQ_SET_ADDRESS:
		break;
	case EndpointRequest | USB_REQ_GET_STATUS:
		data[0] = 0;
		data[1] = 0;
		len = 2;
		break;
	case EndpointOutRequest | USB_REQ_CLEAR_FEATURE:
	case EndpointOutRequest | USB_REQ_SET_FEATURE:
		break;
	default:
nongeneric:
		return usbh->hcd->rh_control(usbh->hcd_data, vsfurb);
	}

	if (len)
	{
		if (vsfurb->transfer_length < len)
			len = vsfurb->transfer_length;
		vsfurb->actual_length = len;

		memcpy (vsfurb->transfer_buffer, data, len);
	}

	vsfusbh_rh_urb_complete(vsfurb, URB_OK);
	return VSFERR_NONE;

error:
	vsfurb->status = URB_FAIL;
	return VSFERR_FAIL;
}

vsf_err_t vsfusbh_submit_urb(struct vsfusbh_t *usbh, struct vsfusbh_urb_t *vsfurb)
{
	if (usb_pipein(vsfurb->pipe))
		vsfurb->packet_size = vsfurb->vsfdev->epmaxpacketin[usb_pipeendpoint(vsfurb->pipe)];
	else
		vsfurb->packet_size = vsfurb->vsfdev->epmaxpacketout[usb_pipeendpoint(vsfurb->pipe)];

	if (vsfurb->vsfdev == usbh->rh_dev)
		return vsfusbh_rh_submit_urb(usbh, vsfurb);
	else
		return usbh->hcd->submit_urb(usbh->hcd_data, vsfurb);
}

vsf_err_t vsfusbh_relink_urb(struct vsfusbh_t *usbh, struct vsfusbh_urb_t *vsfurb)
{
	return usbh->hcd->relink_urb(usbh->hcd_data, vsfurb);
}

vsf_err_t vsfusbh_control_msg(struct vsfusbh_t *usbh, struct vsfusbh_urb_t *vsfurb,
		uint8_t bRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex)
{
	vsfurb->timeout = 200;

	vsfurb->setup_packet.bRequestType = bRequestType;
	vsfurb->setup_packet.bRequest = bRequest;
	vsfurb->setup_packet.wValue = wValue;
	vsfurb->setup_packet.wIndex = wIndex;
	vsfurb->setup_packet.wLength = vsfurb->transfer_length;

	return vsfusbh_submit_urb(usbh, vsfurb);
}


static void vsfusbh_set_maxpacket_ep(struct vsfusbh_device_t *dev)
{
	int i, b;

	for (i = 0; i < dev->actconfig->bNumInterfaces; i++)
	{
		struct usb_interface_t *intf = dev->actconfig->interface + i;
		struct usb_interface_desc_t *intf_desc = intf->altsetting +
					intf->act_altsetting;
		struct usb_endpoint_desc_t *ep_desc;
		int e;
		
		if (intf_desc == NULL)
			continue;
		
		ep_desc = intf_desc->ep_desc;

		for (e = 0; e < intf_desc->bNumEndpoints; e++)
		{
			b = ep_desc[e].bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
			if ((ep_desc[e].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
					USB_ENDPOINT_XFER_CONTROL)
			{
				dev->epmaxpacketout[b] = ep_desc[e].wMaxPacketSize;
				dev->epmaxpacketin[b] = ep_desc[e].wMaxPacketSize;
			}
			else if (usb_pipeout(ep_desc[e].bEndpointAddress))
			{
				if (ep_desc[e].wMaxPacketSize > dev->epmaxpacketout[b])
					dev->epmaxpacketout[b] = ep_desc[e].wMaxPacketSize;
			}
			else
			{
				if (ep_desc[e].wMaxPacketSize > dev->epmaxpacketin[b])
					dev->epmaxpacketin[b] = ep_desc[e].wMaxPacketSize;
			}
		}
	}
}


vsf_err_t vsfusbh_set_address(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb)
{
	vsfurb->pipe = usb_snddefctrl(vsfurb->vsfdev);
	return vsfusbh_control_msg(usbh, vsfurb, USB_DIR_OUT, USB_REQ_SET_ADDRESS,
			vsfurb->vsfdev->devnum, 0);
}
vsf_err_t vsfusbh_get_descriptor(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint8_t type, uint8_t index)
{
	vsfurb->pipe = usb_rcvctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_DIR_IN, USB_REQ_GET_DESCRIPTOR,
			(type << 8) + index, index);
}
vsf_err_t vsfusbh_get_class_descriptor(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint16_t ifnum, uint8_t type, uint8_t id)
{
	vsfurb->pipe = usb_rcvctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_RECIP_INTERFACE | USB_DIR_IN,
			USB_REQ_GET_DESCRIPTOR, (type << 8) + id, ifnum);
}
vsf_err_t vsfusbh_set_configuration(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint8_t configuration)
{
	vsfurb->pipe = usb_sndctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_DIR_OUT,
			USB_REQ_SET_CONFIGURATION, configuration, 0);
}
vsf_err_t vsfusbh_set_interface(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint16_t interface, uint16_t alternate)
{
	vsfurb->pipe = usb_sndctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_RECIP_INTERFACE,
			USB_REQ_SET_INTERFACE, alternate, interface);
}


static int parse_endpoint(struct usb_endpoint_desc_t *endpoint,
		unsigned char *buffer, int size)
{
	struct usb_descriptor_header_t *header;
	int parsed = 0, len, numskipped;
	uint32_t begin;

	header = (struct usb_descriptor_header_t *)buffer;

	/* Everything should be fine being passed into here, but we sanity */
	/* check JIC */
	if (header->bLength > size)
	{
		return -1;
	}

	if (header->bDescriptorType != USB_DT_ENDPOINT)
	{
		return parsed;
	}

	if (header->bLength == USB_DT_ENDPOINT_AUDIO_SIZE)
		memcpy(endpoint, buffer, USB_DT_ENDPOINT_AUDIO_SIZE);
	else
		memcpy(endpoint, buffer, USB_DT_ENDPOINT_SIZE);

	buffer += header->bLength;
	size -= header->bLength;
	parsed += header->bLength;

	/* Skip over the rest of the Class Specific or Vendor Specific */
	/* descriptors */
	begin = (uint32_t)buffer;
	numskipped = 0;
	while (size >= sizeof(struct usb_descriptor_header_t))
	{
		header = (struct usb_descriptor_header_t *)buffer;

		if (header->bLength < 2)
		{
			return -1;
		}

		/* If we find another "proper" descriptor then we're done */
		if ((header->bDescriptorType == USB_DT_ENDPOINT) ||
				(header->bDescriptorType == USB_DT_INTERFACE) ||
				(header->bDescriptorType == USB_DT_CONFIG) ||
				(header->bDescriptorType == USB_DT_DEVICE))
			break;

		numskipped++;

		buffer += header->bLength;
		size -= header->bLength;
		parsed += header->bLength;
	}

	len = (int)((uint32_t)buffer - begin);
	if (!len)
	{
		endpoint->extra = NULL;
		endpoint->extralen = 0;
	}
	else
	{
		endpoint->extra = (void *)begin;
		endpoint->extralen = len;
	}

	return parsed;
}

static int parse_interface(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev,
		struct usb_interface_t *interface, unsigned char *buffer, int size)
{
	int i, len, numskipped, retval, parsed = 0;
	struct usb_descriptor_header_t *header;
	struct usb_interface_desc_t *ifp;
	uint32_t begin;

	interface->act_altsetting = 0;
	interface->num_altsetting = 0;
	interface->max_altsetting = USB_ALTSETTINGALLOC;
	interface->altsetting = vsf_bufmgr_malloc\
			(sizeof(struct usb_interface_desc_t) * interface->max_altsetting);

	if (!interface->altsetting)
	{
		return -1;
	}

	while (size > 0)
	{
		if (interface->num_altsetting >= interface->max_altsetting)
		{
			void *ptr;
			int oldmas;

			oldmas = interface->max_altsetting;
			interface->max_altsetting += USB_ALTSETTINGALLOC;
#ifdef USB_MAXALTSETTING
			if (interface->max_altsetting > USB_MAXALTSETTING)
			{
				return -1;
			}
#endif

			ptr = interface->altsetting;
			interface->altsetting = vsf_bufmgr_malloc\
					(sizeof(struct usb_interface_desc_t) *
							interface->max_altsetting);
			if (!interface->altsetting)
			{
				interface->altsetting = ptr;
				return -1;
			}
			memcpy(interface->altsetting, ptr,
					sizeof(struct usb_interface_desc_t) * oldmas);
			vsf_bufmgr_free(ptr);
		}

		ifp = interface->altsetting + interface->num_altsetting;
		interface->num_altsetting++;

		memcpy(ifp, buffer, USB_DT_INTERFACE_SIZE);

		/* Skip over the interface */
		buffer += ifp->bLength;
		parsed += ifp->bLength;
		size -= ifp->bLength;

		begin = (uint32_t)buffer;
		numskipped = 0;

		/* Skip over any interface, class or vendor descriptors */
		while (size >= sizeof(struct usb_descriptor_header_t))
		{
			header = (struct usb_descriptor_header_t *)buffer;

			if (header->bLength < 2)
			{
				return -1;
			}

			/* If we find another "proper" descriptor then we're done */
			if ((header->bDescriptorType == USB_DT_INTERFACE) ||
					(header->bDescriptorType == USB_DT_ENDPOINT) ||
					(header->bDescriptorType == USB_DT_CONFIG) ||
					(header->bDescriptorType == USB_DT_DEVICE))
				break;

			numskipped++;

			buffer += header->bLength;
			parsed += header->bLength;
			size -= header->bLength;
		}

		len = (int)((uint32_t)buffer - begin);
		if (len)
		{
			ifp->extra = (void *)begin;
			ifp->extralen = len;
		}
		else
		{
			ifp->extra = NULL;
			ifp->extralen = 0;
		}

		/* Did we hit an unexpected descriptor? */
		header = (struct usb_descriptor_header_t *)buffer;
		if ((size >= sizeof(struct usb_descriptor_header_t)) &&
				((header->bDescriptorType == USB_DT_CONFIG) ||
						(header->bDescriptorType == USB_DT_DEVICE)))
			goto end;

		if (ifp->bNumEndpoints > USB_MAXENDPOINTS)
		{
			return -1;
		}

		if (ifp->bNumEndpoints != 0)
		{
			ifp->ep_desc = vsf_bufmgr_malloc(ifp->bNumEndpoints *
					sizeof(struct usb_endpoint_desc_t));
			if (!ifp->ep_desc)
			{
				return -1;
			}

			memset(ifp->ep_desc, 0, ifp->bNumEndpoints *
					sizeof(struct usb_endpoint_desc_t));

			for (i = 0; i < ifp->bNumEndpoints; i++)
			{
				header = (struct usb_descriptor_header_t *)buffer;

				if (header->bLength > size)
				{
					return -1;
				}

				retval = parse_endpoint(ifp->ep_desc + i, buffer, size);
				if (retval < 0)
					goto end;

				buffer += retval;
				parsed += retval;
				size -= retval;
			}
		}

		/* We check to see if it's an alternate to this one */
		ifp = (struct usb_interface_desc_t *)buffer;
		if (size < USB_DT_INTERFACE_SIZE ||
				ifp->bDescriptorType != USB_DT_INTERFACE ||
				!ifp->bAlternateSetting)
			goto end;
	}

end:
#if USBH_INTERFACE_RAM_OPTIMIZE
	if (vsfusbh_match_intrface_driver(usbh, dev, interface) != VSFERR_NONE)
	{
		uint32_t k;		
		ifp = interface->altsetting;		
		for (k = 0; k < interface->num_altsetting; k++)
		{
			if (ifp[k].ep_desc != NULL)
				vsf_bufmgr_free(ifp[k].ep_desc);
		}
		
		vsf_bufmgr_free(interface->altsetting);
		interface->altsetting = NULL;
		interface->act_altsetting = 0;
		interface->num_altsetting = 0;
	}
#endif
	return parsed;
}

static vsf_err_t parse_configuration(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct usb_config_t *config,
		uint8_t *buffer)
{
	int i, retval, size;
	struct usb_descriptor_header_t *header;

	memcpy(config, buffer, USB_DT_CONFIG_SIZE);
	size = config->wTotalLength;

#ifdef USB_MAXINTERFACES
	if (config->bNumInterfaces > USB_MAXINTERFACES)
		return VSFERR_FAIL;
#endif

	config->interface = vsf_bufmgr_malloc(sizeof(struct usb_interface_t) *
			config->bNumInterfaces);
	if (config->interface == NULL)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	memset(config->interface, 0, sizeof(struct usb_interface_t) *
			config->bNumInterfaces);

	buffer += config->bLength;
	size -= config->bLength;

	//config->extra = NULL;
	//config->extralen = 0;

	for (i = 0; i < config->bNumInterfaces; i++)
	{
		int32_t numskipped;
		//int32_t len;
		//uint8_t *begin;

		/* Skip over the rest of the Class Specific or Vendor */
		/* Specific descriptors */
		//begin = buffer;
		numskipped = 0;
		while (size >= sizeof(struct usb_descriptor_header_t))
		{
			header = (struct usb_descriptor_header_t *)buffer;

			if ((header->bLength > size) || (header->bLength < 2))
			{
				return VSFERR_FAIL;
			}

			/* If we find another "proper" descriptor then we're done */
			if ((header->bDescriptorType == USB_DT_ENDPOINT) ||
					(header->bDescriptorType == USB_DT_INTERFACE) ||
					(header->bDescriptorType == USB_DT_CONFIG) ||
					(header->bDescriptorType == USB_DT_DEVICE))
				break;

			numskipped++;

			buffer += header->bLength;
			size -= header->bLength;
		}

		/*
				len = (int32_t)(buffer - begin);
				if (len)
				{
					if (config->extralen == 0)
					{
						// info
					}
					else
					{
						config->extra = begin;
						config->extralen = len;
					}
				}
		*/

		retval = parse_interface(usbh, dev, config->interface + i, buffer, size);
		if (retval < 0)
			return VSFERR_FAIL;

		buffer += retval;
		size -= retval;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfusbh_probe_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	uint32_t len;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)pt->user_data;
	struct vsfusbh_urb_t *probe_urb = usbh->probe_urb;
	struct vsfusbh_device_t *dev = usbh->new_dev;

	vsfsm_pt_begin(pt);

	dev->devnum_temp = dev->devnum;
	dev->devnum = 0;
	dev->epmaxpacketin[0] = 8;
	dev->epmaxpacketout[0] = 8;

	probe_urb->vsfdev = dev;
	probe_urb->sm = &usbh->sm;
	probe_urb->timeout = DEFAULT_TIMEOUT;

	// get 8 bytes device descriptor
	probe_urb->transfer_buffer = &dev->descriptor;
	probe_urb->transfer_length = 8;
	err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_DEVICE, 0);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (probe_urb->status != URB_OK)
		return VSFERR_FAIL;
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	// set address
	dev->devnum = dev->devnum_temp;
	probe_urb->transfer_buffer = NULL;
	probe_urb->transfer_length = 0;
	err = vsfusbh_set_address(usbh, probe_urb);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (probe_urb->status != URB_OK)
		return VSFERR_FAIL;

	vsfsm_pt_delay(pt, 10);

	// get full device descriptor
	probe_urb->transfer_buffer = &dev->descriptor;
	probe_urb->transfer_length = sizeof(dev->descriptor);
	err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_DEVICE, 0);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (probe_urb->status != URB_OK)
		return VSFERR_FAIL;

	// NOTE: only probe first configuration
	if (dev->descriptor.bNumConfigurations < 1)
		return VSFERR_FAIL;
	dev->num_config = 1; // NOTE: not min(USB_MAXCONFIG, dev->descriptor.bNumConfigurations)
	len = sizeof(struct usb_config_t) * dev->num_config;
	dev->config = vsf_bufmgr_malloc_aligned(len, 4);
	if (dev->config == NULL)
	{
		vsf_bufmgr_free(probe_urb->transfer_buffer);
		return VSFERR_FAIL;
	}
	memset(dev->config, 0, sizeof(struct usb_config_t) * dev->num_config);

	for (dev->temp_u8 = 0; dev->temp_u8 < dev->num_config; dev->temp_u8++)
	{
		// get 9 bytes configuration
		probe_urb->transfer_buffer = vsf_bufmgr_malloc(9);
		if (probe_urb->transfer_buffer == NULL)
			goto get_config_fail;
		probe_urb->transfer_length = 9;
		err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_CONFIG,
				dev->temp_u8);
		if (err != VSFERR_NONE)
			goto get_config_fail;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (probe_urb->status != URB_OK)
			goto get_config_fail;
		// get wTotalLength
		len = GET_U16_LSBFIRST(&((uint8_t *)(probe_urb->transfer_buffer))[2]);
		vsf_bufmgr_free(probe_urb->transfer_buffer);

		// get full configuation
		probe_urb->transfer_buffer = vsf_bufmgr_malloc(len);
		if (probe_urb->transfer_buffer == NULL)
			goto get_config_fail;
		probe_urb->transfer_length = len;
		err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_CONFIG,
				dev->temp_u8);
		if (err != VSFERR_NONE)
			goto get_config_fail;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (probe_urb->status != URB_OK)
			goto get_config_fail;
		// check wTotalLength
		len = GET_U16_LSBFIRST(&((uint8_t *)(probe_urb->transfer_buffer))[2]);
		if (probe_urb->actual_length != len)
			goto get_config_fail;

		err = parse_configuration(usbh, dev, dev->config + dev->temp_u8,
				probe_urb->transfer_buffer);
		if (err != VSFERR_NONE)
			goto get_config_fail;
		dev->config[dev->temp_u8].config_buffer = probe_urb->transfer_buffer;
		probe_urb->transfer_buffer = NULL;
		continue;

get_config_fail:
		if (probe_urb->transfer_buffer != NULL)
		{
			vsf_bufmgr_free(probe_urb->transfer_buffer);
			probe_urb->transfer_buffer = NULL;
		}
		// NOTE: do not free dev->config here !!!
		return VSFERR_FAIL;
	}

	// set the default configuration
	probe_urb->transfer_buffer = NULL;
	probe_urb->transfer_length = 0;
	err = vsfusbh_set_configuration(usbh, probe_urb,
			dev->config->bConfigurationValue);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (probe_urb->status != URB_OK)
		return VSFERR_FAIL;

	dev->actconfig = dev->config;
	vsfusbh_set_maxpacket_ep(dev);

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static struct vsfsm_state_t *vsfusbh_probe_evt_handler(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_NEW_DEVICE:
		usbh->dev_probe_pt.thread = vsfusbh_probe_thread;
		usbh->dev_probe_pt.user_data = usbh;
		usbh->dev_probe_pt.state = 0;
		usbh->dev_probe_pt.sm = sm;
	default:
		err = usbh->dev_probe_pt.thread(&usbh->dev_probe_pt, evt);
		if (VSFERR_NONE == err)
		{
			err = vsfusbh_add_device(usbh, usbh->new_dev);
			if (err < 0)
			{
				vsfusbh_free_device(usbh, usbh->new_dev);
			}
			usbh->new_dev = NULL;
		}
		else if (err < 0)
		{
			vsfusbh_free_device(usbh, usbh->new_dev);
			usbh->new_dev = NULL;
		}
		break;
	}
	return NULL;
}

static struct vsfsm_state_t *vsfusbh_init_evt_handler(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_ENTER:
		break;
	case VSFSM_EVT_INIT:
		usbh->dev_probe_pt.thread = NULL;
		sllist_init_node(usbh->drv_list);
		usbh->hcd_init_pt.thread = usbh->hcd->init_thread;
		usbh->hcd_init_pt.user_data = usbh;
		usbh->hcd_init_pt.state = 0;
		usbh->hcd_init_pt.sm = sm;
	default:
		err = usbh->hcd_init_pt.thread(&usbh->hcd_init_pt, evt);
		if (VSFERR_NONE == err)
		{
			// alloc probe urb
			usbh->probe_urb = usbh->hcd->alloc_urb();
			if (usbh->probe_urb == NULL)
			{
				// error
				usbh->hcd_init_pt.thread = NULL;
			}

			sm->init_state.evt_handler = vsfusbh_probe_evt_handler;
			usbh->rh_dev = vsfusbh_alloc_device(usbh);
			if (NULL == usbh->rh_dev)
			{
				// error
				usbh->hcd_init_pt.thread = NULL;
			}
			else
			{
				usbh->rh_dev->speed = usbh->hcd_rh_speed;
				if (usbh->rh_dev->speed == USB_SPEED_LOW)
					usbh->rh_dev->slow = 1;
				usbh->new_dev = usbh->rh_dev;
				vsfsm_post_evt_pending(&usbh->sm, VSFSM_EVT_NEW_DEVICE);
			}
		}
		else if (err < 0)
		{
			// error
			usbh->hcd_init_pt.thread = NULL;
		}
		break;
	}
	return NULL;
}

vsf_err_t vsfusbh_init(struct vsfusbh_t *usbh)
{
	if (NULL == usbh->hcd)
	{
		return VSFERR_INVALID_PARAMETER;
	}

	memset(&usbh->sm, 0, sizeof(usbh->sm));
	usbh->device_bitmap[0] = 1;		// remove address 0
	usbh->device_bitmap[1] = 0;
	usbh->device_bitmap[2] = 0;
	usbh->device_bitmap[3] = 0;
	usbh->sm.init_state.evt_handler = vsfusbh_init_evt_handler;
	usbh->sm.user_data = (void*)usbh;
	return vsfsm_init(&usbh->sm);
}

vsf_err_t vsfusbh_fini(struct vsfusbh_t *usbh)
{
	// TODO
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_register_driver(struct vsfusbh_t *usbh,
		const struct vsfusbh_class_drv_t *drv)
{
	struct vsfusbh_class_drv_list *drv_list;

	drv_list = (struct vsfusbh_class_drv_list *)\
			vsf_bufmgr_malloc(sizeof(struct vsfusbh_class_drv_list));
	if (drv_list == NULL)
	{
		return VSFERR_FAIL;
	}
	memset(drv_list, 0, sizeof(struct vsfusbh_class_drv_list));
	drv_list->drv = drv;
	sllist_append(&usbh->drv_list, &drv_list->list);
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_get_extra_descriptor(uint8_t *buf, uint16_t size,
		uint8_t type, void **ptr)
{
	struct usb_descriptor_header_t *header;

	if ((buf == NULL) || (ptr == NULL))
		return VSFERR_FAIL;

	while (size >= sizeof(struct usb_descriptor_header_t))
	{
		header = (struct usb_descriptor_header_t *)buf;

		if (header->bLength < 2)
			break;

		if (header->bDescriptorType == type)
		{
			*ptr = header;
			return VSFERR_NONE;
		}

		if (size < header->bLength)
			break;

		buf += header->bLength;
		size -= header->bLength;
	}
	return VSFERR_FAIL;
}

#ifdef VSFCFG_STANDALONE_MODULE
vsf_err_t vsfusbh_modexit(struct vsf_module_t *module)
{
	vsf_bufmgr_free(module->ifs);
	module->ifs = NULL;
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_modinit(struct vsf_module_t *module,
								struct app_hwcfg_t const *cfg)
{
	struct vsfusbh_modifs_t *ifs;
	ifs = vsf_bufmgr_malloc(sizeof(struct vsfusbh_modifs_t));
	if (!ifs) return VSFERR_FAIL;
	memset(ifs, 0, sizeof(*ifs));

	ifs->init = vsfusbh_init;
	ifs->fini = vsfusbh_fini;
	ifs->register_driver = vsfusbh_register_driver;
	ifs->submit_urb = vsfusbh_submit_urb;
	ifs->relink_urb = vsfusbh_relink_urb;
	module->ifs = ifs;
	return VSFERR_NONE;
}
#endif

