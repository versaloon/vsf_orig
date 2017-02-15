/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces_const.h                                        *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    consts of interface module                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2011-04-08:     created(by SimonQian)                             *
 **************************************************************************/
#ifndef __CMEM7_INTERFACE_CONST_H_INCLUDED__
#define __CMEM7_INTERFACE_CONST_H_INCLUDED__

// cmem7 usart
#define cmem7_USART_STOPBITS_0P5	(0x0 << 0)
#define cmem7_USART_STOPBITS_1		(0x1 << 0)
#define cmem7_USART_STOPBITS_1P5	(0x2 << 0)
#define cmem7_USART_STOPBITS_2		(0x3 << 0)
#define cmem7_USART_PARITY_NONE		(0x0 << 4)
#define cmem7_USART_PARITY_ODD		(0x1 << 4)
#define cmem7_USART_PARITY_EVEN		(0x2 << 4)
#define cmem7_USART_DATALEN_8		(0x0 << 8)
#define cmem7_USART_DATALEN_9		(0x1 << 8)


#endif	// __CMEM7_INTERFACE_CONST_H_INCLUDED__

