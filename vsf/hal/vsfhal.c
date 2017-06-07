/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces.c                                              *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    interfaces implementation file                            *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2010-12-05:     created(by SimonQian)                             *
 **************************************************************************/

#include "app_cfg.h"
#include "vsfhal.h"

#if VSFHAL_USBD_EN
const struct vsfhal_usbd_t vsfhal_usbd =
{
	.init = VSFHAL_USBD_INIT(__TARGET_CHIP__),
	.fini = VSFHAL_USBD_FINI(__TARGET_CHIP__),
	.poll = VSFHAL_USBD_POLL(__TARGET_CHIP__),
	.reset = VSFHAL_USBD_RESET(__TARGET_CHIP__),
	.connect = VSFHAL_USBD_CONNECT(__TARGET_CHIP__),
	.disconnect = VSFHAL_USBD_DISCONNECT(__TARGET_CHIP__),
	.set_address = VSFHAL_USBD_SET_ADDRESS(__TARGET_CHIP__),
	.get_address = VSFHAL_USBD_GET_ADDRESS(__TARGET_CHIP__),
	.suspend = VSFHAL_USBD_SUSPEND(__TARGET_CHIP__),
	.resume = VSFHAL_USBD_RESUME(__TARGET_CHIP__),
	.lowpower = VSFHAL_USBD_LOWPOWER(__TARGET_CHIP__),
	.get_frame_number = VSFHAL_USBD_GET_FRAME_NUM(__TARGET_CHIP__),
	.get_setup = VSFHAL_USBD_GET_SETUP(__TARGET_CHIP__),
	.prepare_buffer = VSFHAL_USBD_PREPARE_BUFFER(__TARGET_CHIP__),
	.ep.num_of_ep = &VSFHAL_USBD_EP_NUM(__TARGET_CHIP__),
	.ep.reset = VSFHAL_USBD_EP_RESET(__TARGET_CHIP__),
	.ep.set_type = VSFHAL_USBD_EP_SET_TYPE(__TARGET_CHIP__),
	.ep.set_IN_dbuffer = VSFHAL_USBD_EP_SET_IN_DBUFFER(__TARGET_CHIP__),
	.ep.is_IN_dbuffer = VSFHAL_USBD_EP_IS_IN_DBUFFER(__TARGET_CHIP__),
	.ep.switch_IN_buffer = VSFHAL_USBD_EP_SWITCH_IN_BUFFER(__TARGET_CHIP__),
	.ep.set_IN_epsize = VSFHAL_USBD_EP_SET_IN_EPSIZE(__TARGET_CHIP__),
	.ep.get_IN_epsize = VSFHAL_USBD_EP_GET_IN_EPSIZE(__TARGET_CHIP__),
	.ep.set_IN_stall = VSFHAL_USBD_EP_SET_IN_STALL(__TARGET_CHIP__),
	.ep.clear_IN_stall = VSFHAL_USBD_EP_CLEAR_IN_STALL(__TARGET_CHIP__),
	.ep.is_IN_stall = VSFHAL_USBD_EP_IS_IN_STALL(__TARGET_CHIP__),
	.ep.reset_IN_toggle = VSFHAL_USBD_EP_RESET_IN_TOGGLE(__TARGET_CHIP__),
	.ep.toggle_IN_toggle = VSFHAL_USBD_EP_TOGGLE_IN_TOGGLE(__TARGET_CHIP__),
	.ep.set_IN_count = VSFHAL_USBD_EP_SET_IN_COUNT(__TARGET_CHIP__),
	.ep.write_IN_buffer = VSFHAL_USBD_EP_WRITE_IN_BUFFER(__TARGET_CHIP__),
	.ep.set_OUT_dbuffer = VSFHAL_USBD_EP_SET_OUT_DBUFFER(__TARGET_CHIP__),
	.ep.is_OUT_dbuffer = VSFHAL_USBD_EP_IS_OUT_DBUFFER(__TARGET_CHIP__),
	.ep.switch_OUT_buffer = VSFHAL_USBD_EP_SWITCH_OUT_BUFFER(__TARGET_CHIP__),
	.ep.set_OUT_epsize = VSFHAL_USBD_EP_SET_OUT_EPSIZE(__TARGET_CHIP__),
	.ep.get_OUT_epsize = VSFHAL_USBD_EP_GET_OUT_EPSIZE(__TARGET_CHIP__),
	.ep.set_OUT_stall = VSFHAL_USBD_EP_SET_OUT_STALL(__TARGET_CHIP__),
	.ep.clear_OUT_stall = VSFHAL_USBD_EP_CLEAR_OUT_STALL(__TARGET_CHIP__),
	.ep.is_OUT_stall = VSFHAL_USBD_EP_IS_OUT_STALL(__TARGET_CHIP__),
	.ep.reset_OUT_toggle = VSFHAL_USBD_EP_RESET_OUT_TOGGLE(__TARGET_CHIP__),
	.ep.toggle_OUT_toggle = VSFHAL_USBD_EP_TOGGLE_OUT_TOGGLE(__TARGET_CHIP__),
	.ep.get_OUT_count = VSFHAL_USBD_EP_GET_OUT_COUNT(__TARGET_CHIP__),
	.ep.read_OUT_buffer = VSFHAL_USBD_EP_READ_OUT_BUFFER(__TARGET_CHIP__),
	.ep.enable_OUT = VSFHAL_USBD_EP_ENABLE_OUT(__TARGET_CHIP__),
	.callback = &VSFHAL_USBD_CALLBACK(__TARGET_CHIP__),
};
#endif

