#include "vsf.h"
#include "vsfdwcotg_priv.h"

static struct hc_t *alloc_hc(struct dwcotg_t *dwcotg)
{
	uint8_t i;
	for (i = 0; i < dwcotg->hc_amount; i++)
	{
		if (dwcotg->hc_pool[i].alloced == 0)
		{
			memset(&dwcotg->hc_pool[i], 0, sizeof(struct hc_t));
			dwcotg->hc_pool[i].hc_num = i;
			dwcotg->hc_pool[i].alloced = 1;
			return &dwcotg->hc_pool[i];
		}
	}
	return NULL;
}

static void free_hc(struct hc_t *hc)
{
	// TODO reset hc reg

	hc->alloced = 0;
}

static struct hc_t *hc_init(struct dwcotg_t *dwcotg,
		struct dwcotg_device_t *dev_priv)
{
	struct hc_t *hc;

	if (dev_priv->hc_num < MAX_HC_NUM_EACH_DEVICE)
	{
		hc = alloc_hc(dwcotg);
		if (hc == NULL)
			return NULL;
		else
		{
			uint8_t i;
			for (i = 0; i < MAX_HC_NUM_EACH_DEVICE; i++)
			{
				if (dev_priv->hc[i] == NULL)
				{
					dev_priv->hc[i] = hc;
					dev_priv->hc_num++;
					hc->owner_dev = dev_priv;
					return hc;
				}
			}
		}
		free_hc(hc);
	}
	return NULL;
}

static void hc_fini(struct hc_t **hcp)
{
	uint8_t i;
	struct hc_t *hc;
	struct dwcotg_device_t *dev_priv;

	if (*hcp == NULL)
		return;

	hc = *hcp;
	hcp = NULL;
	dev_priv = hc->owner_dev;

	for (i = 0; i < MAX_HC_NUM_EACH_DEVICE; i++)
	{
		if (dev_priv->hc[i] == hc)
		{
			dev_priv->hc[i] = NULL;
			dev_priv->hc_num--;
			free_hc(hc);
			break;
		}
	}
}

static void hc_halt(struct dwcotg_t *dwcotg, uint8_t hc_num)
{
	uint32_t count = 0;
	struct dwcotg_hc_regs_t *hc_reg = &dwcotg->hc_regs[hc_num];
	uint8_t type = (hc_reg->hcchar >> 18) & 0x3;

	hc_reg->hcchar |= USB_OTG_HCCHAR_CHDIS;

	if ((type == 0) || (type == 2))
	{
		if (dwcotg->global_reg->gnptxsts & 0xffff)
		{
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_EPDIR;
			do
			{
				if (count++ > 1000)
					break;
			}
			while ((hc_reg->hcchar & USB_OTG_HCCHAR_CHENA) == USB_OTG_HCCHAR_CHENA);
		}
		else
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
	}
	else
	{
		if (dwcotg->host_global_regs->hptxsts & 0xffff)
		{
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_EPDIR;
			do
			{
				if (count++ > 1000)
					break;
			}
			while ((hc_reg->hcchar & USB_OTG_HCCHAR_CHENA) == USB_OTG_HCCHAR_CHENA);
		}
		else
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
	}
}

