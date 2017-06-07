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

#define HID_LONG_ITEM(x)			((x) == 0xFE)

#ifndef VSFCFG_STANDALONE_MODULE
struct vsfusbh_hid_global_t vsfusbh_hid;
#endif

static vsf_err_t usbh_hid_parse_item(struct hid_desc_t *desc, uint8_t tag,
		int size, uint8_t *buf, struct hid_report_t *hidrpt)
{
	int i;
	int value;
	struct hid_usage_t *usage;

	if (hidrpt->need_ignore == 1)
	{
		return VSFERR_NONE;
	}

	if (size == 1)
		value = buf[0];
	else if (size == 2)
		value = GET_LE_U16C(buf);
	else if (size == 4)
		value = GET_LE_U32C(buf);

	switch (tag)
	{
		case HID_ITEM_INPUT:
			if ((desc->usage_min != -1) && (desc->usage_max != -1))
			{
				desc->usage_num = desc->usage_max - desc->usage_min + 1;
				usage = vsf_bufmgr_malloc(sizeof(struct hid_usage_t));
				if (usage == NULL)
					return VSFERR_FAIL;

				usage->data_flag = (uint32_t)value;
				usage->report_size = (int32_t)desc->report_size;
				usage->report_count = (int32_t)desc->report_count;

				usage->usage_page = (uint16_t)desc->usage_page;
				usage->usage_min = (uint8_t)desc->usage_min;
				usage->usage_max = (uint8_t)desc->usage_max;
				usage->bit_offset = (int32_t)hidrpt->input_bitlen;
				usage->bit_length = (int32_t)(desc->report_size * desc->report_count);

				usage->logical_min = desc->logical_min;
				usage->logical_max = desc->logical_max;

				sllist_append(&hidrpt->input_list, &usage->list);

				desc->usage_min = -1;
				desc->usage_max = -1;
			}
			else
			{
				for (i = 0; i < desc->usage_num; i++)
				{
					usage = vsf_bufmgr_malloc(sizeof(struct hid_usage_t));
					if (usage == NULL)
						return VSFERR_FAIL;

					usage->report_size = (int32_t)desc->report_size;
					usage->report_count = (int32_t)desc->report_count / desc->usage_num;
					usage->data_flag = (uint32_t)value;

					usage->usage_page = (uint16_t)desc->usage_page;
					usage->usage_min = (uint8_t)desc->usages[i];
					usage->usage_max = (uint8_t)desc->usages[i];
					usage->bit_length = (int32_t)(desc->report_size * desc->report_count / desc->usage_num);
					usage->bit_offset = (int32_t)(hidrpt->input_bitlen + i * usage->bit_length);

					usage->logical_min = desc->logical_min;
					usage->logical_max = desc->logical_max;

					sllist_append(&hidrpt->input_list, &usage->list);
				}
			}

			desc->usage_num = 0;
			hidrpt->input_bitlen += (desc->report_size * desc->report_count);
			break;

		case HID_ITEM_OUTPUT:
			if ((desc->usage_min != -1) && (desc->usage_max != -1))
			{
				desc->usage_num = desc->usage_max - desc->usage_min + 1;
				usage = vsf_bufmgr_malloc(sizeof(struct hid_usage_t));
				if (usage == NULL)
					return VSFERR_FAIL;

				usage->report_size = desc->report_size;
				usage->report_count = desc->report_count;
				usage->data_flag = value;

				usage->usage_page = desc->usage_page;
				usage->usage_min = desc->usage_min;
				usage->usage_max = desc->usage_max;
				usage->bit_offset = hidrpt->output_bitlen;
				usage->bit_length = desc->report_size * desc->report_count;

				usage->logical_min = desc->logical_min;
				usage->logical_max = desc->logical_max;

				sllist_append(&hidrpt->output_list, &usage->list);

				desc->usage_min = -1;
				desc->usage_max = -1;
			}
			else
			{
				for (i = 0; i < desc->usage_num; i++)
				{
					usage = vsf_bufmgr_malloc(sizeof(struct hid_usage_t));
					if (usage == NULL)
						return VSFERR_FAIL;

					usage->report_size = desc->report_size;
					usage->report_count = desc->report_count;
					usage->data_flag = value;
					value = desc->report_size * desc->report_count / desc->usage_num;

					usage->usage_page = desc->usage_page;
					usage->usage_min = desc->usages[i];
					usage->usage_max = desc->usages[i];
					usage->bit_offset = hidrpt->output_bitlen + i * value;
					usage->bit_length = value;

					usage->logical_min = desc->logical_min;
					usage->logical_max = desc->logical_max;

					sllist_append(&hidrpt->output_list, &usage->list);
				}
			}

			desc->usage_num = 0;
			hidrpt->output_bitlen += (desc->report_size * desc->report_count);
			break;

		case HID_ITEM_FEATURE:
			break;

		case HID_ITEM_COLLECTION:
			desc->collection++;
			desc->usage_num = 0;
			break;

		case HID_ITEM_END_COLLECTION:
			desc->collection--;
			break;

		case HID_ITEM_USAGE_PAGE:
			desc->usage_page = value;
			break;

		case HID_ITEM_LOGI_MINI:
			if (size == 1)
				value = buf[0];
			else if (size == 2)
				value = GET_LE_U16C(buf);
			else if (size == 4)
				value = GET_LE_U32C(buf);
			desc->logical_min = value;
			break;

		case HID_ITEM_LOGI_MAXI:
			if (size == 1)
				value = buf[0];
			else if (size == 2)
				value = GET_LE_U16C(buf);
			else if (size == 4)
				value = GET_LE_U32C(buf);
			desc->logical_max = value;
			break;

		case HID_ITEM_PHY_MINI:
			break;

		case HID_ITEM_PHY_MAXI:
			break;

		case HID_ITEM_UNIT_EXPT:
			break;

		case HID_ITEM_UNIT:
			break;

		case HID_ITEM_REPORT_SIZE:
			desc->report_size = value;
			break;

		case HID_ITEM_REPORT_ID:
			if (hidrpt->need_setreport_flag == 0)
			{
				hidrpt->need_setreport_flag = 1;
				hidrpt->input_bitlen += 8;
			}
			else
			{
				desc->collection = 0;
				hidrpt->need_ignore = 1;
			}
			break;

		case HID_ITEM_REPORT_COUNT:
			desc->report_count = value;
			break;

		case HID_ITEM_PUSH:
			break;

		case HID_ITEM_POP:
			break;

		case HID_ITEM_USAGE:
			desc->usages[desc->usage_num++] = value;
			break;

		case HID_ITEM_USAGE_MAX:
			desc->usage_max = value;
			break;

		case HID_ITEM_USAGE_MIN:
			desc->usage_min = value;
			break;
	}

	return VSFERR_NONE;
}

