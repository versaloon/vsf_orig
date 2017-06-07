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
#include "vsfos.h"

#define VSFOSCFG_HANDLER_NUM				32

struct vsfos_ctx_t
{
	struct vsfile_t *curfile;
	uint8_t user_buff[64];
};

static vsf_err_t vsfos_busybox_help(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_help_t
	{
		struct vsfshell_handler_t *handler;
	} *lparam = (struct vsfos_busybox_help_t *)ctx->user_buff;

	vsfsm_pt_begin(pt);

	lparam->handler = param->shell->handlers;

	while ((lparam->handler != NULL) && (lparam->handler->name != NULL))
	{
		vsfshell_printf(outpt, "%s"VSFSHELL_LINEEND, lparam->handler->name);
		lparam->handler = lparam->handler->next;
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_uname(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "%s"VSFSHELL_LINEEND, vsfos->hwcfg->board);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_free(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_top(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

// module handlers
static vsf_err_t vsfos_busybox_lsmod(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_lsmod_t
	{
		struct vsf_module_t *module;
		uint32_t flashbase;
	} *lparam = (struct vsfos_busybox_lsmod_t *)ctx->user_buff;

	vsfsm_pt_begin(pt);

	lparam->flashbase = vsfhal_flash_baseaddr(0);
	lparam->module = vsf_module_get(NULL);
	while (lparam->module != NULL)
	{
		vsfshell_printf(outpt, "%08X(%d): %s"VSFSHELL_LINEEND,
				(uint32_t)lparam->module->flash - lparam->flashbase,
				lparam->module->flash->size, lparam->module->flash->name);
		lparam->module = lparam->module->next;
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_insmod(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);

	if (param->argc != 2)
	{
		vsfshell_printf(outpt, "format: %s MODULE"VSFSHELL_LINEEND,
							param->argv[0]);
		goto end;
	}

	if (!vsf_module_load(param->argv[1], false))
	{
		vsfshell_printf(outpt, "fail to load %s MODULE"VSFSHELL_LINEEND,
							param->argv[1]);
		goto end;
	}

end:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

#define VSFOS_REPO				"http://versaloon.github.io/modules/CM0/"
static vsf_err_t vsfos_busybox_repo(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_repo_t
	{
		char *path;
		struct vsf_buffer_t buf;
		uint8_t *buffer_list[1];
		struct vsf_malstream_t *malstream;
		struct vsfip_httpc_param_t *httpc;
		struct vsf_module_t *module;
		struct vsfsm_pt_t local_pt;
		uint32_t offset;
	} *lparam = (struct vsfos_busybox_repo_t *)ctx->user_buff;
	vsf_err_t err;
	struct vsf_multibuf_t *multibuf;

	vsfsm_pt_begin(pt);

	if (param->argc != 3)
	{
		vsfshell_printf(outpt, "format: %s MODULE OFFSET"VSFSHELL_LINEEND,
							param->argv[0]);
		goto end;
	}

	memset(lparam, 0, sizeof(*lparam));
	lparam->offset = strtoul(param->argv[2], NULL, 0);
	lparam->local_pt.sm = pt->sm;
	lparam->path = vsf_bufmgr_malloc(128);
	lparam->httpc = vsf_bufmgr_malloc(sizeof(*lparam->httpc));
	if ((NULL == lparam->path) || (NULL == lparam->httpc))
	{
		vsfshell_printf(outpt, "fail to allocate httpc"VSFSHELL_LINEEND);
		goto end;
	}
	strcpy(lparam->path, VSFOS_REPO);
	strncat(lparam->path, param->argv[1], 127);

	// Step 1: try get xml configuration file
/*	strncat(lparam->path, ".xml", 127);
	lparam->httpc->op = &vsfip_httpc_op_buffer;
	lparam->local_pt.user_data = lparam->httpc;
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_httpc_get(&lparam->local_pt, evt, lparam->path, NULL);
	if (err > 0) return err; else if ((err < 0) || !lparam->httpc->resp_length)
	{
		vsfshell_printf(outpt, "fail to get size of %s"VSFSHELL_LINEEND,
							lparam->path);
		goto end;
	}

	lparam->buf.size = lparam->httpc->resp_length;
	lparam->buf.buffer = vsf_bufmgr_malloc(lparam->buf.size);
	if (NULL == lparam->buf.buffer)
	{
		goto malloc_fail_end;
	}

	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_httpc_get(&lparam->local_pt, evt, lparam->path, &lparam->buf);
	if (err > 0) return err; else if (err < 0)
	{
		vsfshell_printf(outpt, "fail to download %s"VSFSHELL_LINEEND,
							lparam->path);
		goto end;
	}

	// Step 2: check dependency, version and size
	// TODO:
	if (lparam->buf.buffer != NULL)
	{
		vsf_bufmgr_free(lparam->buf.buffer);
		lparam->buf.buffer = NULL;
	}

	// Step 3: download and install
	lparam->path[strlen(lparam->path) - 4] = '\0';
*/	lparam->httpc->op = &vsfip_httpc_op_stream;
	lparam->local_pt.user_data = lparam->httpc;
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_httpc_get(&lparam->local_pt, evt, lparam->path, NULL);
	if (err > 0) return err; else if ((err < 0) || !lparam->httpc->resp_length)
	{
		vsfshell_printf(outpt, "fail to get size of %s"VSFSHELL_LINEEND,
							lparam->path);
		goto end;
	}

	lparam->malstream = vsf_bufmgr_malloc(sizeof(struct vsf_malstream_t));
	if (NULL == lparam->malstream)
		goto malloc_fail_end;
	lparam->malstream->mal = vsf_bufmgr_malloc(sizeof(struct vsfmal_t));
	if (NULL == lparam->malstream->mal)
		goto malloc_fail_end;
	lparam->malstream->mbufstream = vsf_bufmgr_malloc(sizeof(struct vsf_mbufstream_t));
	if (NULL == lparam->malstream->mal)
		goto malloc_fail_end;

	{
		uint32_t pagesize;
		vsfhal_flash_capacity(0, &pagesize, NULL);
		lparam->buffer_list[0] = vsf_bufmgr_malloc(pagesize);
		if (NULL == lparam->buffer_list[0])
			goto malloc_fail_end;

		multibuf = &lparam->malstream->mbufstream->mem.multibuf;
		multibuf->buffer_list = lparam->buffer_list;
		multibuf->size = pagesize;
		multibuf->count = dimof(lparam->buffer_list);
	}

	vsf_malstream_init(lparam->malstream);
	vsf_malstream_write(lparam->malstream, 0, 0);

	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_httpc_get(&lparam->local_pt, evt, lparam->path,
							lparam->malstream->mbufstream);
	if (err > 0) return err; else if (err < 0)
	{
		vsfshell_printf(outpt, "fail to download %s"VSFSHELL_LINEEND,
							lparam->path);
		goto end;
	}

malloc_fail_end:
	vsfshell_printf(outpt, "fail to get allocate buffer"VSFSHELL_LINEEND);
end:
	if (lparam->buf.buffer != NULL)
		vsf_bufmgr_free(lparam->buf.buffer);
	if (lparam->malstream != NULL)
	{
		if (lparam->malstream->mal != NULL)
			vsf_bufmgr_free(lparam->malstream->mal);
		if (lparam->malstream->mbufstream != NULL)
			vsf_bufmgr_free(lparam->malstream->mbufstream);
		vsf_bufmgr_free(lparam->malstream);
	}
	if (lparam->buffer_list[0] != NULL)
		vsf_bufmgr_free(lparam->buffer_list[0]);
	if (lparam->path != NULL)
		vsf_bufmgr_free(lparam->path);
	if (lparam->httpc != NULL)
		vsf_bufmgr_free(lparam->httpc);

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

// fs handlers
static char* vsfos_busybox_filetype(struct vsfile_t *file)
{
	if (file->attr & VSFILE_ATTR_VOLUMID)
		return "VOL";
	else if (file->attr & VSFILE_ATTR_DIRECTORY)
		return "DIR";
	else
		return "FIL";
}
static vsf_err_t vsfos_busybox_ls(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_ls_t
	{
		struct vsfile_t *file;
		struct vsfsm_pt_t local_pt;
	} *lparam = (struct vsfos_busybox_ls_t *)ctx->user_buff;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	lparam->local_pt.sm = pt->sm;
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_findfirst(&lparam->local_pt, evt, ctx->curfile, &lparam->file);
	if (err > 0) return err; else if (err < 0) goto end;

	while (lparam->file)
	{
		vsfshell_printf(outpt, "%s(%lld):%s"VSFSHELL_LINEEND,
				vsfos_busybox_filetype(lparam->file),
				lparam->file->size, lparam->file->name);

		// close file
		lparam->local_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfile_close(&lparam->local_pt, evt, lparam->file);
		if (err > 0) return err; else if (err < 0) goto srch_end;

		lparam->local_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfile_findnext(&lparam->local_pt, evt, ctx->curfile, &lparam->file);
		if (err > 0) return err; else if (err < 0) goto srch_end;
	}

srch_end:
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_findend(&lparam->local_pt, evt, ctx->curfile);
	if (err > 0) return err; else if (err < 0) goto end;
end:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_cd(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_cd_t
	{
		struct vsfile_t *file;
		struct vsfsm_pt_t local_pt;
	} *lparam = (struct vsfos_busybox_cd_t *)ctx->user_buff;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	if (param->argc != 2)
	{
		vsfshell_printf(outpt, "format: %s PATH"VSFSHELL_LINEEND,
							param->argv[0]);
		goto end;
	}
	lparam->local_pt.sm = pt->sm;

	if (!strcmp(param->argv[1], "."))
	{
		goto end;
	}
	else if (!strcmp(param->argv[1], ".."))
	{
		if (ctx->curfile == (struct vsfile_t *)&vsfile.rootfs)
		{
			goto end;
		}

		vsfshell_printf(outpt, "not support"VSFSHELL_LINEEND);
		goto end;
	}
	else if (!strcmp(param->argv[1], "/"))
	{
		lparam->file = (struct vsfile_t *)&vsfile.rootfs;
	}
	else
	{
		lparam->local_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfile_getfile(&lparam->local_pt, evt, ctx->curfile,
								param->argv[1], &lparam->file);
		if (err > 0) return err; else if (err < 0)
		{
			vsfshell_printf(outpt, "directory not found: %s"VSFSHELL_LINEEND,
								param->argv[1]);
			goto end;
		}
	}

	if (!(lparam->file->attr & VSFILE_ATTR_DIRECTORY))
	{
		vsfshell_printf(outpt, "%s is not a directory"VSFSHELL_LINEEND,
								param->argv[1]);
		goto close_end;
	}

	ctx->curfile = (struct vsfile_t *)\
					((uint32_t)lparam->file - (uint32_t)ctx->curfile);
	lparam->file = (struct vsfile_t *)\
					((uint32_t)lparam->file - (uint32_t)ctx->curfile);
	ctx->curfile = (struct vsfile_t *)\
					((uint32_t)lparam->file + (uint32_t)ctx->curfile);

close_end:
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_close(&lparam->local_pt, evt, lparam->file);
	if (err > 0) return err; else if (err < 0) goto end;
end:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_pwd(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "%s"VSFSHELL_LINEEND, ctx->curfile->name);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_mkdir(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_rmdir(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_rm(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_mv(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_cp(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_cat(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_cat_t
	{
		struct vsfile_t *file;
		uint8_t *buff;
		struct vsfsm_pt_t local_pt;
		uint32_t rsize;
		char *line;
		int pos;
		char tmp;
		bool enter;
	} *lparam = (struct vsfos_busybox_cat_t *)ctx->user_buff;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	if (param->argc != 2)
	{
		vsfshell_printf(outpt, "format: %s TEXT_FILE"VSFSHELL_LINEEND,
							param->argv[0]);
		goto end;
	}

	lparam->local_pt.sm = pt->sm;
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_getfile(&lparam->local_pt, evt, ctx->curfile, param->argv[1],
							&lparam->file);
	if (err > 0) return err; else if (err < 0)
	{
		vsfshell_printf(outpt, "file not found: %s"VSFSHELL_LINEEND,
								param->argv[1]);
		goto end;
	}

	if (!(lparam->file->attr & VSFILE_ATTR_ARCHIVE))
	{
		vsfshell_printf(outpt, "%s is not a file"VSFSHELL_LINEEND,
								param->argv[1]);
		goto close_end;
	}

	lparam->buff = vsf_bufmgr_malloc(lparam->file->size + 1);
	if (!lparam->buff)
	{
		vsfshell_printf(outpt, "can not allocate buffer"VSFSHELL_LINEEND);
		goto close_end;
	}

	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_read(&lparam->local_pt, evt, lparam->file, 0,
				lparam->file->size, lparam->buff, &lparam->rsize);
	if (err > 0) return err; else if (err < 0) goto free_end;

	// output line by line, because vsfshell_printf not use big buffer
	lparam->line = (char *)lparam->buff;

	while (lparam->rsize > 0)
	{
		lparam->enter = false;
		for (lparam->pos = 0; lparam->pos < lparam->rsize;)
		{
			if (lparam->line[lparam->pos++] == '\r')
			{
				lparam->tmp = lparam->line[lparam->pos];
				lparam->enter = true;
				break;
			}
		}
		lparam->line[lparam->pos] = '\0';

		vsfshell_printf(outpt, "%s%s", lparam->line,
				lparam->enter ? "" : VSFSHELL_LINEEND);
		lparam->rsize -= strlen(lparam->line);
		lparam->line += strlen(lparam->line);
		lparam->line[0] = lparam->tmp;
	}

free_end:
	vsf_bufmgr_free(lparam->buff);
close_end:
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_close(&lparam->local_pt, evt, lparam->file);
	if (err > 0) return err; else if (err < 0) goto end;
end:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

// net handlers
static vsf_err_t vsfos_busybox_ipconfig(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_ipconfig_t
	{
		int i;
		struct vsfip_netif_t *netif;
	} *lparam = (struct vsfos_busybox_ipconfig_t *)ctx->user_buff;

	vsfsm_pt_begin(pt);

	lparam->i = 0;
	lparam->netif = vsfip.netif_list;
	while (lparam->netif != NULL)
	{
		vsfshell_printf(outpt,
			"netif%d:"VSFSHELL_LINEEND\
				"\tmac: %02X:%02X:%02X:%02X:%02X:%02X"VSFSHELL_LINEEND\
				"\tipaddr: %d.%d.%d.%d"VSFSHELL_LINEEND\
				"\tnetmask: %d.%d.%d.%d"VSFSHELL_LINEEND\
				"\tgateway: %d.%d.%d.%d"VSFSHELL_LINEEND,
			lparam->i,
			lparam->netif->macaddr.addr.s_addr_buf[0],
			lparam->netif->macaddr.addr.s_addr_buf[1],
			lparam->netif->macaddr.addr.s_addr_buf[2],
			lparam->netif->macaddr.addr.s_addr_buf[3],
			lparam->netif->macaddr.addr.s_addr_buf[4],
			lparam->netif->macaddr.addr.s_addr_buf[5],
			lparam->netif->ipaddr.addr.s_addr_buf[0],
			lparam->netif->ipaddr.addr.s_addr_buf[1],
			lparam->netif->ipaddr.addr.s_addr_buf[2],
			lparam->netif->ipaddr.addr.s_addr_buf[3],
			lparam->netif->netmask.addr.s_addr_buf[0],
			lparam->netif->netmask.addr.s_addr_buf[1],
			lparam->netif->netmask.addr.s_addr_buf[2],
			lparam->netif->netmask.addr.s_addr_buf[3],
			lparam->netif->gateway.addr.s_addr_buf[0],
			lparam->netif->gateway.addr.s_addr_buf[1],
			lparam->netif->gateway.addr.s_addr_buf[2],
			lparam->netif->gateway.addr.s_addr_buf[3]);
		lparam->i++;
		lparam->netif = lparam->netif->next;
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_arp(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_arp_t
	{
		int i;
		struct vsfip_netif_t *netif;
		struct vsfip_ipaddr_t *ip;
		struct vsfip_macaddr_t *mac;
	} *lparam = (struct vsfos_busybox_arp_t *)ctx->user_buff;

	vsfsm_pt_begin(pt);

	lparam->netif = vsfip.netif_list;
	while (lparam->netif != NULL)
	{
		vsfshell_printf(outpt, "%d.%d.%d.%d:"VSFSHELL_LINEEND,
					lparam->netif->ipaddr.addr.s_addr_buf[0],
					lparam->netif->ipaddr.addr.s_addr_buf[1],
					lparam->netif->ipaddr.addr.s_addr_buf[2],
					lparam->netif->ipaddr.addr.s_addr_buf[3]);
	
		for (lparam->i = 0; lparam->i < dimof(lparam->netif->arp_cache);
				lparam->i++)
		{
			if (lparam->netif->arp_cache[lparam->i].time != 0)
			{
				lparam->ip = &lparam->netif->arp_cache[lparam->i].assoc.ip;
				lparam->mac = &lparam->netif->arp_cache[lparam->i].assoc.mac;
				vsfshell_printf(outpt,
					"\t%d.%d.%d.%d"
					"-%02X:%02X:%02X:%02X:%02X:%02X"VSFSHELL_LINEEND,
					lparam->ip->addr.s_addr_buf[0],
					lparam->ip->addr.s_addr_buf[1],
					lparam->ip->addr.s_addr_buf[2],
					lparam->ip->addr.s_addr_buf[3],
					lparam->mac->addr.s_addr_buf[0],
					lparam->mac->addr.s_addr_buf[1],
					lparam->mac->addr.s_addr_buf[2],
					lparam->mac->addr.s_addr_buf[3],
					lparam->mac->addr.s_addr_buf[4],
					lparam->mac->addr.s_addr_buf[5]);
			}
		}

		lparam->netif = lparam->netif->next;
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_ping(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_httpd(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_httpd_t
	{
		struct vsfip_httpd_t *httpd;
		uint16_t port;
		struct vsfile_t *root;
		struct vsfsm_pt_t local_pt;
	} *lparam = (struct vsfos_busybox_httpd_t *)ctx->user_buff;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	if (param->argc != 5)
	{
		vsfshell_printf(outpt,
						"format: %s SERVICE_NUM PORT ROOT HOME"VSFSHELL_LINEEND,
							param->argv[0]);
		goto end;
	}

	memset(lparam, 0, sizeof(*lparam));
	lparam->httpd = vsf_bufmgr_malloc(sizeof(struct vsfip_httpd_t));
	if (!lparam->httpd)
	{
		vsfshell_printf(outpt, "fail to allocate httpd"VSFSHELL_LINEEND);
		goto end;
	}
	memset(lparam->httpd, 0, sizeof(struct vsfip_httpd_t));
	lparam->httpd->service_num = atoi(param->argv[1]);
	if (!lparam->httpd->service_num)
	{
		vsfshell_printf(outpt, "service number is 0"VSFSHELL_LINEEND);
		goto free_end;
	}
	lparam->httpd->service = vsf_bufmgr_malloc(
			lparam->httpd->service_num * sizeof(struct vsfip_httpd_service_t));
	if (!lparam->httpd->service)
	{
		vsfshell_printf(outpt, "fail to allocate %d service(s)"VSFSHELL_LINEEND,
							lparam->httpd->service_num);
		goto free_end;
	}
	lparam->httpd->homepage = vsf_bufmgr_malloc(strlen(param->argv[4]) + 1);
	if (!lparam->httpd->homepage)
	{
		vsfshell_printf(outpt, "fail to allocate homepage: %s"VSFSHELL_LINEEND,
							param->argv[4]);
		goto free_end;
	}
	strcpy(lparam->httpd->homepage, param->argv[4]);

	lparam->port = atoi(param->argv[2]);
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfile_getfile(&lparam->local_pt, evt, ctx->curfile, param->argv[3],
							&lparam->httpd->root);
	if (err > 0) return err; else if (err < 0)
	{
		vsfshell_printf(outpt, "fail to open root %s"VSFSHELL_LINEEND,
							param->argv[3]);
		goto free_end;
	}

	vsfshell_printf(outpt, "start httpd on port %d, root: %s"VSFSHELL_LINEEND,
							lparam->port, param->argv[3]);
	vsfip_httpd_start(lparam->httpd, lparam->port);
	goto end;

free_end:
	if (lparam->httpd->root != NULL)
	{
		lparam->local_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfile_close(&lparam->local_pt, evt, lparam->httpd->root);
		if (err > 0) return err;
	}
	if (lparam->httpd != NULL)
	{
		if (lparam->httpd->service != NULL)
			vsf_bufmgr_free(lparam->httpd->service);
		if (lparam->httpd->homepage != NULL)
			vsf_bufmgr_free(lparam->httpd->homepage);
		vsf_bufmgr_free(lparam->httpd);
	}
end:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfos_busybox_dns(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfos_ctx_t *ctx = (struct vsfos_ctx_t *)param->context;
	struct vsfos_busybox_dns_t
	{
		struct vsfip_ipaddr_t ip;
		struct vsfsm_pt_t local_pt;
		struct vsfip_ipaddr_t *dns_server;
	} *lparam = (struct vsfos_busybox_dns_t *)ctx->user_buff;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	if ((param->argc > 3) || (param->argc < 2))
	{
		vsfshell_printf(outpt, "format: %s DOMAIN [SERVER]"VSFSHELL_LINEEND,
							param->argv[0]);
		goto end;
	}

	if (param->argc == 3)
	{
		err = vsfip_ip4_pton(&lparam->ip, param->argv[2]);
		if (err < 0)
		{
			vsfshell_printf(outpt,
						"fail to parse ip address: %s"VSFSHELL_LINEEND,
							param->argv[2]);
			goto end;
		}
		lparam->dns_server = &lparam->ip;
	}
	else
	{
		lparam->dns_server = &vsfip.netif_default->dns[0];
	}

	err = vsfip_dnsc_setserver(0, lparam->dns_server);
	if (err < 0)
	{
		vsfshell_printf(outpt,
						"fail to set dns server: %d.%d.%d.%d"VSFSHELL_LINEEND,
							lparam->dns_server->addr.s_addr_buf[0],
							lparam->dns_server->addr.s_addr_buf[1],
							lparam->dns_server->addr.s_addr_buf[2],
							lparam->dns_server->addr.s_addr_buf[3]);
		goto end;
	}

	lparam->local_pt.sm = pt->sm;
	lparam->local_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_gethostbyname(&lparam->local_pt, evt, param->argv[1],
							&lparam->ip);
	if (err > 0) return err; else if (err < 0)
	{
		vsfshell_printf(outpt, "fail to get ip address for: %s"VSFSHELL_LINEEND,
							param->argv[1]);
		goto end;
	}

	vsfshell_printf(outpt, "%d.%d.%d.%d"VSFSHELL_LINEEND,
							lparam->ip.addr.s_addr_buf[0],
							lparam->ip.addr.s_addr_buf[1],
							lparam->ip.addr.s_addr_buf[2],
							lparam->ip.addr.s_addr_buf[3]);

end:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

// usb host
static vsf_err_t vsfos_busybox_lsusb(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
						(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	vsfshell_printf(outpt, "not supported now"VSFSHELL_LINEEND);
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsfos_busybox_init(struct vsfshell_t *shell)
{
	struct vsfshell_handler_t *handlers = vsf_bufmgr_malloc(VSFOSCFG_HANDLER_NUM * sizeof(*handlers));
	struct vsfos_ctx_t *ctx = vsf_bufmgr_malloc(sizeof(*ctx));
	int idx = 0;

	if (!handlers || !ctx) return VSFERR_NOT_ENOUGH_RESOURCES;
	memset(handlers, 0, VSFOSCFG_HANDLER_NUM * sizeof(struct vsfshell_handler_t));
	memset(ctx, 0, sizeof(*ctx));
	ctx->curfile = (struct vsfile_t *)&vsfile.rootfs;

	handlers[idx++] = (struct vsfshell_handler_t){"help", vsfos_busybox_help, ctx};
	handlers[idx++] = (struct vsfshell_handler_t){"uname", vsfos_busybox_uname, ctx};
	handlers[idx++] = (struct vsfshell_handler_t){"free", vsfos_busybox_free, ctx};
	handlers[idx++] = (struct vsfshell_handler_t){"top", vsfos_busybox_top, ctx};

	// module handlers
	handlers[idx++] = (struct vsfshell_handler_t){"lsmod", vsfos_busybox_lsmod, ctx};
	handlers[idx++] = (struct vsfshell_handler_t){"insmod", vsfos_busybox_insmod, ctx};
	handlers[idx++] = (struct vsfshell_handler_t){"repo", vsfos_busybox_repo, ctx};

	// fs handlers
	if (vsf_module_get(VSFILE_MODNAME) != NULL)
	{
		handlers[idx++] = (struct vsfshell_handler_t){"ls", vsfos_busybox_ls, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"cd", vsfos_busybox_cd, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"pwd", vsfos_busybox_pwd, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"mkdir", vsfos_busybox_mkdir, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"rmdir", vsfos_busybox_rmdir, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"rm", vsfos_busybox_rm, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"mv", vsfos_busybox_mv, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"cp", vsfos_busybox_cp, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"cat", vsfos_busybox_cat, ctx};
	}

	// net handlers
	if (vsf_module_get(VSFIP_MODNAME) != NULL)
	{
		handlers[idx++] = (struct vsfshell_handler_t){"ipconfig", vsfos_busybox_ipconfig, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"arp", vsfos_busybox_arp, ctx};
		handlers[idx++] = (struct vsfshell_handler_t){"ping", vsfos_busybox_ping, ctx};
		if (vsf_module_get(VSFIP_HTTPD_MODNAME) != NULL)
		{
			handlers[idx++] = (struct vsfshell_handler_t){"httpd", vsfos_busybox_httpd, ctx};
		}
		if (vsf_module_get(VSFIP_DNSC_MODNAME) != NULL)
		{
			handlers[idx++] = (struct vsfshell_handler_t){"dns", vsfos_busybox_dns, ctx};
			vsfip_dnsc_init();
		}
	}

	// usb host
	if (vsf_module_get(VSFUSBH_MODNAME) != NULL)
	{
		handlers[idx++] = (struct vsfshell_handler_t){"lsusb", vsfos_busybox_lsusb, ctx};
	}

	if (idx >= VSFOSCFG_HANDLER_NUM)
	{
		// memory destroyed
		while(1);
	}
	vsfshell_register_handlers(shell, handlers, idx);
	return VSFERR_NONE;
}
