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

struct vsfusbh_hub_t
{
	struct vsfsm_t sm;
	struct vsfsm_pt_t drv_init_pt;
	struct vsfsm_pt_t drv_scan_pt;
	struct vsfsm_pt_t drv_connect_pt;
	struct vsfsm_pt_t drv_reset_pt;

	struct vsfusbh_t *usbh;
	struct vsfusbh_device_t *dev;
	struct usb_interface_t *interface;

	struct vsfusbh_urb_t *vsfurb;

	struct usb_hub_descriptor_t hub_desc;
	//struct usb_hub_status_t hub_status;
	struct usb_port_status_t hub_portsts;

	int16_t counter;
	int16_t retry;
	int16_t inited;
};

static vsf_err_t hub_set_port_feature(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint16_t port, uint16_t feature)
{
	vsfurb->pipe = usb_sndctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_RT_PORT, USB_REQ_SET_FEATURE,
			feature, port);
}
static vsf_err_t hub_get_port_status(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, uint16_t port)
{
	vsfurb->pipe = usb_rcvctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_DIR_IN | USB_RT_PORT,
			USB_REQ_GET_STATUS, 0, port);
}
static vsf_err_t hub_clear_port_feature(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb, int port, int feature)
{
	vsfurb->pipe = usb_sndctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_RT_PORT, USB_REQ_CLEAR_FEATURE,
			feature, port);
}
#if 0
static vsf_err_t hub_get_status(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb)
{
	vsfurb->pipe = usb_rcvctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_DIR_IN | USB_RT_HUB,
			USB_REQ_GET_STATUS, 0, 0);
}
#endif

static vsf_err_t hub_reset_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfusbh_urb_t *vsfurb = hub->vsfurb;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	hub->retry = 0;

	vsfsm_pt_delay(pt, 50);
	
	do
	{
		/* send command to reset port */
		vsfurb->transfer_buffer = NULL;
		vsfurb->transfer_length = 0;
		err = hub_set_port_feature(hub->usbh, vsfurb, hub->counter,
				USB_PORT_FEAT_RESET);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (vsfurb->status != URB_OK)
			return VSFERR_FAIL;

		/* delay 100ms after port reset*/
		vsfsm_pt_delay(pt, 100);

		// clear reset
		vsfurb->transfer_buffer = NULL;
		vsfurb->transfer_length = 0;
		err = hub_clear_port_feature(hub->usbh, vsfurb,
				hub->counter, USB_PORT_FEAT_C_RESET);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (vsfurb->status != URB_OK)
			return VSFERR_FAIL;

		/* delay 100ms after port reset*/
		vsfsm_pt_delay(pt, 50);

		/* get port status for check */
		vsfurb->transfer_buffer = &hub->hub_portsts;
		vsfurb->transfer_length = sizeof(hub->hub_portsts);
		err = hub_get_port_status(hub->usbh, vsfurb, hub->counter);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (vsfurb->status != URB_OK)
			return VSFERR_FAIL;

		/* check port status after reset */
		if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_ENABLE)
		{
			return VSFERR_NONE;
		}
		else
		{
			/* delay 200ms for next reset*/
			vsfsm_pt_delay(pt, 200);
		}

	} while (hub->retry++ <= 3);

	vsfsm_pt_end(pt);
	return VSFERR_FAIL;
}