const uint8_t pipetype_to_dwctype[4] = {1, 3, 0, 2};
static vsf_err_t submit_hc(struct dwcotg_t *dwcotg, struct hc_t *hc,
		uint32_t *buf, uint16_t size)
{
	uint32_t pkt_num, tmp;
	struct urb_priv_t *urb_priv = hc->owner_priv;
	struct dwcotg_hc_regs_t *reg = &dwcotg->hc_regs[hc->hc_num];

	switch (urb_priv->type)
	{
	case URB_PRIV_TYPE_ISO:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_ACKM |
				USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_FRMORM ;
		if (hc->dir_o0_i1)
			reg->hcintmsk |= USB_OTG_HCINTMSK_TXERRM;
		break;
	case URB_PRIV_TYPE_INT:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_STALLM |
				USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM|
				USB_OTG_HCINTMSK_NAKM | USB_OTG_HCINTMSK_AHBERR |
				USB_OTG_HCINTMSK_FRMORM;
		break;
	case URB_PRIV_TYPE_CTRL:
	case URB_PRIV_TYPE_BULK:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM  | USB_OTG_HCINTMSK_STALLM |
				USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM |
				USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_NAKM;
		if (!hc->dir_o0_i1)
			reg->hcintmsk |= USB_OTG_HCINTMSK_NYET;
		break;
	}
	if (hc->dir_o0_i1)
		reg->hcintmsk |= USB_OTG_HCINTMSK_BBERRM;

	dwcotg->host_global_regs->haintmsk |= 0x1ul << hc->hc_num;

	reg->hcchar = (hc->dev_addr << 22) | (hc->ep_num << 11) |
			(hc->dir_o0_i1 ? USB_OTG_HCCHAR_EPDIR : 0) |
			((hc->speed == USB_SPEED_LOW) << 17) |
			((uint32_t)pipetype_to_dwctype[urb_priv->type] << 18) |
			(urb_priv->packet_size & USB_OTG_HCCHAR_MPSIZ);

	if (urb_priv->type == URB_PRIV_TYPE_INT)
	{
		reg->hcchar |= USB_OTG_HCCHAR_ODDFRM;
	}

	// transfer size
	if (size > 0)
	{
		pkt_num = (size + urb_priv->packet_size - 1) / urb_priv->packet_size;
		if (pkt_num > 256)
		{
			pkt_num = 256;
			size = urb_priv->packet_size * 256;
		}
	}
	else
		pkt_num = 1;
	urb_priv->toggle_next = urb_priv->toggle_start ^ (pkt_num & 0x1);

	if (hc->dir_o0_i1)
		size = urb_priv->packet_size * pkt_num;
	hc->transfer_size = size;

	reg->hctsiz = ((pkt_num << 19) & USB_OTG_HCTSIZ_PKTCNT) |
			((uint32_t)hc->dpid << 29) | size;

	if (dwcotg->dma_en)
		reg->hcdma = (uint32_t)buf;

	tmp = dwcotg->host_global_regs->hfnum & 0x1 ? 0x0ul : 0x1ul;
	reg->hcchar |= tmp << 29;

	// enable hc, TODO merge up code
	tmp = (reg->hcchar & (~USB_OTG_HCCHAR_CHDIS)) | USB_OTG_HCCHAR_CHENA;
	reg->hcchar = tmp;

	if ((dwcotg->dma_en == 0) && (hc->dir_o0_i1 == 0) && (hc->transfer_size))
	{
		// TODO: check if there is enough space in FIFO space
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_CTRL:
		case URB_PRIV_TYPE_BULK:
			break;
		case URB_PRIV_TYPE_ISO:
		case URB_PRIV_TYPE_INT:
			break;
		}
		//size = (size + 3) >> 2;
		tmp = 0;
		while (tmp < size)
		{
			dwcotg->dfifo[hc->hc_num] = buf[tmp >> 2];
			tmp += 4;
		}
	}

	return VSFERR_NONE;
}

static vsf_err_t submit_priv_urb(struct dwcotg_t *dwcotg,
		struct urb_priv_t *urb_priv)
{
	uint32_t interval;
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);
	struct hc_t *hc = urb_priv->hc;

	switch (urb_priv->phase)
	{
	case URB_PRIV_PHASE_PERIOD_WAIT:
		if (dwcotg->speed == USB_SPEED_HIGH)
			interval = vsfurb->interval * 8;
		else
			interval = vsfurb->interval;
		if (dwcotg->softick % interval == 0)
		{
			hc->hc_state = HC_START;
			urb_priv->phase = URB_PRIV_PHASE_DATA_WAIT;
			return submit_priv_urb(dwcotg, urb_priv);
		}
		else
		{
			hc->hc_state = HC_WAIT;
		}
		return VSFERR_NONE;
	case URB_PRIV_PHASE_SETUP_WAIT:
		hc->dir_o0_i1 = 0;
		hc->dpid = HC_DPID_SETUP;

		return submit_hc(dwcotg, hc, (uint32_t *)(&vsfurb->setup_packet),
				sizeof(struct usb_ctrlrequest_t));
	case URB_PRIV_PHASE_DATA_WAIT:
		hc->dir_o0_i1 = urb_priv->dir_o0_i1;
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_ISO:
			hc->dpid = HC_DPID_DATA0;
			break;
		case URB_PRIV_TYPE_INT:
		case URB_PRIV_TYPE_BULK:
			hc->dpid = urb_priv->toggle_start ? HC_DPID_DATA1 : HC_DPID_DATA0;
			break;
		case URB_PRIV_TYPE_CTRL:
			hc->dpid = HC_DPID_DATA1;
			break;
		}
		return submit_hc(dwcotg, hc, (uint32_t *)urb_priv->transfer_buffer,
				urb_priv->transfer_length);
	case URB_PRIV_PHASE_STATE_WAIT:
		hc->dir_o0_i1 = !urb_priv->dir_o0_i1;
		hc->dpid = HC_DPID_DATA1;

		return submit_hc(dwcotg, hc, NULL, 0);
	default :
		return VSFERR_FAIL;
	}
}

static void free_priv_urb(struct urb_priv_t *urb_priv)
{
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);

	hc_fini(&urb_priv->hc);
	if (vsfurb->transfer_buffer != NULL)
		vsf_bufmgr_free(vsfurb->transfer_buffer);
	vsf_bufmgr_free(vsfurb);
}