static void usbh_hid_free_report(struct hid_report_t *rpt)
{
	struct sllist *list;
	struct hid_usage_t *usage;

	/* free input list */
	list = rpt->input_list.next;
	while (list)
	{
		usage = sllist_get_container(list, struct hid_usage_t, list);
		list = list->next;

		vsf_bufmgr_free(usage);
	}

	/* free output list */
	list = rpt->output_list.next;
	while (list)
	{
		usage = sllist_get_container(list, struct hid_usage_t, list);
		list = list->next;

		vsf_bufmgr_free(usage);
	}

	/* free buffer */
	if (rpt->cur_value)
		vsf_bufmgr_free(rpt->cur_value);
	if (rpt->pre_value)
		vsf_bufmgr_free(rpt->pre_value);

	rpt->cur_value = NULL;
	rpt->pre_value = NULL;
}


static vsf_err_t hid_parse_report(struct vsfusbh_hid_t *hid, uint8_t *buf,
		uint32_t len)
{
	struct hid_report_t *hidrpt = &hid->hid_report;
	uint8_t b;
	uint8_t *end = buf + len;
	int item_size;
	vsf_err_t err;
	struct hid_desc_t *desc = vsf_bufmgr_malloc(sizeof(struct hid_desc_t));

	if (desc == NULL)
		return VSFERR_FAIL;

	memset(hidrpt, 0, sizeof(struct hid_report_t));
	memset(desc, 0, sizeof(struct hid_desc_t));

	sllist_init_node(hidrpt->input_list);
	sllist_init_node(hidrpt->output_list);

	desc->usage_min = -1;
	desc->usage_max = -1;

	while (buf < end)
	{
		b = *buf;

		if (HID_LONG_ITEM(b))
		{
			item_size = *(buf + 1);
		}
		else
		{
			item_size = HID_ITEM_SIZE(b);
			err = usbh_hid_parse_item(desc, HID_ITEM_TAG(b), item_size, buf + 1, hidrpt);
			if (err)
			{
				err = -1;
				break;
			}
		}

		if ((hidrpt->generic_usage == 0) && (desc->collection == 0) &&
				(desc->usages[0] != 0))
		{
			// get report type such as mouse, keyboard, or game pad
			hidrpt->generic_usage = desc->usages[0];
		}


		buf += (item_size + 1);
	}

	/* error happened */
	if ((desc->collection != 0) || err)
		goto free_hid_report;

	/* alloc memory for input event data */
	hidrpt->cur_value = vsf_bufmgr_malloc(max(hidrpt->input_bitlen >> 3, 32));
	if (hidrpt->cur_value == NULL)
		goto free_hid_report;

	hidrpt->pre_value = vsf_bufmgr_malloc(max(hidrpt->input_bitlen >> 3, 32));
	if (hidrpt->pre_value == NULL)
		goto free_cur_value;

	memset(hidrpt->pre_value, 0, hidrpt->input_bitlen >> 3);
	memset(hidrpt->cur_value, 0, hidrpt->input_bitlen >> 3);

	vsf_bufmgr_free(desc);
	return VSFERR_NONE;

	/* error process */
free_cur_value:
	vsf_bufmgr_free(hidrpt->cur_value);
free_hid_report:
	vsf_bufmgr_free(desc);
	usbh_hid_free_report(hidrpt);

	return VSFERR_FAIL;
}

