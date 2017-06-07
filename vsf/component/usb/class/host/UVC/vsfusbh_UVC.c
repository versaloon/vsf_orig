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

#undef vsfusbh_uvc_report
#undef vsfusbh_uvc_set

#include "stack/usb/class/host/UVC/vsfusbh_UVC.h"

#define UVC_PROBE_CRTL_DATA_SIZE 36

enum uav_evt_t
{
	UAV_RESET_STREAM_PARAM = VSFSM_EVT_USER_LOCAL + 1,
	UAV_ISO_ENABLE,
	UAV_ISO_DISABLE,
};

enum uav_request_t
{
	RC_UNDEFINED = 0x00,
	SET_CUR = 0x01,
	GET_CUR = 0x81,
	GET_MIN = 0x82,
	GET_MAX = 0x83,
	GET_RES = 0x84,
	GET_LEN = 0x85,
	GET_INFO = 0x86,
	GET_DEF = 0x87,
};

struct vsfusbh_uvc_t
{
	struct vsfusbh_t *usbh;
	struct vsfusbh_device_t *dev;

	struct vsfsm_t init_sm;
	struct vsfsm_pt_t init_pt;
	struct vsfsm_t ctrl_sm;
	struct vsfsm_pt_t ctrl_pt;
	struct vsfsm_t video_sm;
	//struct vsfsm_t audio_sm;

	struct vsfusbh_urb_t ctrl_urb;
	struct vsfusbh_urb_t video_urb;
	//struct vsfusbh_urb_t audio_urb;

	uint8_t *ctrl_urb_buf;
	uint8_t *video_urb_buf;
	//uint8_t *audio_urb_buf;

	struct vsfusbh_uvc_payload_t video_payload;
	//struct vsfusbh_uvc_payload_t audio_payload;

	struct vsfusbh_uvc_param_t set_param;
	struct vsfusbh_uvc_param_t cur_param;

	uint16_t video_iso_packet_len;
	//uint16_t audio_iso_packet_len;
	uint8_t video_iso_ep;
	//uint8_t audio_iso_ep;

};

#ifndef VSFCFG_STANDALONE_MODULE
void (*vsfusbh_uvc_report)(void *dev_data, struct vsfusbh_uvc_param_t *param,
		struct vsfusbh_uvc_payload_t *payload) = NULL;
#endif

static vsf_err_t uvc_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *hdata = (struct vsfusbh_uvc_t *)pt->user_data;
	struct vsfusbh_urb_t *vsfurb = &hdata->ctrl_urb;

	vsfsm_pt_begin(pt);

	// reset interfaces 1 (video)
	vsfurb->transfer_buffer = NULL;
	vsfurb->transfer_length = 0;
	err = vsfusbh_set_interface(hdata->usbh, vsfurb, 1, 0);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (vsfurb->status != URB_OK)
		return VSFERR_FAIL;

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static struct vsfsm_state_t *uvc_evt_handler_init(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *hdata = (struct vsfusbh_uvc_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
	case VSFSM_EVT_URB_COMPLETE:
	case VSFSM_EVT_DELAY_DONE:
		err = hdata->init_pt.thread(&hdata->init_pt, evt);
		if (err < 0)
		{
			// TODO
		}
		else if (err == 0)
		{
			hdata->ctrl_urb.sm = &hdata->ctrl_sm;

			if (vsfusbh_uvc_report)
				vsfusbh_uvc_report(hdata, &hdata->cur_param, NULL);
		}
		break;
	default:
		break;
	}
	return NULL;
}