static vsf_err_t hub_connect_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfusbh_urb_t *vsfurb = hub->vsfurb;
	struct vsfusbh_device_t *dev;

	vsfsm_pt_begin(pt);

	/* clear the cnnection change state */
	vsfurb->transfer_buffer = NULL;
	vsfurb->transfer_length = 0;
	err = hub_clear_port_feature(hub->usbh, vsfurb, hub->counter,
			USB_PORT_FEAT_C_CONNECTION);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (vsfurb->status != URB_OK)
		return VSFERR_FAIL;

	if (hub->dev->children[hub->counter - 1] != NULL)
	{
		vsfusbh_disconnect_device(hub->usbh,
				&hub->dev->children[hub->counter - 1]);
	}

	if (!(hub->hub_portsts.wPortStatus & USB_PORT_STAT_CONNECTION))
	{
		if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_ENABLE)
		{
			vsfurb->transfer_buffer = NULL;
			vsfurb->transfer_length = 0;
			err = hub_clear_port_feature(hub->usbh, vsfurb,
					hub->counter, USB_PORT_FEAT_ENABLE);
			if (err != VSFERR_NONE)
				return err;
			vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
			if (vsfurb->status != URB_OK)
				return VSFERR_FAIL;
		}

		return VSFERR_NONE;
	}

	if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_LOW_SPEED)
	{
		vsfsm_pt_delay(pt, 200);
	}

	vsfsm_pt_wfpt(pt, &hub->drv_reset_pt);

	vsfsm_pt_delay(pt, 200);

	// wait for new_dev free
	while (hub->usbh->new_dev != NULL)
	{
		vsfsm_pt_delay(pt, 200);
	}

	dev = vsfusbh_alloc_device(hub->usbh);
	if (NULL == dev)
		return VSFERR_FAIL;

	if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_LOW_SPEED)
	{
		dev->speed = USB_SPEED_LOW;
		dev->slow = 1;
	}
	else if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_HIGH_SPEED)
		dev->speed = USB_SPEED_HIGH;
	else
		dev->speed = USB_SPEED_FULL;

	hub->dev->children[hub->counter - 1] = dev;
	dev->parent = hub->dev;

	hub->usbh->new_dev = dev;
	vsfsm_post_evt_pending(&hub->usbh->sm, VSFSM_EVT_NEW_DEVICE);

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static vsf_err_t hub_scan_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfusbh_urb_t *vsfurb = hub->vsfurb;

	vsfsm_pt_begin(pt);

	do
	{
		hub->counter = 1;
		do
		{
			// get port status
			vsfurb->transfer_buffer = &hub->hub_portsts;
			vsfurb->transfer_length = sizeof(hub->hub_portsts);
			err = hub_get_port_status(hub->usbh, vsfurb, hub->counter);
			if (err != VSFERR_NONE)
				return err;
			vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
			if (vsfurb->status != URB_OK)
				return VSFERR_FAIL;

			if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_CONNECTION)
			{
				// try to connect
				vsfsm_pt_wfpt(pt, &hub->drv_connect_pt);
			}
			else if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_ENABLE)
			{
				vsfurb->transfer_buffer = NULL;
				vsfurb->transfer_length = 0;
				err = hub_clear_port_feature(hub->usbh, vsfurb,
						hub->counter, USB_PORT_FEAT_C_ENABLE);
				if (err != VSFERR_NONE)
					return err;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (vsfurb->status != URB_OK)
					return VSFERR_FAIL;

				hub->hub_portsts.wPortChange &= ~USB_PORT_STAT_C_ENABLE;
			}

			if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_SUSPEND)
			{
				vsfurb->transfer_buffer = NULL;
				vsfurb->transfer_length = 0;
				err = hub_clear_port_feature(hub->usbh, vsfurb,
						hub->counter, USB_PORT_FEAT_C_SUSPEND);
				if (err != VSFERR_NONE)
					return err;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (vsfurb->status != URB_OK)
					return VSFERR_FAIL;

				hub->hub_portsts.wPortChange &= ~USB_PORT_STAT_C_SUSPEND;
			}
			if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_OVERCURRENT)
			{
				vsfurb->transfer_buffer = NULL;
				vsfurb->transfer_length = 0;
				err = hub_clear_port_feature(hub->usbh, vsfurb,
						hub->counter, USB_PORT_FEAT_C_OVER_CURRENT);
				if (err != VSFERR_NONE)
					return err;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (vsfurb->status != URB_OK)
					return VSFERR_FAIL;

				hub->hub_portsts.wPortChange &= ~USB_PORT_FEAT_C_OVER_CURRENT;

				// TODO : power every port
			}
			if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_RESET)
			{
				vsfurb->transfer_buffer = NULL;
				vsfurb->transfer_length = 0;
				err = hub_clear_port_feature(hub->usbh, vsfurb,
						hub->counter, USB_PORT_FEAT_C_RESET);
				if (err != VSFERR_NONE)
					return err;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (vsfurb->status != URB_OK)
					return VSFERR_FAIL;

				hub->hub_portsts.wPortChange &= ~USB_PORT_FEAT_C_RESET;
			}
		} while (hub->counter++ < hub->dev->maxchild);

		// TODO : poll hub status

		vsfsm_pt_delay(pt, 200);
	} while (1);
	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static vsf_err_t hub_class_check(struct vsfusbh_hub_t *hub)
{
	struct vsfusbh_device_t *dev = hub->dev;
	struct usb_interface_desc_t *iface;
	struct usb_endpoint_desc_t *ep;

	if (dev == NULL)
		return VSFERR_FAIL;

	iface = dev->config->interface->altsetting;
	ep = iface->ep_desc;
	if ((iface->bInterfaceClass != USB_CLASS_HUB) ||
			(iface->bInterfaceSubClass > 1) ||
			(iface->bNumEndpoints != 1) ||
			(!(ep->bEndpointAddress & USB_DIR_IN)) ||
			((ep->bmAttributes & USB_ENDPOINT_XFER_INT) != USB_ENDPOINT_XFER_INT))
		return VSFERR_FAIL;

	return VSFERR_NONE;
}

