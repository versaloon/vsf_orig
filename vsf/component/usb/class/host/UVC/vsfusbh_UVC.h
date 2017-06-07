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

#ifndef __VSFUSBH_UVC_H_INCLUDED__
#define __VSFUSBH_UVC_H_INCLUDED__

#define VSFUSBH_UVC_VIDEO_FORMAT_MJPEG	0x1
#define VSFUSBH_UVC_VIDEO_FORMAT_YUY2	0x2
#define VSFUSBH_UVC_VIDEO_FORMAT_RGB24	0x3

enum VSFUSBH_UVC_PAYLOAD_TYPE_T
{
	VSFUSBH_UVC_PAYLOAD_VIDEO = 0x01,
	VSFUSBH_UVC_PAYLOAD_AUDIO = 0x02,
};

struct vsfusbh_uvc_payload_t
{
	uint32_t type : 8;
	uint32_t len : 24;
	uint8_t *buf;
};

struct vsfusbh_uvc_param_t
{
	uint8_t connected : 1;
	uint8_t video_enable : 1;
	uint8_t audio_enable : 1;
	uint8_t video_format : 3;
	uint8_t : 2;

	uint8_t video_fps;
	uint16_t video_width;
	uint16_t video_height;
};

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFUSBH_UVC_MODNAME					"vsf.stack.usb.host.uvc"

struct vsfusbh_uvc_modifs_t
{
	struct vsfusbh_class_drv_t drv;
	vsf_err_t (*set)(void*, struct vsfusbh_uvc_param_t*);
	void (*report)(void*, struct vsfusbh_uvc_param_t*,
						struct vsfusbh_uvc_payload_t*);
};

vsf_err_t vsfusbh_uvc_modexit(struct vsf_module_t*);
vsf_err_t vsfusbh_uvc_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFUSBH_UVCMOD						\
	((struct vsfusbh_uvc_modifs_t *)vsf_module_load(VSFUSBH_UVC_MODNAME, true))
#define vsfusbh_uvc_drv						VSFUSBH_UVCMOD->drv
#define vsfusbh_uvc_set						VSFUSBH_UVCMOD->set
#define vsfusbh_uvc_report					VSFUSBH_UVCMOD->report

#else
vsf_err_t vsfusbh_uvc_set(void *dev_data, struct vsfusbh_uvc_param_t *param);

extern void (*vsfusbh_uvc_report)(void *dev_data,
		struct vsfusbh_uvc_param_t *param,
		struct vsfusbh_uvc_payload_t *payload);
extern const struct vsfusbh_class_drv_t vsfusbh_uvc_drv;
#endif

#endif // __VSFUSBH_UVC_H_INCLUDED__