static uint32_t get_bits(uint8_t *buf, uint32_t bit_offset, uint32_t length)
{
	uint16_t temp = 0, temp2;

	if (NULL == buf)
	{
		return 0;
	}
	temp = (bit_offset & 0x07) + length;
	if (temp <= 8)
	{
		return (buf[bit_offset >> 3] & (((0x01ul << length) - 1) << (bit_offset & 0x07))) >> (bit_offset & 0x07);
	}
	else if (temp <= 16)
	{
		temp2 = (buf[bit_offset >> 3] & (0xff << (bit_offset & 0x07))) >> (bit_offset & 0x07);
		return temp2 + ((buf[(bit_offset >> 3) + 1] & (0xff >> (16 - temp))) << (8 - (bit_offset & 0x07)));
	}
	else
	{
		// not support
		return 0;
	}
}

static void usbh_hid_process_input(struct hid_report_t *report_x)
{
	struct hid_report_t *report = report_x;
	struct hid_usage_t *usage;
	uint32_t cur_value, pre_value;
	int32_t i;

	/* have a quick check */
	if (0 == memcmp(report->cur_value, report->pre_value, report->input_bitlen >> 3))
		return;

	usage = sllist_get_container(report->input_list.next, struct hid_usage_t, list);
	while (usage != NULL)
	{
		for (i=0; i<usage->report_count; ++i)
		{
			/* get usage value */
			cur_value = get_bits(report->cur_value, usage->bit_offset + i * usage->report_size, usage->report_size);
			pre_value = get_bits(report->pre_value, usage->bit_offset + i * usage->report_size, usage->report_size);

			/* compare and process */
			if (cur_value != pre_value)
			{
				struct usbh_hid_event_t event;
				event.type = HID_VALUE_TYPE_UNKNOWN;

				event.usage_page = usage->usage_page;
				event.usage_id = (usage->data_flag & 0x2) ? (usage->usage_min + i) :
						(cur_value ? (uint16_t)cur_value : (uint16_t)pre_value);

				if (event.usage_page != 0x01)
				{
					event.pre_value = pre_value;
					event.cur_value = cur_value;
				}
				else
				{
					int32_t middle;
					event.type = HID_VALUE_TYPE_SIGN_16BIT;
					middle = (usage->logical_min + usage->logical_max) / 2;
					if (usage->logical_min >= 0)
					{
						event.pre_value = -(middle - pre_value);
						event.cur_value = -(middle - cur_value);
					}
					else
					{
						event.pre_value = pre_value & (~(0x1ul << (usage->report_size - 1)));
						if (pre_value & (0x1ul << (usage->report_size - 1)))
						{
							event.pre_value -= 0x1l << (usage->report_size - 1);
						}
						event.cur_value = cur_value & (~(0x1ul << (usage->report_size - 1)));
						if (cur_value & (0x1ul << (usage->report_size - 1)))
						{
							event.cur_value -= 0x1l << (usage->report_size - 1);
						}
					}
				}

				if (usage->data_flag & 0x4)
				{
					event.type |= HID_VALUE_TYPE_REL;
				}

				if (vsfusbh_hid.report)
					vsfusbh_hid.report(&event);
			}
		}
		usage = sllist_get_container(usage->list.next, struct hid_usage_t, list);
	}
}