static const uint8_t negotiate_temp[26] =
{
	0x00, 0x00, 0x01, 0x03, 0x15, 0x16, 0x05, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

static vsf_err_t uvc_ctrl_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *hdata = (struct vsfusbh_uvc_t *)pt->user_data;
	struct vsfusbh_urb_t *vsfurb = &hdata->ctrl_urb;

	vsfsm_pt_begin(pt);

	if (hdata->set_param.video_enable)
	{
		// negotiate
		hdata->video_iso_packet_len = 1024;
		hdata->video_iso_ep = 1;

		// commit param
		vsfurb->transfer_buffer = hdata->ctrl_urb_buf;
		memcpy(vsfurb->transfer_buffer, negotiate_temp, 26);
		vsfurb->transfer_length = 26;
		vsfurb->pipe = usb_sndctrlpipe(vsfurb->vsfdev, 0);
		err =  vsfusbh_control_msg(hdata->usbh, vsfurb,
				USB_TYPE_CLASS |USB_RECIP_INTERFACE | USB_DIR_OUT, SET_CUR,
				0x0200, 0x0001);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (vsfurb->status != URB_OK)
			return VSFERR_FAIL;

		// set interfaces
		vsfurb->transfer_buffer = NULL;
		vsfurb->transfer_length = 0;
		err = vsfusbh_set_interface(hdata->usbh, vsfurb, 1, 4);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (vsfurb->status != URB_OK)
			return VSFERR_FAIL;

		// enable video
		vsfsm_post_evt_pending(&hdata->video_sm, UAV_ISO_ENABLE);
	}
	else
	{
		vsfurb->transfer_buffer = NULL;
		vsfurb->transfer_length = 0;
		err = vsfusbh_set_interface(hdata->usbh, vsfurb, 1, 0);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (vsfurb->status != URB_OK)
			return VSFERR_FAIL;
	}

	vsf_bufmgr_free(hdata->ctrl_urb_buf);
	hdata->ctrl_urb_buf = NULL;

	memcpy(&hdata->cur_param, &hdata->set_param,
			sizeof(struct vsfusbh_uvc_param_t));
	vsfusbh_uvc_report(hdata, &hdata->cur_param, NULL);

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static struct vsfsm_state_t *uvc_evt_handler_ctrl(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *hdata = (struct vsfusbh_uvc_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		break;
	case UAV_RESET_STREAM_PARAM:
		hdata->ctrl_pt.state = 0;
		if (hdata->ctrl_urb_buf == NULL)
		{
			hdata->ctrl_urb_buf = vsf_bufmgr_malloc(UVC_PROBE_CRTL_DATA_SIZE);
			if (hdata->ctrl_urb_buf == NULL)
				return NULL;
		}
	case VSFSM_EVT_URB_COMPLETE:
	case VSFSM_EVT_DELAY_DONE:
		err = hdata->ctrl_pt.thread(&hdata->ctrl_pt, evt);
		if (err < 0)
		{
			// TODO

			vsf_bufmgr_free(hdata->ctrl_urb_buf);
			hdata->ctrl_urb_buf = NULL;
		}
		else if (err == 0)
		{
			vsf_bufmgr_free(hdata->ctrl_urb_buf);
			hdata->ctrl_urb_buf = NULL;
		}
		break;
	default:
		break;
	}
	return NULL;
}

static struct vsfsm_state_t *uvc_evt_handler_video(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *hdata = (struct vsfusbh_uvc_t *)sm->user_data;
	struct vsfusbh_t *usbh = hdata->usbh;
	struct vsfusbh_urb_t *vsfurb = &hdata->video_urb;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		break;
	case UAV_ISO_ENABLE:
		if (hdata->video_urb_buf == NULL)
		{
			vsfurb->vsfdev->epmaxpacketin[hdata->video_iso_ep] = hdata->video_iso_packet_len;
			hdata->video_urb_buf = vsf_bufmgr_malloc(hdata->video_iso_packet_len);
			if (hdata->video_urb_buf == NULL)
				return NULL;
			vsfurb->transfer_buffer = hdata->video_urb_buf;
			vsfurb->transfer_length = hdata->video_iso_packet_len;
			vsfurb->pipe = usb_rcvisocpipe(vsfurb->vsfdev, hdata->video_iso_ep);
			vsfurb->transfer_flags |= USB_ISO_ASAP;
			vsfurb->number_of_packets = 1;
			vsfurb->iso_frame_desc[0].offset = 0;
			vsfurb->iso_frame_desc[0].length = hdata->video_iso_packet_len;
			err = vsfusbh_submit_urb(usbh, vsfurb);
			if (err != VSFERR_NONE)
				goto error;
		}
		break;
	case UAV_ISO_DISABLE:
		// TODO
		break;
	case VSFSM_EVT_URB_COMPLETE:
		if (vsfurb->status == URB_OK)
		{
			hdata->video_payload.len = vsfurb->actual_length;
			vsfusbh_uvc_report(hdata, &hdata->cur_param, &hdata->video_payload);
		}
		else
		{
			goto error;
		}
		err = vsfusbh_relink_urb(usbh, vsfurb);
		if (err != VSFERR_NONE)
			goto error;
		break;
	default:
		break;
	}
	return NULL;

error:
	vsf_bufmgr_free(hdata->video_urb_buf);
	hdata->video_urb_buf = NULL;
	return NULL;
}

