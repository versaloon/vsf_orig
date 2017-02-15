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

static void vsfsm_enter_critical_internal(void)
{
	vsf_enter_critical();
}
static void vsfsm_leave_critical_internal(void)
{
	vsf_leave_critical();
}

#ifdef VSFCFG_MODULE
static struct vsf_module_t *modulelist = NULL;

void vsf_module_register(struct vsf_module_t *module)
{
	module->code_buff = NULL;
	module->next = modulelist;
	modulelist = module;
}

void vsf_module_unregister(struct vsf_module_t *module)
{
	if (module == modulelist)
	{
		modulelist = modulelist->next;
	}
	else
	{
		struct vsf_module_t *moduletmp = modulelist;

		while (moduletmp->next != NULL)
		{
			if (moduletmp->next == module)
			{
				moduletmp->next = module->next;
			}
			moduletmp = moduletmp->next;
		}
	}
}

struct vsf_module_t* vsf_module_get(char *name)
{
	struct vsf_module_t *module = modulelist;

	if (!name)
		return module;

	while ((module != NULL) && strcmp(module->flash->name, name))
		module = module->next;
	return module;
}

void vsf_module_unload(char *name)
{
	struct vsf_module_t *module = vsf_module_get(name);
	vsf_err_t (*mod_exit)(struct vsf_module_t *);

	if ((module != NULL) && (module->code_buff != NULL))
	{
		if (module->flash->exit)
		{
			mod_exit = (vsf_err_t (*)(struct vsf_module_t *))
							(module->code_buff + module->flash->exit);
			mod_exit(module);
		}

#ifdef VSFCFG_MODULE_ALLOC_RAM
		if (module->flash->size > 0)
		{
			vsf_bufmgr_free(module->code_buff);
		}
#endif
		module->code_buff = NULL;
	}
}

void* vsf_module_load(char *name, bool dead)
{
	struct vsf_module_t *module = vsf_module_get(name);
	vsf_err_t (*mod_entry)(struct vsf_module_t *, struct app_hwcfg_t const *);

	if ((module != NULL) && module->flash->entry)
	{
		if (module->code_buff != NULL)
		{
			goto succeed;
		}
#ifdef VSFCFG_MODULE_ALLOC_RAM
		if (module->flash->size > 0)
		{
			module->code_buff = vsf_bufmgr_malloc(module->flash->size);
			if (NULL == module->code_buff)
			{
				goto fail;
			}
			memcpy(module->code_buff, module->flash, module->flash->size);
		}
		else
		{
			module->code_buff = (uint8_t *)module->flash;
		}
#else
		module->code_buff = (uint8_t *)module->flash;
#endif
		mod_entry =
			(vsf_err_t (*)(struct vsf_module_t *, struct app_hwcfg_t const *))
						(module->code_buff + module->flash->entry);

		if (mod_entry(module, &app_hwcfg))
		{
#ifdef VSFCFG_MODULE_ALLOC_RAM
			if (module->flash->size > 0)
			{
				vsf_bufmgr_free(module->code_buff);
			}
#endif
			module->code_buff = NULL;
			goto fail;
		}
succeed:
		return module->ifs;
	}
fail:
	if (dead) while (1);
	return NULL;
}
#endif		// VSFCFG_MODULE

void* vsfhal_getif(char *ifname)
{
	if (!strcmp(ifname, "core")) return (void *)&core_interfaces.core;
	else if (!strcmp(ifname, "tickclk")) return (void *)&core_interfaces.tickclk;
#if IFS_FLASH_EN
	else if (!strcmp(ifname, "flash")) return (void *)&core_interfaces.flash;
#endif
#if IFS_GPIO_EN
	else if (!strcmp(ifname, "gpio")) return (void *)&core_interfaces.gpio;
#endif
#if IFS_EINT_EN
	else if (!strcmp(ifname, "eint")) return (void *)&core_interfaces.eint;
#endif
#if IFS_USART_EN
	else if (!strcmp(ifname, "usart")) return (void *)&core_interfaces.usart;
#endif
#if IFS_SPI_EN
	else if (!strcmp(ifname, "spi")) return (void *)&core_interfaces.spi;
#endif
#if IFS_ADC_EN
	else if (!strcmp(ifname, "adc")) return (void *)&core_interfaces.adc;
#endif
#if IFS_IIC_EN
	else if (!strcmp(ifname, "i2c")) return (void *)&core_interfaces.i2c;
#endif
#if IFS_USBD_EN
	else if (!strcmp(ifname, "usbd")) return (void *)&core_interfaces.usbd;
#endif
#if IFS_HCD_EN
	else if (!strcmp(ifname, "hcd")) return (void *)&core_interfaces.hcd;
#endif
#if IFS_PWM_EN
	else if (!strcmp(ifname, "pwm")) return (void *)&core_interfaces.pwm;
#endif
#if IFS_EBI_EN
	else if (!strcmp(ifname, "ebi")) return (void *)&core_interfaces.ebi;
#endif
#if IFS_SDIO_EN
	else if (!strcmp(ifname, "sdio")) return (void *)&core_interfaces.sdio;
#endif
	else return NULL;
}