static void vsfdwcotg_hc_done_handler(struct dwcotg_t *dwcotg,
		struct urb_priv_t *urb_priv, struct hc_t *hc)
{
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);
	struct dwcotg_device_t *dev_priv = hc->owner_dev;
	struct vsfusbh_device_t *vsfdev = dev_priv->vsfdev;

	if ((vsfdev == NULL) || (urb_priv->discarded))
	{
		free_priv_urb(urb_priv);
		if (vsfdev == NULL)
		{
			if (dev_priv->hc_num == 0)
				vsf_bufmgr_free(dev_priv);
		}
		return;
	}

	switch (urb_priv->state)
	{
	case URB_PRIV_STATE_NAK:
		hc->hc_state = HC_WAIT;
		urb_priv->phase = URB_PRIV_PHASE_PERIOD_WAIT;
		submit_priv_urb(dwcotg, urb_priv);
		break;
	case URB_PRIV_STATE_NOTREADY:
		submit_priv_urb(dwcotg, urb_priv);
		break;
	case URB_PRIV_STATE_DONE:
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_INT:
			urb_priv->toggle_start = urb_priv->toggle_next;
			if (urb_priv->toggle_next)
			{
				vsfdev->toggle[hc->dir_o0_i1] |= 0x1ul << hc->ep_num;
			}
			else
			{
				vsfdev->toggle[hc->dir_o0_i1] &= ~(0x1ul << hc->ep_num);
			}
		case URB_PRIV_TYPE_ISO:
			vsfurb->actual_length = urb_priv->actual_length;
			vsfurb->status = URB_OK;
			vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
			break;
		case URB_PRIV_TYPE_BULK:
			if (urb_priv->toggle_next)
			{
				vsfdev->toggle[hc->dir_o0_i1] |= 0x1ul << hc->ep_num;
			}
			else
			{
				vsfdev->toggle[hc->dir_o0_i1] &= ~(0x1ul << hc->ep_num);
			}
			vsfurb->actual_length = urb_priv->actual_length;
			hc_fini(&urb_priv->hc);
			vsfurb->status = URB_OK;
			vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
			break;
		case URB_PRIV_TYPE_CTRL:
			if ((urb_priv->phase == URB_PRIV_PHASE_SETUP_WAIT) &&
					(urb_priv->transfer_length == 0))
			{
				urb_priv->phase = URB_PRIV_PHASE_STATE_WAIT;
			}
			else if (urb_priv->phase == URB_PRIV_PHASE_DATA_WAIT)
			{
				vsfurb->actual_length = urb_priv->actual_length;
				urb_priv->phase++;
			}
			else
				urb_priv->phase++;
			if (urb_priv->phase == URB_PRIV_PHASE_DONE)
			{
				hc_fini(&urb_priv->hc);
				vsfurb->status = URB_OK;
				vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
			}
			else
			{
				submit_priv_urb(dwcotg, urb_priv);
			}
			break;
		}
		break;
	default:
		hc_fini(&urb_priv->hc);
		vsfurb->status = VSFERR_FAIL;
		vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
		break;
	}
}

