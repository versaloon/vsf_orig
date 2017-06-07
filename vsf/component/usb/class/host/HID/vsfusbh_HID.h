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

#ifndef __VSFUSBH_HID_H_INCLUDED__
#define __VSFUSBH_HID_H_INCLUDED__

#define HID_VALUE_TYPE_UNKNOWN			0
#define HID_VALUE_TYPE_SIGN_8BIT		0x01
#define HID_VALUE_TYPE_UNSIGN_8BIT		0x02
#define HID_VALUE_TYPE_SIGN_16BIT		0x03
#define HID_VALUE_TYPE_UNSIGN_16BIT		0x04
#define HID_VALUE_TYPE_ABS				0x0000
#define HID_VALUE_TYPE_REL				0x0100

PACKED_HEAD struct PACKED_MID usbh_hid_event_t
{
	uint16_t usage_page;
	uint16_t usage_id;
	int32_t pre_value;
	int32_t cur_value;
	uint32_t type;
}; PACKED_TAIL

#define HID_LONG_ITEM(x)			((x) == 0xFE)
#define HID_ITEM_SIZE(x)			((((x)&0x03) == 3)?4:(x)&0x03)

#define HID_ITEM_TYPE(x)			(((x) >> 2) & 0x03)
#define HID_ITEM_TYPE_MAIN			0
#define HID_ITEM_TYPE_GLOBAL		1
#define HID_ITEM_TYPE_LOCAL			2

#define HID_ITEM_TAG(x)				((x)&0xFC)
#define HID_ITEM_INPUT				0x80
#define HID_ITEM_OUTPUT				0x90
#define HID_ITEM_FEATURE			0xB0
#define HID_ITEM_COLLECTION			0xA0
#define HID_ITEM_END_COLLECTION		0xC0
#define HID_ITEM_USAGE_PAGE			0x04
#define HID_ITEM_LOGI_MINI			0x14
#define HID_ITEM_LOGI_MAXI			0x24
#define HID_ITEM_PHY_MINI			0x34
#define HID_ITEM_PHY_MAXI			0x44
#define HID_ITEM_UNIT_EXPT			0x54
#define HID_ITEM_UNIT				0x64
#define HID_ITEM_REPORT_SIZE		0x74
#define HID_ITEM_REPORT_ID			0x84
#define HID_ITEM_REPORT_COUNT		0x94
#define HID_ITEM_PUSH				0xA4
#define HID_ITEM_POP				0xB4
#define HID_ITEM_USAGE				0x08
#define HID_ITEM_USAGE_MIN			0x18
#define HID_ITEM_USAGE_MAX			0x28

PACKED_HEAD struct PACKED_MID hid_cleass_descriptor_t
{
	uint8_t bDescriptorType;
	uint16_t wDescriptorLength;
}; PACKED_TAIL

PACKED_HEAD struct PACKED_MID hid_descriptor_t
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;

	struct hid_cleass_descriptor_t desc[1];
}; PACKED_TAIL

PACKED_HEAD struct PACKED_MID hid_desc_t
{
	int collection;
	int report_size;
	int report_count;
	int usage_page;
	int usage_num;
	int logical_min;
	int logical_max;
	int physical_min;
	int physical_max;
	int usage_min;
	int usage_max;
	int usages[16];
}; PACKED_TAIL

struct hid_usage_t
{
	uint16_t usage_page;
	uint8_t usage_min;
	uint8_t usage_max;
	int32_t logical_min;
	int32_t logical_max;
	int32_t bit_offset;
	int32_t bit_length;
	int32_t report_size;
	int32_t report_count;
	uint32_t data_flag;
	struct sllist list;
};

struct hid_report_t
{
	int input_bitlen;
	int output_bitlen;
	uint8_t *cur_value;
	uint8_t *pre_value;
	struct sllist input_list;
	struct sllist output_list;

	uint8_t need_setreport_flag;
	uint8_t need_ignore;

	uint16_t generic_usage;
};

struct vsfusbh_hid_t
{
	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;

	struct vsfusbh_t *usbh;
	struct vsfusbh_device_t *dev;

	struct vsfusbh_urb_t *ctrlurb;
	struct vsfusbh_urb_t *inturb;

	struct usb_interface_t *interface;
	const struct usb_interface_desc_t *intf_desc;
	const struct hid_descriptor_t *hid_desc;
	struct hid_report_t hid_report;
};

struct vsfusbh_hid_global_t
{
	int32_t (*report)(struct usbh_hid_event_t *);
};

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFUSBH_HID_MODNAME					"vsf.stack.usb.host.hid"

struct vsfusbh_hid_modifs_t
{
	struct vsfusbh_class_drv_t drv;
	struct vsfusbh_hid_global_t global;
};

vsf_err_t vsfusbh_hid_modexit(struct vsf_module_t*);
vsf_err_t vsfusbh_hid_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFUSBH_HIDMOD						\
	((struct vsfusbh_hid_modifs_t *)vsf_module_load(VSFUSBH_HID_MODNAME, true))
#define vsfusbh_hid_drv						VSFUSBH_HIDMOD->drv
#define vsfusbh_hid							VSFUSBH_HIDMOD->global

#else
extern const struct vsfusbh_class_drv_t vsfusbh_hid_drv;
extern struct vsfusbh_hid_global_t vsfusbh_hid;
#endif

#endif
