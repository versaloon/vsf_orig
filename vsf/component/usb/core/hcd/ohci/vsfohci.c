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
#include "vsfohci_priv.h"

#define OHCI_ISO_DELAY			2

#define CC_TO_ERROR(cc) (cc == 0 ? VSFERR_NONE : VSFERR_FAIL)

#define usb_maxpacket(dev, pipe, out) (out \
							? (dev)->epmaxpacketout[usb_pipeendpoint(pipe)] \
							: (dev)->epmaxpacketin [usb_pipeendpoint(pipe)] )

static struct td_t *td_alloc(struct vsfohci_t *vsfohci)
{
	uint32_t i;
	struct td_t *td = NULL;
	for (i = 0; i < TD_MAX_NUM; i++)
	{
		if (vsfohci->td_pool[i].busy == 0)
		{
			td = &vsfohci->td_pool[i];
			memset(td, 0, sizeof(struct td_t));
			td->busy = 1;
			break;
		}
	}
	return td;
}

static void td_free(struct td_t *td)
{
	td->busy = 0;
}

static void urb_free_tds(struct urb_priv_t *urb_priv)
{
	uint32_t i;
	for (i = 0; i < urb_priv->length; i++)
	{
		if (urb_priv->td[i] != NULL)
		{
			td_free(urb_priv->td[i]);
			urb_priv->td[i] = NULL;
		}
	}
	urb_priv->length = 0;
	urb_priv->state &= ~URB_PRIV_TDALLOC;
}

static int8_t ed_balance(struct ohci_t *ohci, uint8_t interval, uint8_t load)
{
	int8_t i, j, branch = -1;

	// iso periods can be huge; iso tds specify frame numbers
	if (interval > NUM_INTS)
		interval = NUM_INTS;

	// search for the least loaded schedule branch of that period
	// that has enough bandwidth left unreserved.
	for (i = 0; i < interval; i++)
	{
		if (branch < 0 || ohci->load[branch] < ohci->load[i])
		{
			for (j = i; j < NUM_INTS; j += interval)
			{
				// usb 1.1 says 90% of one frame
				if (ohci->load[j] + load > 900)
					break;
			}
			if (j < NUM_INTS)
				continue;
			branch = i;
		}
	}
	return branch;
}

static void periodic_link(struct ohci_t *ohci, struct ed_t *ed)
{
	int8_t i;

	for (i = ed->branch; i < NUM_INTS; i += ed->interval)
	{
		struct ed_t **prev = (struct ed_t **)&ohci->hcca->int_table[i];
		struct ed_t *here = *prev;

		// sorting each branch by period (slow before fast)
		while (here && ed != here)
		{
			if (ed->interval > here->interval)
				break;
			prev = (struct ed_t **)&here->hwNextED;
			here = *prev;
		}
		if (ed != here)
		{
			ed->hwNextED = (uint32_t)here;
			*prev = ed;
		}
		ohci->load[i] += ed->load;
	}
}

static void periodic_unlink(struct ohci_t *ohci, struct ed_t *ed)
{
	int8_t i;

	for (i = ed->branch; i < NUM_INTS; i += ed->interval)
	{
		struct ed_t **prev = (struct ed_t **)&ohci->hcca->int_table[i];

		while (*prev && (*prev != ed))
		{
			prev = (struct ed_t **)&((*prev)->hwNextED);
		}

		if (*prev)
		{
			*prev = (struct ed_t *)ed->hwNextED;
		}
		ohci->load[i] -= ed->load;
	}
}

// link an ed into one of the HC chains
static vsf_err_t ed_schedule(struct ohci_t *ohci, struct urb_priv_t *urb_priv)
{
	struct ed_t *ed = urb_priv->ed;
	ed->hwNextED = 0;

	vsf_enter_critical();
	switch (ed->type)
	{
	case PIPE_CONTROL:
		if (ohci->ed_controltail == NULL)
			ohci->regs->ed_controlhead = (uint32_t)ed;
		else
			ohci->ed_controltail->hwNextED = (uint32_t)ed;
		ed->prev = ohci->ed_controltail;
		if (!ohci->ed_controltail && !ohci->ed_rm_list)
		{
			ohci->hc_control |= OHCI_CTRL_CLE;
			ohci->regs->ed_controlcurrent = 0;
			ohci->regs->control = ohci->hc_control;
		}
		ohci->ed_controltail = ed;
		break;
	case PIPE_BULK:
		if (ohci->ed_bulktail == NULL)
			ohci->regs->ed_bulkhead = (uint32_t)ed;
		else
			ohci->ed_bulktail->hwNextED = (uint32_t)ed;
		ed->prev = ohci->ed_bulktail;
		if (!ohci->ed_bulktail && !ohci->ed_rm_list)
		{
			ohci->hc_control |= OHCI_CTRL_BLE;
			ohci->regs->ed_bulkcurrent = 0;
			ohci->regs->control = ohci->hc_control;
		}
		ohci->ed_bulktail = ed;
		break;
	case PIPE_INTERRUPT:
	case PIPE_ISOCHRONOUS:
		if ((ed->interval == 0) || (ed->interval > 32))
			ed->interval = 32;
		ed->branch = ed_balance(ohci, ed->interval, ed->load);
		if (ed->branch < 0)
			return VSFERR_NOT_ENOUGH_RESOURCES;
		periodic_link(ohci, ed);
		break;
	}
	urb_priv->state |= URB_PRIV_EDLINK;
	vsf_leave_critical();

	return VSFERR_NONE;
}

