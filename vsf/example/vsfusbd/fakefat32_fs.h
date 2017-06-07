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
#ifndef __FAKEFAT32_FS_H_INCLUDED__
#define __FAKEFAT32_FS_H_INCLUDED__

#include "component/fakefat32/fakefat32.h"

// fakefat32
static const uint8_t vsfrndis_inf[] =
"\
[Version]\r\n\
Signature		= \"$Windows NT$\"\r\n\
Class			= Net\r\n\
ClassGUID		= {4d36e972-e325-11ce-bfc1-08002be10318}\r\n\
Provider		= %Microsoft%\r\n\
DriverVer		= 07/21/2008,6.0.6000.16384\r\n\
[Manufacturer]\r\n\
%Microsoft%		= RndisDevices,NTx86,NTamd64,NTia64\r\n\
[RndisDevices.NTx86]\r\n\
%RndisDevice%	= RNDIS.NT.6.0, USB\\VID_0483&PID_A03A&MI_00\r\n\
[RndisDevices.NTamd64]\r\n\
%RndisDevice%	= RNDIS.NT.6.0, USB\\VID_0483&PID_A03A&MI_00\r\n\
[RndisDevices.NTia64]\r\n\
%RndisDevice%	= RNDIS.NT.6.0, USB\\VID_0483&PID_A03A&MI_00\r\n\
[ControlFlags]\r\n\
ExcludeFromSelect=*\r\n\
[RNDIS.NT.6.0]\r\n\
Characteristics	= 0x84\r\n\
BusType			= 15\r\n\
include			= netrndis.inf\r\n\
needs			= usbrndis6.ndi\r\n\
AddReg			= Rndis_AddReg\r\n\
*IfType				= 6\r\n\
*MediaType			= 16\r\n\
*PhysicalMediaType	= 14\r\n\
[RNDIS.NT.6.0.Services]\r\n\
include			= netrndis.inf\r\n\
needs			= usbrndis6.ndi.Services\r\n\
[RNDIS_AddReg] \r\n\
HKR, NDI\\params\\RndisProperty, ParamDesc,  0, %Rndis_Property%\r\n\
HKR, NDI\\params\\RndisProperty, type,       0, \"edit\"\r\n\
HKR, NDI\\params\\RndisProperty, LimitText,  0, \"12\"\r\n\
HKR, NDI\\params\\RndisProperty, UpperCase,  0, \"1\"\r\n\
HKR, NDI\\params\\RndisProperty, default,    0, \" \"\r\n\
HKR, NDI\\params\\RndisProperty, optional,   0, \"1\"\r\n\
[Strings]\r\n\
Microsoft		= \"Microsoft Corporation\"\r\n\
RndisDevice		= \"Remote NDIS6 based Device\"\r\n\
Rndis_Property	= \"Optional RNDIS Property\"\
";

static struct fakefat32_file_t fakefat32_root_dir[] =
{
	{
		.memfile.file.name = "VSFDriver",
		.memfile.file.attr = VSFILE_ATTR_VOLUMID,
	},
	{
		.memfile.file.name = "VSFRNDIS.inf",
		.memfile.file.size = sizeof(vsfrndis_inf) - 1,
		.memfile.file.attr = VSFILE_ATTR_ARCHIVE | VSFILE_ATTR_READONLY,
		.memfile.f.buff = (uint8_t *)vsfrndis_inf,
	},
	{
		.memfile.file.name = NULL,
	},
};

#endif		// __FAKEFAT32_FS_H_INCLUDED__
