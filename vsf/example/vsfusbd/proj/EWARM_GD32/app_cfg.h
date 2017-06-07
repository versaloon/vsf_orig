/**************************************************************************
 *  Copyright (C) 2008 - 2012 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    VSF                                                       *
 *  File:       app_cfg.h                                                 *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    configuration file                                        *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

// hardware config file
#include "hw_cfg_GD32.h"

// compiler config
#include "compiler.h"

#define APPCFG_VSFTIMER_NUM				4
// For IAR KickStarter users:
//	remove APPCFG_BUFMGR_SIZE and the binary will be below 32K
//#define APPCFG_BUFMGR_SIZE				512

// The 3 MACROs below define the Hard/Soft/Non Real Time tasking
// undefine to indicating that corresponding real time tasking is unsupported
//	note that AT LEASE one real time tasking should be defined
// define to 0 indicating that corresponding real time tasking is unqueued
//	note that the real time tasking can be unqueued ONLY IF it will not receive
//		events from tasks in higher priority real time tasking
// define to n indicating the length of corresponding real time takesing queue
//#define APPCFG_HRT_QUEUE_LEN			0
#define APPCFG_SRT_QUEUE_LEN			0
//#define APPCFG_NRT_QUEUE_LEN			0

#if (defined(APPCFG_HRT_QUEUE_LEN) && (APPCFG_HRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_SRT_QUEUE_LEN) && (APPCFG_SRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_NRT_QUEUE_LEN) && (APPCFG_NRT_QUEUE_LEN > 0))
#define VSFSM_CFG_PREMPT_EN				1
#else
#define VSFSM_CFG_PREMPT_EN				0
#endif

#ifdef APPCFG_BUFMGR_SIZE
#define APPCFG_VSFIP_BUFFER_NUM			1
#define APPCFG_VSFIP_SOCKET_NUM			1
#define APPCFG_VSFIP_TCPPCB_NUM			1
#else
#define APPCFG_VSFIP_BUFFER_NUM			2
#define APPCFG_VSFIP_SOCKET_NUM			2
#define APPCFG_VSFIP_TCPPCB_NUM			1
#endif