static void ed_deschedule(struct ohci_t *ohci, struct urb_priv_t *urb_priv)
{
	struct ed_t *ed = urb_priv->ed;
	ed->hwINFO |= ED_SKIP;
	urb_priv->state |= URB_PRIV_EDSKIP;

	switch (ed->type)
	{
	case PIPE_CONTROL:
		if (ed->prev == NULL)
		{
			if (!ed->hwNextED)
			{
				ohci->hc_control &= ~OHCI_CTRL_CLE;
				ohci->regs->control = ohci->hc_control;
			}
			else
				ohci->regs->ed_controlhead = ed->hwNextED;
		}
		else
		{
			ed->prev->hwNextED = ed->hwNextED;
		}
		if (ohci->ed_controltail == ed)
			ohci->ed_controltail = ed->prev;
		else
			((struct ed_t *)(ed->hwNextED))->prev = ed->prev;
		break;
	case PIPE_BULK:
		if (ed->prev == NULL)
		{
			if (!ed->hwNextED)
			{
				ohci->hc_control &= ~OHCI_CTRL_BLE;
				ohci->regs->control = ohci->hc_control;
			}
			else
				ohci->regs->ed_bulkhead = ed->hwNextED;
		}
		else
		{
			ed->prev->hwNextED = ed->hwNextED;
		}

		if (ohci->ed_bulktail == ed)
			ohci->ed_bulktail = ed->prev;
		else
			((struct ed_t *)(ed->hwNextED))->prev = ed->prev;
		break;
	case PIPE_INTERRUPT:
	case PIPE_ISOCHRONOUS:
		periodic_unlink(ohci, ed);
		break;
	}
	urb_priv->state &= ~URB_PRIV_EDLINK;
}

static vsf_err_t ed_init(struct vsfohci_t *vsfohci,
		struct vsfusbh_urb_t *vsfurb, struct urb_priv_t *urb_priv)
{
	uint32_t pipe, interval, c;
	struct ed_t *ed = urb_priv->ed;
	struct vsfusbh_device_t *vsfdev = vsfurb->vsfdev;
	struct td_t *td = NULL;

	if (ed == NULL)
	{
		// ERROR
		return VSFERR_FAIL;
	}
	c = ed->hwHeadP & 0x2;
	memset(ed, 0, sizeof(struct ed_t));

	pipe = vsfurb->pipe;

	/* dummy td; end of td list for ed */
	td = td_alloc(vsfohci);
	if (!td)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	td->urb_priv = urb_priv;
	ed->dummy_td = td;
	ed->hwTailP = (uint32_t)td;
	ed->hwHeadP = ed->hwTailP | c;
	ed->type = usb_pipetype(pipe);
	ed->hwINFO = usb_pipedevice(pipe)
			| usb_pipeendpoint(pipe) << 7
			| (usb_pipeisoc(pipe) ? 0x8000 : 0)
			| usb_pipeslow(pipe) << 13
			| usb_maxpacket(vsfdev, pipe, usb_pipeout(pipe)) << 16;

	if (ed->type != PIPE_CONTROL)
	{
		ed->hwINFO |= usb_pipeout(pipe) ? 0x800 : 0x1000;

		if (ed->type != PIPE_BULK)
		{
			interval = vsfurb->interval;
			if (ed->type == PIPE_ISOCHRONOUS)
				ed->hwINFO |= 0x8000;
			else if (interval > 32)
				interval = 32;

			ed->interval = interval;
			ed->load = 1;
		}
	}
	return VSFERR_NONE;
}
static void ed_fini(struct ed_t *ed)
{
	td_free(ed->dummy_td);
	ed->dummy_td = NULL;
}

static void td_fill(uint32_t info, void *data, uint16_t len, uint16_t index,
		struct urb_priv_t *urb_priv)
{
	struct td_t *td, *td_pt;

	if (index > urb_priv->length)
		return;

	td_pt = urb_priv->td[index];

	// fill the old dummy TD
	td = urb_priv->td[index] = urb_priv->ed->dummy_td;
	urb_priv->ed->dummy_td = td_pt;

	td_pt->urb_priv = urb_priv;

	td->next_dl_td = NULL;
	td->index = index;

	td->hwINFO = info;
	td->hwCBP = (uint32_t)((!data || !len) ? 0 : data);
	td->hwBE = (uint32_t)((!data || !len) ? 0 : (uint32_t)data + len - 1);
	td->hwNextTD = (uint32_t)td_pt;

	urb_priv->ed->hwTailP = td->hwNextTD;
}

