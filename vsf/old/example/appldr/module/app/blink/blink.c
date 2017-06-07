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
#include "../../vsfos/vsfos.h"

struct app_blink_t
{
	struct interface_gpio_pin_t pin;
	uint32_t interval;
};

static vsf_err_t app_blink(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct app_blink_t *blink = (struct app_blink_t *)param->priv;

	vsfsm_pt_begin(pt);

	if (param->argc != 4)
	{
		vsfshell_printf(outpt, "format: %s PORT PIN INTERVAL"VSFSHELL_LINEEND,
							param->argv[0]);
		goto end;
	}

	param->priv = vsf_bufmgr_malloc(sizeof(struct app_blink_t));
	if (NULL == param->priv)
	{
		vsfshell_printf(outpt, "not enough resources"VSFSHELL_LINEEND);
		goto end;
	}
	blink = (struct app_blink_t *)param->priv;
	blink->pin.port = strtoul(param->argv[1], NULL, 0);
	blink->pin.pin = strtoul(param->argv[2], NULL, 0);
	blink->interval = strtoul(param->argv[3], NULL, 0);

	vsfshell_printf(outpt, "blinking on GPIO%d.%d"VSFSHELL_LINEEND,
					blink->pin.port, blink->pin.pin);

	// switch to background thread
	vsfshell_handler_release_io(param);
	vsfhal_gpio_init(blink->pin.port);
	vsfhal_gpio_config_pin(blink->pin.port, blink->pin.pin, GPIO_OUTPP);
	while (1)
	{
		vsfhal_gpio_clear(blink->pin.port, 1 << blink->pin.pin);
		vsfsm_pt_delay(pt, blink->interval);
		vsfhal_gpio_set(blink->pin.port, 1 << blink->pin.pin);
		vsfsm_pt_delay(pt, blink->interval);
	}

end:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t app_blink_modexit(struct vsf_module_t *module)
{
	vsf_bufmgr_free(module->ifs);
	module->ifs = NULL;
	return VSFERR_NONE;
}

vsf_err_t app_blink_modinit(struct vsf_module_t *module,
								struct app_hwcfg_t const *cfg)
{
	struct vsfshell_handler_t *handlers;
	handlers = vsf_bufmgr_malloc(sizeof(struct vsfshell_handler_t));
	if (!handlers) return VSFERR_FAIL;
	memset(handlers, 0, sizeof(*handlers));

	handlers[0] = (struct vsfshell_handler_t){"blink", app_blink};
	vsfshell_register_handlers(&vsfos->shell, handlers, 1);
	module->ifs = handlers;
	return VSFERR_NONE;
}