static void *vsfusbh_uvc_init(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev)
{
	struct vsfusbh_class_data_t *cdata;
	struct vsfusbh_uvc_t *hdata;

	cdata = vsf_bufmgr_malloc(sizeof(struct vsfusbh_class_data_t));
	if (NULL == cdata)
		return NULL;

	hdata = vsf_bufmgr_malloc(sizeof(struct vsfusbh_uvc_t));
	if (NULL == hdata)
	{
		vsf_bufmgr_free(cdata);
		return NULL;
	}
	memset(cdata, 0, sizeof(struct vsfusbh_class_data_t));
	memset(hdata, 0, sizeof(struct vsfusbh_uvc_t));

	cdata->dev = dev;
	hdata->dev = dev;
	hdata->usbh = usbh;
	cdata->param = hdata;

	hdata->video_payload.type = VSFUSBH_UVC_PAYLOAD_VIDEO;
	hdata->video_payload.buf = hdata->video_urb_buf;

	hdata->init_sm.init_state.evt_handler = uvc_evt_handler_init;
	hdata->init_sm.user_data = (void*)hdata;
	hdata->init_pt.thread = uvc_init_thread;
	hdata->init_pt.user_data = hdata;
	hdata->init_pt.sm = &hdata->init_sm;
	hdata->init_pt.state = 0;

	hdata->ctrl_sm.init_state.evt_handler = uvc_evt_handler_ctrl;
	hdata->ctrl_sm.user_data = (void*)hdata;
	hdata->ctrl_pt.thread = uvc_ctrl_thread;
	hdata->ctrl_pt.user_data = hdata;
	hdata->ctrl_pt.sm = &hdata->ctrl_sm;
	hdata->ctrl_pt.state = 0;

	hdata->ctrl_urb.vsfdev = dev;
	hdata->ctrl_urb.timeout = 200;
	hdata->ctrl_urb.sm = &hdata->init_sm;

	hdata->video_sm.init_state.evt_handler = uvc_evt_handler_video;
	hdata->video_sm.user_data = (void*)hdata;
	hdata->video_urb.vsfdev = dev;
	hdata->video_urb.timeout = 200;
	hdata->video_urb.sm = &hdata->video_sm;

	vsfsm_init(&hdata->init_sm);
	vsfsm_init(&hdata->ctrl_sm);
	vsfsm_init(&hdata->video_sm);

	return cdata;
}

static vsf_err_t vsfusbh_uvc_match(struct vsfusbh_device_t *dev)
{
	if ((dev->descriptor.idVendor == 0x04f2) &&
			(dev->descriptor.idProduct == 0xb130))
	{
		return VSFERR_NONE;
	}

	return VSFERR_FAIL;
}

static void vsfusbh_uvc_free(struct vsfusbh_device_t *dev)
{
	struct vsfusbh_class_data_t *cdata = (struct vsfusbh_class_data_t *)(dev->priv);
	struct vsfusbh_uvc_t *hdata = (struct vsfusbh_uvc_t *)cdata->param;

	// TODO
	// free urb

	vsf_bufmgr_free(hdata);
	vsf_bufmgr_free(cdata);
}

vsf_err_t vsfusbh_uvc_set(void *dev_data, struct vsfusbh_uvc_param_t *param)
{
	struct vsfusbh_uvc_t *hdata = (struct vsfusbh_uvc_t *)dev_data;

	hdata->set_param = *param;
	vsfsm_post_evt_pending(&hdata->ctrl_sm, UAV_RESET_STREAM_PARAM);
	return VSFERR_NONE;
}

#ifdef VSFCFG_STANDALONE_MODULE
vsf_err_t vsfusbh_ucv_modexit(struct vsf_module_t *module)
{
	vsf_bufmgr_free(module->ifs);
	module->ifs = NULL;
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_uvc_modinit(struct vsf_module_t *module,
								struct app_hwcfg_t const *cfg)
{
	struct vsfusbh_uvc_modifs_t *ifs;
	ifs = vsf_bufmgr_malloc(sizeof(struct vsfusbh_uvc_modifs_t));
	if (!ifs) return VSFERR_FAIL;
	memset(ifs, 0, sizeof(*ifs));

	ifs->drv.name = "uvc";
	ifs->set = vsfusbh_uvc_set;
	ifs->report = vsfusbh_uvc_report;
	module->ifs = ifs;
	return VSFERR_NONE;
}
#else
const struct vsfusbh_class_drv_t vsfusbh_uvc_drv =
{
	vsfusbh_uvc_init,
	vsfusbh_uvc_free,
	vsfusbh_uvc_match,
};
#endif