#if OHCI_ENABLE_ISO
static void iso_td_fill(uint32_t info, void *data, uint16_t len, uint16_t index,
		struct urb_priv_t *urb_priv)
{
	struct td_t *td, *td_pt;
	uint32_t bufferStart;
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);

	if (index > urb_priv->length)
		return;

	td_pt = urb_priv->td[index];

	// fill the old dummy TD
	td = urb_priv->td[index] = (struct td_t *)\
			((uint32_t)urb_priv->ed->hwTailP & 0xfffffff0);
	td->next_dl_td = NULL;
	td->index = index;
	td->urb_priv = urb_priv;

	bufferStart = (uint32_t)data + urb_priv->iso_frame_desc[index].offset;
	len = urb_priv->iso_frame_desc[index].length;

	td->hwINFO = info;
	td->hwCBP = (uint32_t)((!bufferStart || !len) ? 0 : bufferStart) & 0xfffff000;
	td->hwBE = (uint32_t)((!bufferStart || !len) ? 0 : (uint32_t)bufferStart + len - 1);
	td->hwNextTD = (uint32_t)td_pt;

	td->hwPSW[0] = ((uint32_t)data + urb_priv->iso_frame_desc[index].offset) & 0x0FFF | 0xE000;

	td_pt->hwNextTD = 0;
	urb_priv->ed->hwTailP = td->hwNextTD;
}
#endif // OHCI_ENABLE_ISO

static void td_submit_urb(struct ohci_t *ohci, struct vsfusbh_urb_t *vsfurb)
{
	uint32_t data_len, info = 0, toggle = 0, cnt = 0;
	void *data;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;

	data = vsfurb->transfer_buffer;
	data_len = vsfurb->transfer_length;

	if (usb_gettoggle(vsfurb->vsfdev, usb_pipeendpoint(vsfurb->pipe), usb_pipeout(vsfurb->pipe)))
	{
		toggle = TD_T_TOGGLE;
	}
	else
	{
		toggle = TD_T_DATA0;
		usb_settoggle(vsfurb->vsfdev, usb_pipeendpoint(vsfurb->pipe), usb_pipeout(vsfurb->pipe), 1);
	}
	urb_priv->td_cnt = 0;

	switch (usb_pipetype(vsfurb->pipe))
	{
	case PIPE_CONTROL:
		info = TD_CC | TD_DP_SETUP | TD_T_DATA0;
		td_fill(info, (void *)&vsfurb->setup_packet, 8, cnt++, urb_priv);
		if (data_len > 0)
		{
			info = usb_pipeout(vsfurb->pipe) ?
					TD_CC | TD_R | TD_DP_OUT | TD_T_DATA1 :
					TD_CC | TD_R | TD_DP_IN | TD_T_DATA1;
			td_fill(info, data, data_len, cnt++, urb_priv);
		}
		info = (usb_pipeout(vsfurb->pipe) || data_len == 0) ?
				(TD_CC | TD_DP_IN | TD_T_DATA1) :
				(TD_CC | TD_DP_OUT | TD_T_DATA1);
		td_fill(info, NULL, 0, cnt++, urb_priv);
		ohci->regs->cmdstatus = OHCI_CLF;
		break;
	case PIPE_BULK:
		info = usb_pipeout(vsfurb->pipe) ? (TD_CC | TD_DP_OUT) : (TD_CC | TD_DP_IN);
		while (data_len > 4096)
		{
			td_fill(info | (cnt ? TD_T_TOGGLE : toggle), data, 4096, cnt, urb_priv);
			data = (void *)((uint32_t)data + 4096);
			data_len -= 4096;
			cnt++;
		}
		info = usb_pipeout(vsfurb->pipe) ?
				(TD_CC | TD_DP_OUT) : (TD_CC | TD_R | TD_DP_IN);
		td_fill(info | (cnt ? TD_T_TOGGLE : toggle), data, data_len, cnt, urb_priv);
		cnt++;
		ohci->regs->cmdstatus = OHCI_BLF;
		break;
	case PIPE_INTERRUPT:
		info = usb_pipeout(vsfurb->pipe) ? (TD_CC | TD_DP_OUT | toggle) :
				(TD_CC | TD_R | TD_DP_IN | toggle);
		td_fill(info, data, data_len, cnt++, urb_priv);
		break;
#if OHCI_ENABLE_ISO
	case PIPE_ISOCHRONOUS:
		for (cnt = urb_priv->td_cnt; cnt < urb_priv->number_of_packets; cnt++)
		{
			iso_td_fill(TD_CC | TD_ISO | ((vsfurb->start_frame + cnt) & 0xffff),
					data, data_len, cnt, urb_priv);
		}
		break;
#endif // OHCI_ENABLE_ISO
	}
	urb_priv->state |= URB_PRIV_TDLINK;
}

