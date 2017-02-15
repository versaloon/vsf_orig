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
#include "app_hw_cfg.h"

ROOTFUNC vsf_err_t module_exit(struct vsf_module_t *module)
{
	return MODULE_EXIT(module);
}

ROOTFUNC vsf_err_t module_entry(struct vsf_module_t *module,
							struct app_hwcfg_t const *hwcfg)
{
	// check api version
	if (api_ver != VSF_API_VERSION)
	{
		return VSFERR_NOT_SUPPORT;
	}
	return MODULE_INIT(module, hwcfg);
}
