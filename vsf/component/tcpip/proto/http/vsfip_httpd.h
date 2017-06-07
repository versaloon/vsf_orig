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
#ifndef __VSFIP_HTTPD_H_INCLUDED__
#define __VSFIP_HTTPD_H_INCLUDED__

//#define HTTPD_DEBUG
enum vsfip_httpd_req_t
{
	VSFIP_HTTP_POST,
	VSFIP_HTTP_GET,
};

enum vsfip_httpd_resp_t
{
	VSFIP_HTTP_200_OK = 200,
	VSFIP_HTTP_404_NOTFOUND = 404,
};

struct vsfip_httpd_service_req_t
{
	struct vsfip_buffer_t *inbuf;

	enum vsfip_httpd_req_t req;
	char *url;
	char *arg;
	char *head;
	char *body;

	union
	{
		struct
		{
			uint32_t size;
			uint8_t *buf;
			uint8_t type;
		} post;
	};
};

struct vsfip_httpd_service_resp_t
{
	enum vsfip_httpd_resp_t resp;
	char *target_filename;
	struct vsfip_buffer_t *outbuf;

	// private
	struct vsfile_t *targetfile;
	uint32_t fileoffset;
	struct vsfip_httpd_urlhandler_t *handler;
};

struct vsfip_httpd_urlhandler_t;
struct vsfip_httpd_t;
struct vsfip_httpd_service_t
{
	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t caller_pt;

	struct vsfip_httpd_service_req_t req;
	struct vsfip_httpd_service_resp_t resp;

	struct vsfip_socket_t *so;
	struct vsfip_httpd_t *httpd;
};

struct vsfip_httpd_urlhandler_t
{
	char *url;
	vsf_err_t (*handle)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
				struct vsfip_httpd_service_t *service);
	void *param;
};

struct vsfip_httpd_cb_t
{
	vsf_err_t (*onca)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						struct vsfip_httpd_service_t *service);
	void *param;
};

struct vsfip_httpd_t
{
	struct vsfip_httpd_service_t *service;
	uint32_t service_num;
	struct vsfile_t *root;
	char *homepage;
	struct vsfip_httpd_urlhandler_t *urlhandler;
	struct vsfip_httpd_cb_t cb;

	// private
	struct vsfip_sockaddr_t sockaddr;
	struct vsfip_socket_t *so;
	struct vsfip_socket_t *acceptso;

	bool isactive;
	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t daemon_pt;
};

#define VSFIP_HTTPD_MIMETYPECNT				10
#define VSFIP_HTTPD_MIMETYPE_XWWW			0
#define VSFIP_HTTPD_MIMETYPE_MUTIFORM		1

struct vsfip_http_mimetype_t
{
	char *str;
	char *ext;
};

#ifdef VSFCFG_STANDALONE_MODULE
#define VSFIP_HTTPD_MODNAME					"vsf.stack.net.tcpip.proto.httpd"

struct vsfip_httpd_modifs_t
{
	vsf_err_t (*start)(struct vsfip_httpd_t*, uint16_t);

	vsf_err_t (*header_resp)(struct vsfip_httpd_service_resp_t*,
								enum vsfip_httpd_resp_t);
	vsf_err_t (*header_str)(struct vsfip_httpd_service_resp_t*,
								const char*, const char*);
	vsf_err_t (*header_u32)(struct vsfip_httpd_service_resp_t*,
								const char*, uint32_t);
	vsf_err_t (*header_end)(struct vsfip_httpd_service_resp_t*);
	char* (*getarg)(char*, char*, uint32_t*);

	struct vsfip_http_mimetype_t mimetype[VSFIP_HTTPD_MIMETYPECNT];
	struct vsfile_memfile_t http400;
	struct vsfile_memfile_t http404;
};

vsf_err_t vsfip_httpd_modexit(struct vsf_module_t*);
vsf_err_t vsfip_httpd_modinit(struct vsf_module_t*, struct app_hwcfg_t const*);

#define VSFIP_HTTPDMOD						\
	((struct vsfip_httpd_modifs_t *)vsf_module_load(VSFIP_HTTPD_MODNAME, true))
#define vsfip_httpd_start					VSFIP_HTTPDMOD->start
#define vsfip_httpd_header_resp				VSFIP_HTTPDMOD->header_resp
#define vsfip_httpd_header_str				VSFIP_HTTPDMOD->header_str
#define vsfip_httpd_header_u32				VSFIP_HTTPDMOD->header_u32
#define vsfip_httpd_header_end				VSFIP_HTTPDMOD->header_end
#define vsfip_httpd_getarg					VSFIP_HTTPDMOD->getarg
#define vsfip_http400						VSFIP_HTTPDMOD->http400
#define vsfip_http404						VSFIP_HTTPDMOD->http404
#define vsfip_httpd_mimetype				VSFIP_HTTPDMOD->mimetype

#else
vsf_err_t vsfip_httpd_start(struct vsfip_httpd_t *httpd, uint16_t port);

// used by urlhandler
vsf_err_t vsfip_httpd_header_resp(struct vsfip_httpd_service_resp_t *resp,
								enum vsfip_httpd_resp_t r);
vsf_err_t vsfip_httpd_header_str(struct vsfip_httpd_service_resp_t *resp,
								const char *field, const char *value);
vsf_err_t vsfip_httpd_header_u32(struct vsfip_httpd_service_resp_t *resp,
								const char *field, uint32_t value);
vsf_err_t vsfip_httpd_header_end(struct vsfip_httpd_service_resp_t *resp);

char* vsfip_httpd_getheader(char *src, char *name, uint32_t *valuesize);
char* vsfip_httpd_getarg(char *src, char *name, uint32_t *valuesize);
#endif

#endif		// __VSFIP_HTTPD_H_INCLUDED__