static void update_done_list(struct ohci_t *ohci)
{
	uint8_t cc;
	struct td_t *td = NULL, *td_next = NULL;

	td_next = (struct td_t *)(ohci->hcca->done_head & 0xfffffff0);
	ohci->hcca->done_head = 0;

	while (td_next)
	{
		td = td_next;
		td_next = (struct td_t *)(td->hwNextTD & 0xfffffff0);

		td->hwINFO |= TD_DEL;
		cc = TD_CC_GET(td->hwINFO);

		// cc get error and ed halted
		if ((cc != TD_CC_NOERROR) && (td->urb_priv->ed->hwHeadP & 0x01))
		{
			uint8_t i;
			struct urb_priv_t *urb_priv = td->urb_priv;
			struct ed_t *ed = urb_priv->ed;

			ed->hwINFO |= ED_SKIP;
			urb_priv->state |= URB_PRIV_EDSKIP;
			ed->hwHeadP &= ~ED_H;

			for(i = 0; i < urb_priv->length; i++)
			{
				if (urb_priv->td[i] == td)
				{
					urb_priv->td_cnt += urb_priv->length - 1 - i;
				}
			}
		}

		// add to done list
		td->next_dl_td = ohci->dl_start;
		ohci->dl_start = td;
	}
}

static void finish_unlinks(struct ohci_t *ohci)
{
	uint8_t frame = ohci->hcca->frame_no & 0x01;
	struct ed_t *ed, **last;

	for(last = &ohci->ed_rm_list, ed = *last; ed != NULL; ed = *last)
	{
		if (ed->rm_frame != frame)
		{
			struct urb_priv_t *urb_priv = ed->dummy_td->urb_priv;
			struct vsfusbh_urb_t *vsfurb = container_of(urb_priv,
					struct vsfusbh_urb_t, urb_priv);

			*last = ed->ed_next;

			urb_free_tds(urb_priv);
			ed_fini(urb_priv->ed);
			urb_priv->state &= ~URB_PRIV_EDSKIP;

			if (urb_priv->state & URB_PRIV_WAIT_DELETE)
			{
				if (vsfurb->transfer_buffer != NULL)
					vsf_bufmgr_free(vsfurb->transfer_buffer);
				vsf_bufmgr_free(vsfurb);
			}
			else if (urb_priv->state & URB_PRIV_WAIT_COMPLETE)
			{
				vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
				urb_priv->state &= ~URB_PRIV_WAIT_COMPLETE;
			}
		}
		else
		{
			last = &ed->ed_next;
		}
	}
}

static vsf_err_t td_done(struct vsfusbh_urb_t *vsfurb, struct td_t *td)
{
	int32_t cc = 0, err = VSFERR_NONE;

#if OHCI_ENABLE_ISO
	if (td->hwINFO & TD_ISO)
	{
		uint16_t tdPSW;
		uint32_t dlen;

		tdPSW = td->hwPSW[0] & 0xffff;
		cc = (tdPSW >> 12) & 0xf;

		if (td->hwINFO & TD_CC)
			return VSFERR_FAIL;

		if (usb_pipeout(vsfurb->pipe))
			dlen = urb_priv->iso_frame_desc[td->index].length;
		else
		{
			if (cc == TD_DATAUNDERRUN)
				cc = TD_CC_NOERROR;
			dlen = tdPSW & 0x3ff;
		}
		vsfurb->actual_length += dlen;
		urb_priv->iso_frame_desc[td->index].actual_length = dlen;
		urb_priv->iso_frame_desc[td->index].status = CC_TO_ERROR(cc);
	}
	else
#endif // OHCI_ENABLE_ISO
	{
		cc = (td->hwINFO >> 28) & 0xf;

		if ((cc == TD_DATAUNDERRUN) &&
				!(vsfurb->transfer_flags & URB_SHORT_NOT_OK))
			cc = TD_CC_NOERROR;

		if (cc != TD_CC_NOERROR && cc < 0x0e)
			err = CC_TO_ERROR(cc);

		if (((usb_pipetype(vsfurb->pipe) != PIPE_CONTROL) || td->index != 0) &&
				(td->hwBE != 0))
		{
			if (td->hwCBP == 0)
				vsfurb->actual_length = td->hwBE - (uint32_t)vsfurb->transfer_buffer + 1;
			else
				vsfurb->actual_length = td->hwCBP - (uint32_t)vsfurb->transfer_buffer;
		}
	}

	return err;
}

static void start_ed_unlink(struct ohci_t *ohci, struct urb_priv_t *urb_priv)
{
	struct ed_t *ed = urb_priv->ed;

	ed->hwINFO |= ED_DEQUEUE;
	ed_deschedule(ohci, urb_priv);

	ed->ed_next = ohci->ed_rm_list;
	ohci->ed_rm_list = ed;
	ed->prev = NULL;

	ohci->regs->intrstatus = OHCI_INTR_SF;
	ohci->regs->intrenable = OHCI_INTR_SF;

	ed->rm_frame = (ohci->hcca->frame_no + 1) & 0x1;
}

static void takeback_td(struct ohci_t *ohci, struct td_t *td)
{
	vsf_err_t err;
	struct urb_priv_t *urb_priv = td->urb_priv;
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);

	err = td_done(vsfurb, td);
	urb_priv->td_cnt++;

	if (urb_priv->td_cnt >= urb_priv->length)
	{
		urb_priv->state &= ~URB_PRIV_TDLINK;
		vsfurb->status = err;

		if (urb_priv->state & URB_PRIV_WAIT_DELETE)
		{
			start_ed_unlink(ohci, urb_priv);
		}
		else
		{
			if ((usb_pipetype(vsfurb->pipe) == PIPE_BULK) ||
					(usb_pipetype(vsfurb->pipe) == PIPE_CONTROL))
			{
				// pend complete event after ed unlink
				urb_priv->state |= URB_PRIV_WAIT_COMPLETE;
				start_ed_unlink(ohci, urb_priv);
			}
			else
			{
				vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
			}
		}
	}
}