static vsf_err_t hub_get_descriptor(struct vsfusbh_t *usbh,
		struct vsfusbh_urb_t *vsfurb)
{
	vsfurb->pipe = usb_rcvctrlpipe(vsfurb->vsfdev, 0);
	return vsfusbh_control_msg(usbh, vsfurb, USB_DIR_IN | USB_RT_HUB,
			USB_REQ_GET_DESCRIPTOR, (USB_DT_HUB << 8), 0);
}

static vsf_err_t hub_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfusbh_urb_t *vsfurb = hub->vsfurb;

	vsfsm_pt_begin(pt);

	vsfurb->sm = &hub->sm;

	// init hub
	err = hub_class_check(hub);
	if (err != VSFERR_NONE)
		return err;

	vsfurb->transfer_buffer = &hub->hub_desc;
	vsfurb->transfer_length = 4;
	err = hub_get_descriptor(hub->usbh, vsfurb);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (vsfurb->status != URB_OK)
		return VSFERR_FAIL;

	if (hub->hub_desc.bDescLength > sizeof(hub->hub_desc))
	{
		return VSFERR_FAIL;
	}

	vsfurb->transfer_buffer = &hub->hub_desc;
	vsfurb->transfer_length = hub->hub_desc.bDescLength;
	err = hub_get_descriptor(hub->usbh, vsfurb);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (vsfurb->status != URB_OK)
		return VSFERR_FAIL;

	hub->dev->maxchild = min(hub->hub_desc.bNbrPorts, USB_MAXCHILDREN);
#if 0
	vsfurb->transfer_buffer = &hub->hub_status;
	vsfurb->transfer_length = sizeof(hub->hub_status);
	err = hub_get_status(hub->usbh, vsfurb);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (vsfurb->status != URB_OK)
		return VSFERR_FAIL;
#endif
	hub->counter = 0;

	do
	{
		hub->counter++;
		hub->vsfurb->transfer_buffer = NULL;
		hub->vsfurb->transfer_length = 0;
		err = hub_set_port_feature(hub->usbh, hub->vsfurb, hub->counter,
				USB_PORT_FEAT_POWER);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (vsfurb->status != URB_OK)
			return VSFERR_FAIL;
		vsfsm_pt_delay(pt, hub->hub_desc.bPwrOn2PwrGood * 2);
	} while (hub->counter < hub->dev->maxchild);

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static struct vsfsm_state_t *vsfusbh_hub_evt_handler_init(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		hub->drv_init_pt.thread = hub_init_thread;
		hub->drv_init_pt.user_data = hub;
		hub->drv_init_pt.sm = sm;
		hub->drv_init_pt.state = 0;
		hub->drv_scan_pt.thread = hub_scan_thread;
		hub->drv_scan_pt.user_data = hub;
		hub->drv_scan_pt.sm = sm;
		hub->drv_scan_pt.state = 0;
		hub->drv_connect_pt.thread = hub_connect_thread;
		hub->drv_connect_pt.user_data = hub;
		hub->drv_connect_pt.sm = &hub->sm;
		hub->drv_connect_pt.state = 0;
		hub->drv_reset_pt.thread = hub_reset_thread;
		hub->drv_reset_pt.user_data = hub;
		hub->drv_reset_pt.sm = &hub->sm;
		hub->drv_reset_pt.state = 0;
		hub->inited = 0;

	case VSFSM_EVT_URB_COMPLETE:
	case VSFSM_EVT_DELAY_DONE:
		if (hub->inited == 0)
		{
			err = hub->drv_init_pt.thread(&hub->drv_init_pt, evt);
			if (err == 0)
			{
				hub->inited = 1;
				vsfsm_post_evt_pending(sm, VSFSM_EVT_DELAY_DONE);
			}
			if (err < 0)
			{
				vsfusbh_remove_interface(hub->usbh, hub->dev, hub->interface);
			}
		}
		else
		{
			err = hub->drv_scan_pt.thread(&hub->drv_scan_pt, evt);
			if (err < 0)
			{
				hub->drv_scan_pt.state = 0;
				vsfsm_post_evt_pending(sm, VSFSM_EVT_DELAY_DONE);
			}
		}
		break;
	default:
		break;
	}
	return NULL;
}