static void vsfdwcotg_hc_in_handler(struct dwcotg_t *dwcotg, uint8_t hc_num)
{
	struct dwcotg_hc_regs_t *hc_reg = &dwcotg->hc_regs[hc_num];
	struct hc_t *hc = &dwcotg->hc_pool[hc_num];
	struct urb_priv_t *urb_priv = hc->owner_priv;
	uint32_t intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_AHBERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_AHBERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
	}
	else if (intsts & USB_OTG_HCINT_STALL)
	{
		hc->hc_state = HC_STALL;
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_STALL;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_DTERR)
	{
		hc->hc_state = HC_DATATGLERR;
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_DTERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_FRMOR)
	{
		hc_reg->hcint = USB_OTG_HCINT_FRMOR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_TXERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_TXERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc->hc_state = HC_XACTERR;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_ACK)
	{
		hc_reg->hcint = USB_OTG_HCINT_ACK;
	}

	if (intsts & USB_OTG_HCINT_XFRC)
	{
		if (dwcotg->dma_en)
		{
			urb_priv->actual_length = hc->transfer_size -
					(hc_reg->hctsiz & USB_OTG_HCTSIZ_XFRSIZ);
		}

		hc->hc_state = HC_XFRC;
		hc_reg->hcint = USB_OTG_HCINT_XFRC;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;

		if ((urb_priv->type == URB_PRIV_TYPE_CTRL) ||
			(urb_priv->type == URB_PRIV_TYPE_BULK))
		{
			hc_halt(dwcotg, hc_num);
			hc_reg->hcint = USB_OTG_HCINT_NAK;
		}
		else if (urb_priv->type == URB_PRIV_TYPE_INT)
		{
			hc_reg->hcchar |= USB_OTG_HCCHAR_ODDFRM;
			urb_priv->state = URB_PRIV_STATE_DONE;
		}
	}
	else if (intsts & USB_OTG_HCINT_NAK)
	{
		hc->hc_state = HC_NAK;
		hc_reg->hcint = USB_OTG_HCINT_NAK;

		if (urb_priv->type == URB_PRIV_TYPE_INT)
		{
			hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
			hc_halt(dwcotg, hc_num);
		}
		else if ((urb_priv->type == URB_PRIV_TYPE_CTRL) ||
				(urb_priv->type == URB_PRIV_TYPE_BULK))
		{
			// re-activate the channel
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_CHDIS;
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
		}
	}

	intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_CHH)
	{
		hc_reg->hcintmsk &= ~USB_OTG_HCINTMSK_CHHM;
		hc_reg->hcint = USB_OTG_HCINT_CHH;

		if (hc->hc_state == HC_XFRC)
			urb_priv->state = URB_PRIV_STATE_DONE;
		else if (hc->hc_state == HC_STALL)
			urb_priv->state = URB_PRIV_STATE_STALL;
		else if ((hc->hc_state == HC_XACTERR) ||
				(hc->hc_state == HC_DATATGLERR))
		{
			if (++hc->err_cnt > 8)
			{
				hc->err_cnt = 0;
				urb_priv->state = URB_PRIV_STATE_ERROR;
			}
			else
			{
				urb_priv->state = URB_PRIV_STATE_NOTREADY;
			}
		}
		else if (hc->hc_state == HC_NAK)
		{
			urb_priv->state = URB_PRIV_STATE_NAK;
		}
		else
		{
			urb_priv->state = URB_PRIV_STATE_ERROR;
		}

		vsfdwcotg_hc_done_handler(dwcotg, urb_priv, hc);
	}
}
static void vsfdwcotg_hc_out_handler(struct dwcotg_t *dwcotg, uint8_t hc_num)
{
	struct dwcotg_hc_regs_t *hc_reg = &dwcotg->hc_regs[hc_num];
	struct hc_t *hc = &dwcotg->hc_pool[hc_num];
	struct urb_priv_t *urb_priv = hc->owner_priv;
	uint32_t intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_AHBERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_AHBERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
	}
	else if (intsts & USB_OTG_HCINT_NYET)
	{
		hc->hc_state = HC_NYET;
		hc->err_cnt = 0;
		hc_reg->hcint = USB_OTG_HCINT_NYET;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_FRMOR)
	{
		hc_reg->hcint = USB_OTG_HCINT_FRMOR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_STALL)
	{
		hc->hc_state = HC_STALL;
		hc_reg->hcint = USB_OTG_HCINT_STALL;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_TXERR)
	{
		hc->hc_state = HC_XACTERR;
		hc_reg->hcint = USB_OTG_HCINT_TXERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_DTERR)
	{
		hc->hc_state = HC_DATATGLERR;
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_DTERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_ACK)
	{
		hc_reg->hcint = USB_OTG_HCINT_ACK;

		if (urb_priv->do_ping)
		{
			hc->hc_state = HC_NYET;
			hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
			hc_halt(dwcotg, hc_num);
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
		}
	}

	if (intsts & USB_OTG_HCINT_XFRC)
	{
		if (dwcotg->dma_en)
		{
			urb_priv->actual_length = hc->transfer_size;
		}

		hc->hc_state = HC_XFRC;
		hc_reg->hcint = USB_OTG_HCINT_XFRC;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_NAK)
	{
		hc->hc_state = HC_NAK;
		hc_reg->hcint = USB_OTG_HCINT_NAK;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
	}

	intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_CHH)
	{
		hc_reg->hcintmsk &= ~USB_OTG_HCINTMSK_CHHM;
		hc_reg->hcint = USB_OTG_HCINT_CHH;

		if (hc->hc_state == HC_XFRC)
		{
			urb_priv->state = URB_PRIV_STATE_DONE;
			if (urb_priv->type == URB_PRIV_TYPE_BULK)
			{
				// TODO
				//hc->toggle ^= 1;
			}

		}
		else if (hc->hc_state == HC_NAK)
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
		else if (hc->hc_state == HC_NYET)
		{
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
			urb_priv->do_ping = 0;
		}
		else if (hc->hc_state == HC_STALL)
			urb_priv->state = URB_PRIV_STATE_STALL;
		else if ((hc->hc_state == HC_XACTERR) ||
				(hc->hc_state == HC_DATATGLERR))
		{
			if (++hc->err_cnt > 8)
			{
				hc->err_cnt = 0;
				urb_priv->state = URB_PRIV_STATE_ERROR;
			}
			else
			{
				urb_priv->state = URB_PRIV_STATE_NOTREADY;
			}
		}
		else if (hc->hc_state == HC_NAK)
		{
			urb_priv->state = URB_PRIV_STATE_NAK;
		}
		else
		{
			urb_priv->state = URB_PRIV_STATE_ERROR;
		}

		vsfdwcotg_hc_done_handler(dwcotg, urb_priv, hc);
	}
}
static vsf_err_t vsfdwcotg_interrupt(void *param)
{
	struct dwcotg_t *dwcotg = (struct dwcotg_t *)param;
	uint32_t intsts = dwcotg->global_reg->gintmsk;
	intsts &= dwcotg->global_reg->gintsts;

	if (!intsts)
		return VSFERR_NONE;

	if (dwcotg->global_reg->gintsts & USB_OTG_GINTSTS_CMOD) // host mode
	{
		if (intsts & USB_OTG_GINTSTS_SOF)
		{
			uint8_t i;
			struct hc_t *hc = dwcotg->hc_pool;

			dwcotg->softick++;
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_SOF;

			for (i = 0; i < dwcotg->hc_amount; i++)
			{
				if ((hc[i].alloced) && (hc[i].hc_state == HC_WAIT))
				{
					submit_priv_urb(dwcotg, hc[i].owner_priv);
				}
			}
		}
		if (intsts & USB_OTG_GINTSTS_RXFLVL)
		{
			// TODO
		}
		if (intsts & USB_OTG_GINTSTS_NPTXFE)
		{
			// TODO
		}
		if (intsts & USB_OTG_GINTSTS_HPRTINT)
		{
			// TODO
		}
		if (intsts & USB_OTG_GINTSTS_HCINT)
		{
			uint8_t i;
			uint32_t hc_intsts = dwcotg->host_global_regs->haint;

			for (i = 0; i < dwcotg->hc_amount; i++)
			{
				if (hc_intsts & (0x1ul << i))
				{
					if (dwcotg->hc_regs[i].hcchar & USB_OTG_HCCHAR_EPDIR)
						vsfdwcotg_hc_in_handler(dwcotg, i);
					else
						vsfdwcotg_hc_out_handler(dwcotg, i);
				}
			}

			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_HCINT;
		}
		if (intsts & USB_OTG_GINTSTS_PTXFE)
		{
			// TODO
		}
	}
	else // device mode
	{
		// TODO
	}

	return VSFERR_NONE;
}