// TD takeback and URB giveback must be single-threaded.
// This routine takes care of it all.
static void ohci_work(struct ohci_t *ohci)
{
	struct td_t *td;

	if (ohci->working)
	{
		ohci->restart_work = 1;
		return;
	}
	ohci->working = 1;

restart:
	// process done list
	while (ohci->dl_start)
	{
		vsf_enter_critical();
		td = ohci->dl_start;
		ohci->dl_start = td->next_dl_td;
		vsf_leave_critical();
		takeback_td(ohci, td);
	}

	finish_unlinks(ohci);

	if (ohci->restart_work)
	{
		ohci->restart_work = 0;
		goto restart;
	}
	ohci->working = 0;
}

static int32_t vsfohci_interrupt(void *param)
{
	struct vsfohci_t *vsfohci = (struct vsfohci_t *)param;
	struct ohci_t *ohci = vsfohci->ohci;
	uint32_t intrstatus = ohci->regs->intrstatus;

	intrstatus &= ohci->regs->intrenable;

	if (intrstatus & OHCI_INTR_UE)
	{
		ohci->disabled++;
		vsfohci->ohci->regs->intrdisable = OHCI_INTR_MIE;
		vsfohci->ohci->regs->control = 0;
		vsfohci->ohci->regs->cmdstatus = OHCI_HCR;
		vsfohci->ohci->hc_control = OHCI_USB_RESET;
		return -1;
	}

	if (intrstatus & OHCI_INTR_RHSC)
	{
		ohci->regs->intrstatus = OHCI_INTR_RHSC | OHCI_INTR_RD;
		ohci->regs->intrdisable = OHCI_INTR_RHSC;
	}
	else if (intrstatus & OHCI_INTR_RD)
	{
		ohci->regs->intrstatus = OHCI_INTR_RD;
		ohci->regs->intrdisable = OHCI_INTR_RD;
	}

	if (intrstatus & OHCI_INTR_WDH)
		update_done_list(ohci);

	ohci_work(ohci);

	if ((intrstatus & OHCI_INTR_SF) && !ohci->ed_rm_list)
		ohci->regs->intrdisable = OHCI_INTR_SF;

	ohci->regs->intrstatus = intrstatus;
	ohci->regs->intrenable = OHCI_INTR_MIE;

	return 0;
}

static vsf_err_t vsfohci_init_get_resource(struct vsfusbh_t *usbh,
		uint32_t reg_base)
{
	struct vsfohci_t *vsfohci;

	vsfohci = vsf_bufmgr_malloc(sizeof(struct vsfohci_t));
	if (vsfohci == NULL)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	memset(vsfohci, 0, sizeof(struct vsfohci_t));
	usbh->hcd_data = vsfohci;

	vsfohci->ohci = vsf_bufmgr_malloc(sizeof(struct ohci_t));
	if (vsfohci->ohci == NULL)
		goto err_failed_alloc_ohci;
	memset(vsfohci->ohci, 0, sizeof(struct ohci_t));
	vsfohci->ohci->vsfohci = vsfohci;

	vsfohci->ohci->hcca = vsf_bufmgr_malloc_aligned(sizeof(struct ohci_hcca_t),
			256);
	if (vsfohci->ohci->hcca == NULL)
		goto err_failed_alloc_hcca;
	memset(vsfohci->ohci->hcca, 0, sizeof(struct ohci_hcca_t));

	vsfohci->td_pool = vsf_bufmgr_malloc_aligned(sizeof(struct td_t) *\
			TD_MAX_NUM, 32);
	if (vsfohci->td_pool == NULL)
		goto err_failed_alloc_ohci_td;
	memset(vsfohci->td_pool, 0, sizeof(struct td_t) * TD_MAX_NUM);

	__asm("NOP");

	vsfohci->ohci->regs = (struct ohci_regs_t *)reg_base;
	return VSFERR_NONE;

err_failed_alloc_ohci_td:
	vsf_bufmgr_free(vsfohci->ohci->hcca);
err_failed_alloc_hcca:
	vsf_bufmgr_free(vsfohci->ohci);
err_failed_alloc_ohci:
	vsf_bufmgr_free(vsfohci);
	usbh->hcd_data = NULL;
	return VSFERR_NOT_ENOUGH_RESOURCES;
}