static void *vsfusbh_hub_probe(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct usb_interface_t *interface,
		const struct vsfusbh_device_id_t *id)
{
	struct usb_interface_desc_t *intf_desc;
	struct usb_endpoint_desc_t *ep_desc;
	struct vsfusbh_hub_t *hub;

	intf_desc = interface->altsetting + interface->act_altsetting;

	if ((intf_desc->bInterfaceSubClass != 0) &&
			(intf_desc->bInterfaceSubClass != 1))
		return NULL;

	if (intf_desc -> bNumEndpoints != 1)
		return NULL;

	ep_desc = intf_desc->ep_desc;
	if ((ep_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) !=
			USB_ENDPOINT_XFER_INT)
		return NULL;

	hub = vsf_bufmgr_malloc(sizeof(struct vsfusbh_hub_t));
	if (NULL == hub)
		return NULL;
	memset(hub, 0, sizeof(struct vsfusbh_hub_t));

	hub->vsfurb = usbh->hcd->alloc_urb();
	if (hub->vsfurb == NULL)
	{
		vsf_bufmgr_free(hub);
		return NULL;
	}

	hub->usbh = usbh;
	hub->dev = dev;
	hub->vsfurb->vsfdev = dev;
	hub->vsfurb->timeout = 200;
	hub->sm.init_state.evt_handler = vsfusbh_hub_evt_handler_init;
	hub->sm.user_data = hub;
	vsfsm_init(&hub->sm);

	return hub;
}

static void vsfusbh_hub_disconnect(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, void *priv)
{
	struct vsfusbh_hub_t *hub = priv;

	if (hub->vsfurb != NULL)
		usbh->hcd->free_urb(usbh->hcd_data, &hub->vsfurb);

	vsfsm_fini(&hub->sm);
	vsf_bufmgr_free(hub);
}

static const struct vsfusbh_device_id_t vsfusbh_hub_id_table[] =
{
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_HUB,
	},
	{0},
};

#ifdef VSFCFG_STANDALONE_MODULE
vsf_err_t vsfusbh_hub_modexit(struct vsf_module_t *module)
{
	vsf_bufmgr_free(module->ifs);
	module->ifs = NULL;
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_hub_modinit(struct vsf_module_t *module,
								struct app_hwcfg_t const *cfg)
{
	struct vsfusbh_hub_modifs_t *ifs;
	ifs = vsf_bufmgr_malloc(sizeof(struct vsfusbh_hub_modifs_t));
	if (!ifs) return VSFERR_FAIL;
	memset(ifs, 0, sizeof(*ifs));

	ifs->drv.name = "hub";
	ifs->drv.id_table = vsfusbh_hub_id_table;
	ifs->drv.probe = vsfusbh_hub_probe;
	ifs->drv.disconnect = vsfusbh_hub_disconnect;
	module->ifs = ifs;
	return VSFERR_NONE;
}
#else
const struct vsfusbh_class_drv_t vsfusbh_hub_drv =
{
	.name = "hub",
	.id_table = vsfusbh_hub_id_table,
	.probe = vsfusbh_hub_probe,
	.disconnect = vsfusbh_hub_disconnect,
};
#endif