static vsf_err_t dwcotg_init_get_resource(struct vsfusbh_t *usbh,
		uint32_t reg_base)
{
	struct dwcotg_t *dwcotg;
	struct vsfdwcotg_hcd_param_t *hcd_param = usbh->hcd_param;

	dwcotg = vsf_bufmgr_malloc(sizeof(struct dwcotg_t));
	if (dwcotg == NULL)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	memset(dwcotg, 0, sizeof(struct dwcotg_t));
	usbh->hcd_data = dwcotg;

	// config init
	dwcotg->speed = hcd_param->speed;
	dwcotg->dma_en = hcd_param->dma_en;
	dwcotg->ulpi_en = hcd_param->ulpi_en;
	dwcotg->external_vbus_en = hcd_param->vbus_en;
	dwcotg->hc_amount = hcd_param->hc_amount;

	//dwcotg->ep_in_amount = VSFUSBD_CFG_MAX_IN_EP;
	//dwcotg->ep_out_amount = VSFUSBD_CFG_MAX_OUT_EP;
	dwcotg->ep_in_amount = 0;
	dwcotg->ep_out_amount = 0;

	// reg addr init
	dwcotg->global_reg =
			(struct dwcotg_core_global_regs_t *)(reg_base + 0);
	dwcotg->host_global_regs =
			(struct dwcotg_host_global_regs_t *)(reg_base + 0x400);
	dwcotg->hprt0 =
			(volatile uint32_t *)(reg_base + 0x440);
	dwcotg->hc_regs =
			(struct dwcotg_hc_regs_t *)(reg_base + 0x500);
	dwcotg->dev_global_regs =
			(struct dwcotg_dev_global_regs_t *)(reg_base + 0x800);
	dwcotg->in_ep_regs =
			(struct dwcotg_dev_in_ep_regs_t *)(reg_base + 0x900);
	dwcotg->out_ep_regs =
			(struct dwcotg_dev_out_ep_regs_t *)(reg_base + 0xb00);
	dwcotg->dfifo = (uint32_t *)(reg_base + 0x1000);

	dwcotg->hc_pool = vsf_bufmgr_malloc(sizeof(struct hc_t) *
			dwcotg->hc_amount);
	if (dwcotg->hc_pool == NULL)
	{
		vsf_bufmgr_free(dwcotg);
		usbh->hcd_data = NULL;
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}
	memset(dwcotg->hc_pool, 0, sizeof(struct hc_t) * dwcotg->hc_amount);

	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	uint8_t i;
	vsf_err_t err;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)pt->user_data;
	struct vsfdwcotg_hcd_param_t *hcd_param = usbh->hcd_param;
	struct dwcotg_t *dwcotg = (struct dwcotg_t *)usbh->hcd_data;

	vsfsm_pt_begin(pt);

	usbh->hcd_rh_speed = hcd_param->speed;
	err = dwcotg_init_get_resource(usbh,
			(uint32_t)vsfhal_hcd_regbase(hcd_param->index));
	if (err)
		return err;
	dwcotg = (struct dwcotg_t *)usbh->hcd_data;

	vsfhal_hcd_init(hcd_param->index, vsfdwcotg_interrupt, usbh->hcd_data);

	// disable global int
	dwcotg->global_reg->gahbcfg &= ~USB_OTG_GAHBCFG_GINT;

	if (dwcotg->ulpi_en)
	{
		// GCCFG &= ~(USB_OTG_GCCFG_PWRDWN)
		dwcotg->global_reg->gccfg &= ~USB_OTG_GCCFG_PWRDWN;

		// Init The ULPI Interface
		dwcotg->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_TSDPS |
				USB_OTG_GUSBCFG_ULPIFSLS | USB_OTG_GUSBCFG_PHYSEL);
		// Select vbus source
		dwcotg->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_ULPIEVBUSD |
				USB_OTG_GUSBCFG_ULPIEVBUSI);
		if (dwcotg->external_vbus_en)
		{
			dwcotg->global_reg->gusbcfg |= USB_OTG_GUSBCFG_ULPIEVBUSD;
		}

	}
	else
	{
		dwcotg->global_reg->gusbcfg |= USB_OTG_GUSBCFG_PHYSEL;
	}

	// Core Reset
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_AHBIDL) == 0)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;

		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}
	dwcotg->global_reg->grstctl |= USB_OTG_GRSTCTL_CSRST;
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_CSRST) ==
			USB_OTG_GRSTCTL_CSRST)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;

		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}

	if (dwcotg->ulpi_en == 0)
	{
		dwcotg->global_reg->gccfg = USB_OTG_GCCFG_PWRDWN;
	}

	if (dwcotg->dma_en)
	{
		dwcotg->global_reg->gahbcfg |= USB_OTG_GAHBCFG_DMAEN |
				USB_OTG_GAHBCFG_HBSTLEN_1 | USB_OTG_GAHBCFG_HBSTLEN_2;
	}

	// Force Host Mode
	dwcotg->global_reg->gusbcfg &= ~USB_OTG_GUSBCFG_FDMOD;
	dwcotg->global_reg->gusbcfg |= USB_OTG_GUSBCFG_FHMOD;

	vsfsm_pt_delay(pt, 50);

	// Enable Core
	// USBx->GCCFG |= USB_OTG_GCCFG_VBDEN;
	dwcotg->global_reg->gccfg |= USB_OTG_GCCFG_VBDEN;

	if (dwcotg->speed == USB_SPEED_HIGH)
	{
		dwcotg->host_global_regs->hcfg &= ~USB_OTG_HCFG_FSLSS;
	}
	else
	{
		dwcotg->host_global_regs->hcfg |= USB_OTG_HCFG_FSLSS;
	}

	// Flush FIFO
	dwcotg->global_reg->grstctl = USB_OTG_GRSTCTL_TXFFLSH |
			USB_OTG_GRSTCTL_TXFNUM_4;
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_TXFFLSH) ==
			USB_OTG_GRSTCTL_TXFFLSH)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;
		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}
	dwcotg->global_reg->grstctl = USB_OTG_GRSTCTL_RXFFLSH;
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_RXFFLSH) ==
			USB_OTG_GRSTCTL_RXFFLSH)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;
		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}

	// Clear all pending HC Interrupts
	for (i = 0; i < dwcotg->hc_amount; i++)
	{
		dwcotg->hc_regs[i].hcint = 0xffffffff;
		dwcotg->hc_regs[i].hcintmsk = 0;
	}

	vsfsm_pt_delay(pt, 10);

	// Disable all interrupts
	dwcotg->global_reg->gintmsk = 0;

	// Clear any pending interrupts
	dwcotg->global_reg->gintsts = 0xffffffff;

	// fifo size
	dwcotg->global_reg->grxfsiz = (((hcd_param->in_packet_size_max / 4) + 1) * 2);
	dwcotg->global_reg->gnptxfsiz = ((hcd_param->non_periodic_out_packet_size_max / 4) << 16) |
			(dwcotg->global_reg->grxfsiz & 0xffff);
	dwcotg->global_reg->hptxfsiz = ((hcd_param->periodic_out_packet_size_max / 4 * 2) << 16) |
			(dwcotg->global_reg->gnptxfsiz & 0xffff);

	if (dwcotg->dma_en == 0)
		dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_RXFLVLM;

	// Enable interrupts matching to the Host mode ONLY
	dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_HCIM | USB_OTG_GINTMSK_SOFM |
			USB_OTG_GINTMSK_PXFRM_IISOOXFRM;

	vsfsm_pt_delay(pt, 10);

	// enable global int
	dwcotg->global_reg->gahbcfg |= USB_OTG_GAHBCFG_GINT;

	dwcotg->dwcotg_state = DWCOTG_WORKING;

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_fini(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_suspend(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_resume(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_alloc_device(void *param, struct vsfusbh_device_t *dev)
{
	struct dwcotg_device_t *dev_priv = vsf_bufmgr_malloc(sizeof(struct dwcotg_device_t));
	if (dev_priv == NULL)
		return VSFERR_FAIL;
	memset(dev_priv, 0, sizeof(struct dwcotg_device_t));
	dev->priv = dev_priv;
	dev_priv->vsfdev = dev;
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_free_device(void *param, struct vsfusbh_device_t *dev)
{
	struct dwcotg_device_t *dev_priv = dev->priv;
	if (dev_priv->hc_num == 0)
		vsf_bufmgr_free(dev_priv);
	dev_priv->vsfdev = NULL;
	dev->priv = NULL;
	return VSFERR_NONE;
}

static struct vsfusbh_urb_t *dwcotgh_alloc_urb(void)
{
	uint32_t size;
	struct vsfusbh_urb_t *vsfurb;
	size = sizeof(struct vsfusbh_urb_t) - 4 + sizeof(struct urb_priv_t);
	vsfurb = vsf_bufmgr_malloc_aligned(size, 16);
	if (vsfurb == NULL)
		return NULL;
	memset(vsfurb, 0, size);
	return vsfurb;
}

static vsf_err_t dwcotgh_free_urb(void *param, struct vsfusbh_urb_t **vsfurbp)
{
	struct vsfusbh_urb_t *vsfurb = *vsfurbp;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;

	if (vsfurb == NULL)
		return VSFERR_FAIL;
	*vsfurbp = NULL;

	if (urb_priv->hc)
	{
		urb_priv->discarded = 1;
	}
	else
	{
		if (vsfurb->transfer_buffer != NULL)
			vsf_bufmgr_free(vsfurb->transfer_buffer);
		vsf_bufmgr_free(vsfurb);
	}
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_submit_urb(void *param, struct vsfusbh_urb_t *vsfurb)
{
	vsf_err_t err;
	struct hc_t *hc;
	struct dwcotg_t *dwcotg = (struct dwcotg_t *)param;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;
	uint32_t pipe = vsfurb->pipe;

	// rh address check
	if (usb_pipedevice(pipe) == 1)
		return VSFERR_FAIL;

	if (dwcotg->dwcotg_state == DWCOTG_DISABLE)
		return VSFERR_FAIL;

	memset(urb_priv, 0, sizeof(struct urb_priv_t));

	urb_priv->type = usb_pipetype(pipe);
	if (urb_priv->type == URB_PRIV_TYPE_CTRL)
		urb_priv->phase = URB_PRIV_PHASE_SETUP_WAIT;
	else if (urb_priv->type == URB_PRIV_TYPE_BULK)
		urb_priv->phase = URB_PRIV_PHASE_DATA_WAIT;
	else
		urb_priv->phase = URB_PRIV_PHASE_PERIOD_WAIT;

	if ((urb_priv->type == URB_PRIV_TYPE_INT) ||
		(urb_priv->type == URB_PRIV_TYPE_BULK))
	{
		urb_priv->toggle_start = vsfurb->vsfdev->toggle[usb_pipein(pipe)] &
				(0x1ul << usb_pipeendpoint(pipe));

		urb_priv->do_ping = dwcotg->dma_en ? 0 :
				(usb_pipespeed(pipe) == USB_SPEED_HIGH);
	}

	urb_priv->transfer_buffer = vsfurb->transfer_buffer;
	urb_priv->transfer_length = vsfurb->transfer_length;
	urb_priv->packet_size = vsfurb->packet_size;
	urb_priv->dir_o0_i1 = usb_pipein(pipe);

	hc = hc_init(dwcotg, (struct dwcotg_device_t *)vsfurb->vsfdev->priv);
	if (hc)
	{
		hc->dev_addr = usb_pipedevice(pipe);
		hc->ep_num = usb_pipeendpoint(pipe);
		hc->speed = usb_pipespeed(pipe);
		hc->owner_priv = urb_priv;
		urb_priv->hc = hc;

		err = submit_priv_urb(dwcotg, urb_priv);
		if (err == VSFERR_NONE)
			return VSFERR_NONE;
	}
	else
	{
		err = VSFERR_NOT_ENOUGH_RESOURCES;
	}

	vsfurb->status = URB_FAIL;
	return err;
}

// use for int/iso urb
static vsf_err_t dwcotgh_relink_urb(void *param, struct vsfusbh_urb_t *vsfurb)
{
	return submit_priv_urb((struct dwcotg_t *)param,
			(struct urb_priv_t *)vsfurb->urb_priv);
}



const static uint8_t root_hub_str_index0[] =
{
	0x04,				/* u8  bLength; */
	0x03,				/* u8  bDescriptorType; String-descriptor */
	0x09,				/* u8  lang ID */
	0x04,				/* u8  lang ID */
};
const static uint8_t root_hub_str_index1[] =
{
	30,					/* u8  bLength; */
	0x03,				/* u8  bDescriptorType; String-descriptor */
	'D',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'W',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'C',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'O',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'T',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'G',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'-',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'R',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'o',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'o',				/* u8  Unicode */
	0,					/* u8  Unicode */
	't',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'H',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'u',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'b',				/* u8  Unicode */
	0,					/* u8  Unicode */
};

static uint32_t rd_rh_portstat(volatile uint32_t *port)
{
	uint32_t dwcotg_port = *port;
	uint32_t value = 0;

	if ((dwcotg_port & USB_OTG_HPRT_PSPD) == 0)
		value |= USB_PORT_STAT_HIGH_SPEED;
	else if ((dwcotg_port & USB_OTG_HPRT_PSPD) == USB_OTG_HPRT_PSPD_1)
		value |= USB_PORT_STAT_LOW_SPEED;

	if (dwcotg_port & USB_OTG_HPRT_PCDET)
		value |= (USB_PORT_STAT_C_CONNECTION << 16);

	if (dwcotg_port & USB_OTG_HPRT_PENA)
		value |= USB_PORT_STAT_ENABLE;

	if (dwcotg_port & USB_OTG_HPRT_PCSTS)
		value |= USB_PORT_STAT_CONNECTION;

	return value;
}

static vsf_err_t dwcotgh_rh_control(void *param,
		struct vsfusbh_urb_t *vsfurb)
{
	//uint16_t typeReq, wValue, wIndex, wLength;
	uint16_t typeReq, wValue, wLength;
	struct dwcotg_t *dwcotg = (struct dwcotg_t *)param;
	struct usb_ctrlrequest_t *cmd = &vsfurb->setup_packet;
	uint32_t datadw[4];
	uint8_t *data = (uint8_t*)datadw;
	uint8_t len = 0;


	typeReq = (cmd->bRequestType << 8) | cmd->bRequest;
	wValue = cmd->wValue;
	//wIndex = cmd->wIndex;
	wLength = cmd->wLength;

	switch (typeReq)
	{
	case GetHubStatus:
		datadw[0] = 0;
		len = 4;
		break;
	case GetPortStatus:
		datadw[0] = rd_rh_portstat(dwcotg->hprt0);
		len = 4;
		break;
	case SetPortFeature:
		switch (wValue)
		{
		case(USB_PORT_FEAT_ENABLE):
			*dwcotg->hprt0 |= USB_OTG_HPRT_PENA;
			len = 0;
			break;
		case(USB_PORT_FEAT_RESET):
			if (*dwcotg->hprt0 & USB_OTG_HPRT_PCSTS)
				*dwcotg->hprt0 |= USB_OTG_HPRT_PRST;
			len = 0;
			break;
		case(USB_PORT_FEAT_POWER):
			*dwcotg->hprt0 |= USB_OTG_HPRT_PPWR;
			len = 0;
			break;
		default:
			goto error;
		}
		break;
	case ClearPortFeature:
		switch (wValue)
		{
		case(USB_PORT_FEAT_ENABLE):
			*dwcotg->hprt0 &= ~USB_OTG_HPRT_PENA;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_RESET):
			*dwcotg->hprt0 &= ~USB_OTG_HPRT_PRST;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_CONNECTION):
			*dwcotg->hprt0 |= USB_OTG_HPRT_PCDET;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_ENABLE):
			*dwcotg->hprt0 &= ~USB_OTG_HPRT_PENCHNG;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_SUSPEND):
			*dwcotg->hprt0 &= ~USB_OTG_HPRT_PSUSP;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_OVER_CURRENT):
			*dwcotg->hprt0 &= ~USB_OTG_HPRT_POCA;
			len = 0;
			break;
		default:
			goto error;
		}
		break;
	case GetHubDescriptor:
		data[0] = 9;			// min length;
		data[1] = 0x29;
		data[2] = 1;
		data[3] = 0x8;
		datadw[1] = 0;
		data[5] = 0;
		data[8] = 0xff;
		len = min(data[0], wLength);
		break;
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (wValue & 0xff00)
		{
		case USB_DT_STRING << 8:
			if (wValue == 0x0300)
			{
				data = (uint8_t *)root_hub_str_index0;
				len = min(sizeof(root_hub_str_index0), wLength);
			}
			if (wValue == 0x0301)
			{
				data = (uint8_t *)root_hub_str_index1;
				len = min(sizeof(root_hub_str_index0), wLength);
			}
			break;
		default:
			goto error;
		}
		break;
	default:
		goto error;
	}

	if (len)
	{
		if (vsfurb->transfer_length < len)
			len = vsfurb->transfer_length;
		vsfurb->actual_length = len;

		memcpy (vsfurb->transfer_buffer, data, len);
	}
	vsfurb->status = URB_OK;

	vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
	return VSFERR_NONE;

error:
	vsfurb->status = URB_FAIL;
	return VSFERR_FAIL;
}

const struct vsfusbh_hcddrv_t vsfdwcotgh_drv =
{
	.init_thread = dwcotgh_init_thread,
	.fini = dwcotgh_fini,
	.suspend = dwcotgh_suspend,
	.resume = dwcotgh_resume,
	.alloc_device = dwcotgh_alloc_device,
	.free_device = dwcotgh_free_device,
	.alloc_urb = dwcotgh_alloc_urb,
	.free_urb = dwcotgh_free_urb,
	.submit_urb = dwcotgh_submit_urb,
	.relink_urb = dwcotgh_relink_urb,
	.rh_control = dwcotgh_rh_control,
};