static uint32_t vsfohci_init_hc_start(struct vsfohci_t *vsfohci)
{
	uint32_t temp;
	struct ohci_t *ohci = vsfohci->ohci;

	ohci->disabled = 1;

	ohci->regs->ed_controlhead = 0;
	ohci->regs->ed_bulkhead = 0;

	ohci->regs->hcca = (uint32_t)ohci->hcca;

	ohci->hc_control = OHCI_CONTROL_INIT | OHCI_USB_OPER;
	ohci->disabled = 0;
	ohci->regs->control = ohci->hc_control;

	ohci->regs->fminterval = 0x2edf | (((0x2edf - 210) * 6 / 7) << 16);
	ohci->regs->periodicstart = (0x2edf * 9) / 10;
	ohci->regs->lsthresh = 0x628;

	temp = OHCI_INTR_MIE | OHCI_INTR_UE | OHCI_INTR_WDH | OHCI_INTR_SO;
	ohci->regs->intrstatus = temp;
	ohci->regs->intrenable = temp;

	temp = ohci->regs->roothub.a & ~(RH_A_PSM | RH_A_OCPM);
	ohci->regs->roothub.a = temp;
	ohci->regs->roothub.status = RH_HS_LPSC;

	return (ohci->regs->roothub.a >> 23) & 0x1fe;
}