// reserve 512 bytes for vector table
ROOTFUNC const struct vsf_t vsf @ VSFCFG_API_ADDR =
{
	.ver = VSF_API_VERSION,
	.ifs = &core_interfaces,
	.getif = vsfhal_getif,

	.libc.stdlib.abs = abs,
	.libc.stdlib.labs = labs,
	.libc.stdlib.llabs = llabs,
	.libc.stdlib.div = div,
	.libc.stdlib.ldiv = ldiv,
	.libc.stdlib.lldiv = lldiv,
	.libc.stdlib.strtod = strtod,
	.libc.stdlib.strtof = strtof,
	.libc.stdlib.strtold = strtold,
	.libc.stdlib.strtol = strtol,
	.libc.stdlib.strtoul = strtoul,
	.libc.stdlib.strtoll = strtoll,
	.libc.stdlib.strtoull = strtoull,
	.libc.stdlib.atof = atof,
	.libc.stdlib.atoi = atoi,
	.libc.stdlib.atol = atol,
	.libc.stdlib.atoll = atoll,
	.libc.stdio.sprintf = sprintf,
	.libc.stdio.snprintf = snprintf,
	.libc.stdio.vsprintf = vsprintf,
	.libc.stdio.vsnprintf = vsnprintf,
	.libc.string.memcmp = memcmp,
	.libc.string.memcpy = memcpy,
	.libc.string.memmove = memmove,
	.libc.string.memset = memset,
	.libc.string.strcat = strcat,
	.libc.string.strcmp = strcmp,
	.libc.string.strcpy = strcpy,
	.libc.string.strcspn = strcspn,
	.libc.string.strlen = strlen,
	.libc.string.strncat = strncat,
	.libc.string.strncmp = strncmp,
	.libc.string.strncpy = strncpy,
	.libc.string.strspn = strspn,
	.libc.string.strtok = strtok,
	.libc.string.strxfrm = strxfrm,
	.libc.string.strcasecmp = strcasecmp,
	.libc.string.strncasecmp = strncasecmp,
	.libc.string.strtok_r = strtok_r,
	.libc.string.strnlen = strnlen,
	.libc.string.memchr = memchr,
	.libc.string.strchr = strchr,
	.libc.string.strpbrk = strpbrk,
	.libc.string.strrchr = strrchr,
	.libc.string.strstr = strstr,
	.libc.ctype.isdigit = isdigit,
	.libc.ctype.isspace = isspace,
	.libc.ctype.isalpha = isalpha,
	.libc.ctype.isalnum = isalnum,
	.libc.ctype.isprint = isprint,
	.libc.ctype.isupper = isupper,
	.libc.ctype.islower = islower,
	.libc.ctype.isxdigit = isxdigit,
	.libc.ctype.isblank = isblank,
	.libc.ctype.isgraph = isgraph,
	.libc.ctype.iscntrl = iscntrl,
	.libc.ctype.ispunct = ispunct,
	.libc.ctype.tolower = tolower,
	.libc.ctype.toupper = toupper,

	.framework.evtq_init = vsfsm_evtq_init,
	.framework.evtq_set = vsfsm_evtq_set,
	.framework.poll = vsfsm_poll,
	.framework.get_event_pending = vsfsm_get_event_pending,
	.framework.sm_init = vsfsm_init,
	.framework.sm_fini = vsfsm_fini,
#if VSFSM_CFG_PT_EN
	.framework.pt_init = vsfsm_pt_init,
#endif
	.framework.post_evt = vsfsm_post_evt,
	.framework.post_evt_pending = vsfsm_post_evt_pending,
	.framework.enter_critical = vsfsm_enter_critical_internal,
	.framework.leave_critical = vsfsm_leave_critical_internal,
#if VSFSM_CFG_SYNC_EN
	.framework.sync.init = vsfsm_sync_init,
	.framework.sync.cancel = vsfsm_sync_cancel,
	.framework.sync.increase = vsfsm_sync_increase,
	.framework.sync.decrease = vsfsm_sync_decrease,
#endif
	.framework.timer.init = vsftimer_init,
	.framework.timer.create = vsftimer_create,
	.framework.timer.free = vsftimer_free,
	.framework.timer.enqueue = vsftimer_enqueue,
	.framework.timer.dequeue = vsftimer_dequeue,
	.framework.timer.callback = vsftimer_callback_int,
#ifdef VSFCFG_MODULE
	.framework.module.get = vsf_module_get,
	.framework.module.reg = vsf_module_register,
	.framework.module.unreg = vsf_module_unregister,
	.framework.module.load = vsf_module_load,
	.framework.module.unload = vsf_module_unload,
#endif

#ifdef VSFCFG_BUFFER
	.component.buffer.queue.init = vsfq_init,
	.component.buffer.queue.append = vsfq_append,
	.component.buffer.queue.remove = vsfq_remove,
	.component.buffer.queue.enqueue = vsfq_enqueue,
	.component.buffer.queue.dequeue = vsfq_dequeue,
	.component.buffer.fifo.init = vsf_fifo_init,
	.component.buffer.fifo.push8 = vsf_fifo_push8,
	.component.buffer.fifo.pop8 = vsf_fifo_pop8,
	.component.buffer.fifo.push = vsf_fifo_push,
	.component.buffer.fifo.pop = vsf_fifo_pop,
	.component.buffer.fifo.get_data_length = vsf_fifo_get_data_length,
	.component.buffer.fifo.get_avail_length = vsf_fifo_get_avail_length,
	.component.buffer.fifo.get_rbuf = vsf_fifo_get_rbuf,
	.component.buffer.fifo.get_wbuf = vsf_fifo_get_wbuf,
	.component.buffer.multibuf.init = vsf_multibuf_init,
	.component.buffer.multibuf.get_empty = vsf_multibuf_get_empty,
	.component.buffer.multibuf.push = vsf_multibuf_push,
	.component.buffer.multibuf.get_payload = vsf_multibuf_get_payload,
	.component.buffer.multibuf.pop = vsf_multibuf_pop,
	.component.buffer.bufmgr.malloc_aligned_do = vsf_bufmgr_malloc_aligned_do,
	.component.buffer.bufmgr.free_do = vsf_bufmgr_free_do,
	.component.buffer.pool.init = vsfpool_init,
	.component.buffer.pool.alloc = vsfpool_alloc,
	.component.buffer.pool.free = vsfpool_free,
#endif

	.tool.bittool.bit_reverse_u8 = BIT_REVERSE_U8,
	.tool.bittool.bit_reverse_u16 = BIT_REVERSE_U16,
	.tool.bittool.bit_reverse_u32 = BIT_REVERSE_U32,
	.tool.bittool.bit_reverse_u64 = BIT_REVERSE_U64,
	.tool.bittool.get_u16_msb = GET_U16_MSBFIRST,
	.tool.bittool.get_u24_msb = GET_U24_MSBFIRST,
	.tool.bittool.get_u32_msb = GET_U32_MSBFIRST,
	.tool.bittool.get_u64_msb = GET_U64_MSBFIRST,
	.tool.bittool.get_u16_lsb = GET_U16_LSBFIRST,
	.tool.bittool.get_u24_lsb = GET_U24_LSBFIRST,
	.tool.bittool.get_u32_lsb = GET_U32_LSBFIRST,
	.tool.bittool.get_u64_lsb = GET_U64_LSBFIRST,
	.tool.bittool.set_u16_msb = SET_U16_MSBFIRST,
	.tool.bittool.set_u24_msb = SET_U24_MSBFIRST,
	.tool.bittool.set_u32_msb = SET_U32_MSBFIRST,
	.tool.bittool.set_u64_msb = SET_U64_MSBFIRST,
	.tool.bittool.set_u16_lsb = SET_U16_LSBFIRST,
	.tool.bittool.set_u24_lsb = SET_U24_LSBFIRST,
	.tool.bittool.set_u32_lsb = SET_U32_LSBFIRST,
	.tool.bittool.set_u64_lsb = SET_U64_LSBFIRST,
	.tool.bittool.swap_u16 = SWAP_U16,
	.tool.bittool.swap_u24 = SWAP_U24,
	.tool.bittool.swap_u32 = SWAP_U32,
	.tool.bittool.swap_u64 = SWAP_U64,
	.tool.bittool.mskarr.set = mskarr_set,
	.tool.bittool.mskarr.clr = mskarr_clr,
	.tool.bittool.mskarr.ffz = mskarr_ffz,
	.tool.bittool.msb = msb,
	.tool.bittool.ffz = ffz,
};