static vsf_err_t hid_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hid_t *hid = (struct vsfusbh_hid_t *)pt->user_data;
	struct vsfusbh_urb_t *ctrlurb = hid->ctrlurb;
	struct vsfusbh_urb_t *inturb = hid->inturb;

	vsfsm_pt_begin(pt);

	ctrlurb->sm = &hid->sm;
	inturb->sm = &hid->sm;

	ctrlurb->transfer_length = hid->hid_desc->desc[0].wDescriptorLength;
	if (ctrlurb->transfer_length > 1024)
		return VSFERR_NOT_SUPPORT;
	ctrlurb->transfer_buffer = vsf_bufmgr_malloc(ctrlurb->transfer_length);
	if (ctrlurb->transfer_buffer == NULL)
		return VSFERR_FAIL;

	if (vsfsm_crit_enter(&hid->dev->ep0_crit, &hid->sm))
		vsfsm_pt_wfe(pt, VSFSM_EVT_EP0_CRIT);

	err = vsfusbh_get_class_descriptor(hid->usbh, ctrlurb,
			hid->intf_desc->bInterfaceNumber, USB_DT_REPORT, 0);
	if (err != VSFERR_NONE)
		goto ctrlurb_fail;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (ctrlurb->status != URB_OK)
		goto ctrlurb_fail;
	if (ctrlurb->actual_length != ctrlurb->transfer_length)
		goto ctrlurb_fail;

	err = hid_parse_report(hid, ctrlurb->transfer_buffer,
			ctrlurb->transfer_length);
	if (err != VSFERR_NONE)
		goto ctrlurb_fail;

	vsfsm_crit_leave(&hid->dev->ep0_crit);
	vsf_bufmgr_free(ctrlurb->transfer_buffer);
	ctrlurb->transfer_buffer = NULL;

	// unknown set report
	if (hid->hid_report.need_setreport_flag)
	{
		// TODO
	}

	// submit urb
	inturb->pipe = usb_rcvintpipe(inturb->vsfdev, hid->intf_desc->ep_desc->bEndpointAddress & 0x7f);
	inturb->interval = 4;
	inturb->transfer_length = max(hid->hid_report.input_bitlen >> 3, hid->intf_desc->ep_desc->wMaxPacketSize);
	inturb->transfer_buffer = hid->hid_report.cur_value;
	if (inturb->transfer_buffer == NULL)
		return VSFERR_FAIL;

	err = vsfusbh_submit_urb(hid->usbh, inturb);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (inturb->status != URB_OK)
		return VSFERR_FAIL;

	// poll
	while(1)
	{
		if (inturb->status == URB_OK)
		{
			usbh_hid_process_input(&hid->hid_report);
			memcpy(hid->hid_report.pre_value, hid->hid_report.cur_value,
					hid->hid_report.input_bitlen >> 3);
		}

		err = vsfusbh_relink_urb(hid->usbh, inturb);
		if (err != VSFERR_NONE)
			return err;

		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);

	}

	vsfsm_pt_end(pt);

	return VSFERR_NONE;

ctrlurb_fail:
	vsfsm_crit_leave(&hid->dev->ep0_crit);
	vsf_bufmgr_free(ctrlurb->transfer_buffer);
	ctrlurb->transfer_buffer = NULL;
	return VSFERR_FAIL;
}

static struct vsfsm_state_t *vsfusbh_hid_evt_handler_init(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hid_t *hid = (struct vsfusbh_hid_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		hid->pt.thread = hid_thread;
		hid->pt.user_data = hid;
		hid->pt.sm = sm;
		hid->pt.state = 0;
	case VSFSM_EVT_URB_COMPLETE:
	case VSFSM_EVT_EP0_CRIT:
	case VSFSM_EVT_DELAY_DONE:
		err = hid->pt.thread(&hid->pt, evt);
		if (err < 0)
		{
			vsfusbh_remove_interface(hid->usbh, hid->dev, hid->interface);
		}
		break;
	default:
		break;
	}
	return NULL;
}