static vsf_err_t vsfohci_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfohci_t *vsfohci;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)pt->user_data;
	struct vsfohci_hcd_param_t *hcd_param = usbh->hcd_param;
	vsfohci = (struct vsfohci_t *)usbh->hcd_data;

	vsfsm_pt_begin(pt);

	usbh->hcd_rh_speed = USB_SPEED_FULL;
	err = vsfohci_init_get_resource(usbh,
			(uint32_t)vsfhal_hcd_regbase(hcd_param->index));
	if (err)
		return err;
	vsfohci = (struct vsfohci_t *)usbh->hcd_data;

	vsfhal_hcd_init(hcd_param->index, vsfohci_interrupt, usbh->hcd_data);

	vsfohci->ohci->regs->intrdisable = OHCI_INTR_MIE;
	vsfohci->ohci->regs->control = 0;
	vsfohci->ohci->regs->cmdstatus = OHCI_HCR;
	vsfohci->loops = 30;
	while ((vsfohci->ohci->regs->cmdstatus & OHCI_HCR) != 0)
	{
		if (--vsfohci->loops == 0)
		{
			return VSFERR_FAIL;
		}
		vsfsm_pt_delay(pt, 10);
	}

	vsfohci->ohci->hc_control = OHCI_USB_RESET;

	vsfsm_pt_delay(pt, 100);

	vsfsm_pt_delay(pt, vsfohci_init_hc_start(vsfohci));

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static vsf_err_t vsfohci_fini(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t vsfohci_suspend(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t vsfohci_resume(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t vsfohci_alloc_device(void *param, struct vsfusbh_device_t *dev)
{
	dev->priv = NULL;
	return VSFERR_NONE;
}

static vsf_err_t vsfohci_free_device(void *param, struct vsfusbh_device_t *dev)
{
	return VSFERR_NONE;
}

static struct vsfusbh_urb_t *vsfohci_alloc_urb(void)
{
	uint32_t size;
	struct vsfusbh_urb_t *vsfurb;
	struct urb_priv_t *urb_priv;
	size = sizeof(struct vsfusbh_urb_t) - 4 + sizeof(struct urb_priv_t);
	size = ((size + 15) & 0xfffffff0) + sizeof(struct ed_t);
	vsfurb = vsf_bufmgr_malloc_aligned(size, 16);
	if (vsfurb == NULL)
		return NULL;
	memset(vsfurb, 0, size);
	urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;
	urb_priv->ed = (struct ed_t *)
			((uint32_t)vsfurb + size - sizeof(struct ed_t));
	return vsfurb;
}

static vsf_err_t vsfohci_free_urb(void *param, struct vsfusbh_urb_t **vsfurbp)
{
	struct vsfohci_t *vsfohci = (struct vsfohci_t *)param;
	struct ohci_t *ohci = vsfohci->ohci;
	struct vsfusbh_urb_t *vsfurb = *vsfurbp;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;

	if (vsfurb == NULL)
		return VSFERR_FAIL;
	*vsfurbp = NULL;

	if (urb_priv->state)
	{
		urb_priv->state &= ~URB_PRIV_WAIT_COMPLETE;
		urb_priv->state |= URB_PRIV_WAIT_DELETE;

		if ((urb_priv->state & (URB_PRIV_EDLINK | URB_PRIV_TDALLOC)) ==
				(URB_PRIV_EDLINK | URB_PRIV_TDALLOC))
		{
			start_ed_unlink(ohci, urb_priv);
		}
		else
		{
			// ERROR
		}
	}
	else
	{
		if (vsfurb->transfer_buffer != NULL)
			vsf_bufmgr_free(vsfurb->transfer_buffer);
		vsf_bufmgr_free(vsfurb);
	}
	return VSFERR_NONE;
}

static vsf_err_t vsfohci_submit_urb(void *param, struct vsfusbh_urb_t *vsfurb)
{
	uint32_t i, size = 0;
	struct ed_t *ed;
	struct vsfohci_t *vsfohci = (struct vsfohci_t *)param;
	struct ohci_t *ohci = vsfohci->ohci;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;

	if (ohci->disabled)
		return VSFERR_FAIL;

	ed_init(vsfohci, vsfurb, urb_priv);

	switch (usb_pipetype(vsfurb->pipe))
	{
	case PIPE_CONTROL:/* 1 TD for setup, 1 for ACK and 1 for every 4096 B */
		size = (vsfurb->transfer_length == 0) ?
				2 : (vsfurb->transfer_length - 1) / 4096 + 3;
		break;
	case PIPE_BULK:
		size = (vsfurb->transfer_length - 1) / 4096 + 1;
		break;
	case PIPE_INTERRUPT:
		size = 1;
		break;
#if OHCI_ENABLE_ISO
	case PIPE_ISOCHRONOUS:
		size = urb_priv->number_of_packets;
		if (size == 0)
			return VSFERR_FAIL;
		for (i = 0; i < size; i++)
		{
			urb_priv->iso_frame_desc[i].actual_length = 0;
			urb_priv->iso_frame_desc[i].status = VSFERR_FAIL;
		}
		break;
#endif // OHCI_ENABLE_ISO
	}

	if (size > TD_MAX_NUM_EACH_URB)
		return VSFERR_FAIL;

	ed = urb_priv->ed;
	memset(urb_priv, 0, sizeof(struct urb_priv_t));
	urb_priv->ed = ed;
	urb_priv->length = size;

	for (i = 0; i < size; i++)
	{
		urb_priv->td[i] = td_alloc(vsfohci);
		if (NULL == urb_priv->td[i])
		{
			ed_fini(urb_priv->ed);
			urb_free_tds(urb_priv);
			return VSFERR_FAIL;
		}
	}
	urb_priv->state |= URB_PRIV_TDALLOC;

#if OHCI_ENABLE_ISO
	if (usb_pipetype(vsfurb->pipe) == PIPE_ISOCHRONOUS)
	{
		vsfurb->start_frame = (ohci->hcca->frame_no + OHCI_ISO_DELAY) & 0xffff;
	}
#endif // OHCI_ENABLE_ISO

	vsfurb->actual_length = 0;
	vsfurb->status = URB_PENDING;

	ed_schedule(ohci, urb_priv);
	td_submit_urb(ohci, vsfurb);

	return VSFERR_NONE;
}

// use for int/iso urb
static vsf_err_t vsfohci_relink_urb(void *param, struct vsfusbh_urb_t *vsfurb)
{
	struct vsfohci_t *vsfohci = (struct vsfohci_t *)param;
	struct ohci_t *ohci = vsfohci->ohci;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;

	switch (usb_pipetype(vsfurb->pipe))
	{
	case PIPE_INTERRUPT:
		vsfurb->actual_length = 0;
		if (urb_priv->state == (URB_PRIV_EDLINK | URB_PRIV_TDALLOC))
		{
			vsfurb->status = URB_PENDING;
			td_submit_urb(ohci, vsfurb);
			return VSFERR_NONE;
		}
		break;
#if OHCI_ENABLE_ISO
	case PIPE_ISOCHRONOUS:
		vsfurb->actual_length = 0;
		if (urb_priv->state == (URB_PRIV_EDLINK | URB_PRIV_TDALLOC))
		{
			uint32_t i;
			// NOTE: iso transfer interval fixed to 1
			vsfurb->start_frame = (ohci->hcca->frame_no + 1) & 0xffff;
			for (i = 0; i < urb_priv->number_of_packets; i++)
				urb_priv->iso_frame_desc[i].actual_length = 0;
			td_submit_urb(ohci, vsfurb);
			return VSFERR_NONE;
		}
#endif
	default:
		break;
	}
	return VSFERR_FAIL;
}

static const uint8_t root_hub_str_index0[] =
{
	0x04,				/* u8  bLength; */
	0x03,				/* u8  bDescriptorType; String-descriptor */
	0x09,				/* u8  lang ID */
	0x04,				/* u8  lang ID */
};
static const uint8_t root_hub_str_index1[] =
{
	28,					/* u8  bLength; */
	0x03,				/* u8  bDescriptorType; String-descriptor */
	'O',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'H',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'C',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'I',				/* u8  Unicode */
	0,					/* u8  Unicode */
	' ',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'R',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'o',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'o',				/* u8  Unicode */
	0,					/* u8  Unicode */
	't',				/* u8  Unicode */
	0,					/* u8  Unicode */
	' ',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'H',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'u',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'b',				/* u8  Unicode */
	0,					/* u8  Unicode */
};

static vsf_err_t vsfohci_rh_control(void *param, struct vsfusbh_urb_t *vsfurb)
{
	uint16_t typeReq, wValue, wIndex, wLength;
	struct vsfohci_t *vsfohci = (struct vsfohci_t *)param;
	struct ohci_regs_t *regs = vsfohci->ohci->regs;
	struct usb_ctrlrequest_t *cmd = &vsfurb->setup_packet;
	uint32_t datadw[4], temp;
	uint8_t *data = (uint8_t*)datadw;
	uint8_t len = 0;


	typeReq = (cmd->bRequestType << 8) | cmd->bRequest;
	wValue = cmd->wValue;
	wIndex = cmd->wIndex;
	wLength = cmd->wLength;

	switch (typeReq)
	{
	case GetHubStatus:
		datadw[0] = RD_RH_STAT;
		len = 4;
		break;
	case GetPortStatus:
		datadw[0] = RD_RH_PORTSTAT;
		len = 4;
		break;
	case SetPortFeature:
		switch (wValue)
		{
		case(RH_PORT_SUSPEND):
			WR_RH_PORTSTAT(RH_PS_PSS);
			len = 0;
			break;
		case(RH_PORT_RESET):	/* BUG IN HUP CODE *********/
			if (RD_RH_PORTSTAT & RH_PS_CCS)
				WR_RH_PORTSTAT(RH_PS_PRS);
			len = 0;
			break;
		case(RH_PORT_POWER):
			WR_RH_PORTSTAT(RH_PS_PPS);
			len = 0;
			break;
		case(RH_PORT_ENABLE):	/* BUG IN HUP CODE *********/
			if (RD_RH_PORTSTAT & RH_PS_CCS)
				WR_RH_PORTSTAT(RH_PS_PES);
			len = 0;
			break;
		default:
			goto error;
		}
		break;
	case ClearPortFeature:
		switch (wValue)
		{
		case(RH_PORT_ENABLE):
			WR_RH_PORTSTAT(RH_PS_CCS);
			len = 0;
			break;
		case(RH_PORT_SUSPEND):
			WR_RH_PORTSTAT(RH_PS_POCI);
			len = 0;
			break;
		case(RH_PORT_POWER):
			WR_RH_PORTSTAT(RH_PS_LSDA);
			len = 0;
			break;
		case(RH_C_PORT_CONNECTION):
			WR_RH_PORTSTAT(RH_PS_CSC);
			len = 0;
			break;
		case(RH_C_PORT_ENABLE):
			WR_RH_PORTSTAT(RH_PS_PESC);
			len = 0;
			break;
		case(RH_C_PORT_SUSPEND):
			WR_RH_PORTSTAT(RH_PS_PSSC);
			len = 0;
			break;
		case(RH_C_PORT_OVER_CURRENT):
			WR_RH_PORTSTAT(RH_PS_OCIC);
			len = 0;
			break;
		case(RH_C_PORT_RESET):
			WR_RH_PORTSTAT(RH_PS_PRSC);
			len = 0;
			break;
		default:
			goto error;
		}
		break;
	case GetHubDescriptor:
		temp = regs->roothub.a;
		data[0] = 9;			// min length;
		data[1] = 0x29;
		data[2] = temp & RH_A_NDP;
		data[3] = 0;
		if (temp & RH_A_PSM)		/* per-port power switching? */
			data[3] |= 0x1;
		if (temp & RH_A_NOCP)		/* no over current reporting? */
			data[3] |= 0x10;
		else if (temp & RH_A_OCPM)	/* per-port over current reporting? */
			data[3] |= 0x8;
		datadw[1] = 0;
		data[5] = (temp & RH_A_POTPGT) >> 24;
		temp = regs->roothub.b;
		data[7] = temp & RH_B_DR;
		if (data[2] < 7)
		{
			data[8] = 0xff;
		}
		else
		{
			data[0] += 2;
			data[8] = (temp & RH_B_DR) >> 8;
			data[10] = data[9] = 0xff;
		}
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

#ifdef VSFCFG_STANDALONE_MODULE
vsf_err_t vsfohci_modexit(struct vsf_module_t *module)
{
	vsf_bufmgr_free(module->ifs);
	module->ifs = NULL;
	return VSFERR_NONE;
}

vsf_err_t vsfohci_modinit(struct vsf_module_t *module,
								struct app_hwcfg_t const *cfg)
{
	struct vsfohci_modifs_t *ifs;
	ifs = vsf_bufmgr_malloc(sizeof(struct vsfohci_modifs_t));
	if (!ifs) return VSFERR_FAIL;
	memset(ifs, 0, sizeof(*ifs));

	ifs->drv.init_thread = vsfohci_init_thread;
	ifs->drv.fini = vsfohci_fini;
	ifs->drv.suspend = vsfohci_suspend;
	ifs->drv.resume = vsfohci_resume;
	ifs->drv.alloc_device = vsfohci_alloc_device;
	ifs->drv.free_device = vsfohci_free_device;
	ifs->drv.alloc_urb = vsfohci_alloc_urb;
	ifs->drv.free_urb = vsfohci_free_urb;
	ifs->drv.submit_urb = vsfohci_submit_urb;
	ifs->drv.relink_urb = vsfohci_relink_urb;
	ifs->drv.rh_control = vsfohci_rh_control;
	module->ifs = ifs;
	return VSFERR_NONE;
}
#else
const struct vsfusbh_hcddrv_t vsfohci_drv =
{
	.init_thread = vsfohci_init_thread,
	.fini = vsfohci_fini,
	.suspend = vsfohci_suspend,
	.resume = vsfohci_resume,
	.alloc_device = vsfohci_alloc_device,
	.free_device = vsfohci_free_device,
	.alloc_urb = vsfohci_alloc_urb,
	.free_urb = vsfohci_free_urb,
	.submit_urb = vsfohci_submit_urb,
	.relink_urb = vsfohci_relink_urb,
	.rh_control = vsfohci_rh_control,
};
#endif