static void *vsfusbh_hid_probe(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev,
			struct usb_interface_t *interface,
			const struct vsfusbh_device_id_t *id)
{
	struct usb_interface_desc_t *intf_desc = interface->altsetting +
			interface->act_altsetting;
	struct vsfusbh_hid_t *hid;
	struct hid_descriptor_t *hid_desc;

	if ((vsfusbh_get_extra_descriptor((uint8_t *)intf_desc->extra,
			intf_desc->extralen, USB_DT_HID, (void **)&hid_desc) !=
			VSFERR_NONE) &&
		((!intf_desc->bNumEndpoints) ||
		 	vsfusbh_get_extra_descriptor((uint8_t *)intf_desc->ep_desc[0].extra,
					intf_desc->ep_desc[0].extralen, USB_DT_HID,
					(void **)&hid_desc) !=
					VSFERR_NONE))
	{
		return NULL;
	}

	hid = vsf_bufmgr_malloc(sizeof(struct vsfusbh_hid_t));
	if (hid == NULL)
		return NULL;
	memset(hid, 0, sizeof(struct vsfusbh_hid_t));

	hid->ctrlurb = usbh->hcd->alloc_urb();
	if (hid->ctrlurb == NULL)
	{
		vsf_bufmgr_free(hid);
		return NULL;
	}
	hid->inturb = usbh->hcd->alloc_urb();
	if (hid->inturb == NULL)
	{
		usbh->hcd->free_urb(usbh->hcd_data, &hid->ctrlurb);
		vsf_bufmgr_free(hid);
		return NULL;
	}

	hid->usbh = usbh;
	hid->dev = dev;
	hid->interface = interface;
	hid->intf_desc = intf_desc;
	hid->hid_desc = hid_desc;
	hid->ctrlurb->vsfdev = dev;
	hid->ctrlurb->timeout = 200;
	hid->inturb->vsfdev = dev;
	hid->inturb->timeout = 200;
	hid->sm.init_state.evt_handler = vsfusbh_hid_evt_handler_init;
	hid->sm.user_data = hid;
	vsfsm_init(&hid->sm);

	return hid;
}

static void vsfusbh_hid_disconnect(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, void *priv)
{
	struct vsfusbh_hid_t *hid = priv;
	if (hid == NULL)
		return;

	if (hid->ctrlurb != NULL)
		usbh->hcd->free_urb(usbh->hcd_data, &hid->ctrlurb);
	if (hid->inturb != NULL)
		usbh->hcd->free_urb(usbh->hcd_data, &hid->inturb);

	usbh_hid_free_report(&hid->hid_report);
	vsfsm_fini(&hid->sm);
	vsf_bufmgr_free(hid);
}

static const struct vsfusbh_device_id_t vsfusbh_hid_id_table[] =
{
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_HID,
	},
	{0},
};

#ifdef VSFCFG_STANDALONE_MODULE
vsf_err_t vsfusbh_hid_modexit(struct vsf_module_t *module)
{
	vsf_bufmgr_free(module->ifs);
	module->ifs = NULL;
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_hid_modinit(struct vsf_module_t *module,
								struct app_hwcfg_t const *cfg)
{
	struct vsfusbh_hid_modifs_t *ifs;
	ifs = vsf_bufmgr_malloc(sizeof(struct vsfusbh_hid_modifs_t));
	if (!ifs) return VSFERR_FAIL;
	memset(ifs, 0, sizeof(*ifs));

	ifs->drv.name = "hid";
	ifs->drv.id_table = vsfusbh_hid_id_table;
	ifs->drv.probe = vsfusbh_hid_probe;
	ifs->drv.disconnect = vsfusbh_hid_disconnect;
	module->ifs = ifs;
	return VSFERR_NONE;
}
#else
const struct vsfusbh_class_drv_t vsfusbh_hid_drv =
{
	.name = "hid",
	.id_table = vsfusbh_hid_id_table,
	.probe = vsfusbh_hid_probe,
	.disconnect = vsfusbh_hid_disconnect,
	.ioctl = NULL,
};
#endif
