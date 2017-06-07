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
; Remote NDIS template device setup file\r\n\
; Copyright (c) Microsoft Corporation\r\n\
;\r\n\
; This is the template for the INF installation script  for the RNDIS-over-USB\r\n\
; host driver that leverages the newer NDIS 6.x miniport (rndismp6.sys) for\r\n\
; improved performance. This INF works for Windows 7, Windows Server 2008 R2,\r\n\
; and later operating systems on x86, amd64 and ia64 platforms.\r\n\
\r\n\
[Version]\r\n\
Signature           = \"$Windows NT$\"\r\n\
Class               = Net\r\n\
ClassGUID           = {4d36e972-e325-11ce-bfc1-08002be10318}\r\n\
Provider            = %Microsoft%\r\n\
DriverVer           = 07/21/2008,6.0.6000.16384\r\n\
;CatalogFile        = device.cat\r\n\
\r\n\
[Manufacturer]\r\n\
%Microsoft%         = RndisDevices,NTx86,NTamd64,NTia64\r\n\
\r\n\
; Decoration for x86 architecture\r\n\
[RndisDevices.NTx86]\r\n\
%RndisDevice%    = RNDIS.NT.6.0, USB\\VID_0483&PID_A03A&MI_00\r\n\
\r\n\
; Decoration for x64 architecture\r\n\
[RndisDevices.NTamd64]\r\n\
%RndisDevice%    = RNDIS.NT.6.0, USB\\VID_0483&PID_A03A&MI_00\r\n\
\r\n\
; Decoration for ia64 architecture\r\n\
[RndisDevices.NTia64]\r\n\
%RndisDevice%    = RNDIS.NT.6.0, USB\\VID_0483&PID_A03A&MI_00\r\n\
\r\n\
;@@@ This is the common setting for setup\r\n\
[ControlFlags]\r\n\
ExcludeFromSelect=*\r\n\
\r\n\
; DDInstall section\r\n\
; References the in-build Netrndis.inf\r\n\
[RNDIS.NT.6.0]\r\n\
Characteristics = 0x84   ; NCF_PHYSICAL + NCF_HAS_UI\r\n\
BusType         = 15\r\n\
; NEVER REMOVE THE FOLLOWING REFERENCE FOR NETRNDIS.INF\r\n\
include         = netrndis.inf\r\n\
needs           = usbrndis6.ndi\r\n\
AddReg          = Rndis_AddReg\r\n\
*IfType            = 6    ; IF_TYPE_ETHERNET_CSMACD.\r\n\
*MediaType         = 16   ; NdisMediumNative802_11\r\n\
*PhysicalMediaType = 14   ; NdisPhysicalMedium802_3\r\n\
\r\n\
; DDInstal.Services section\r\n\
[RNDIS.NT.6.0.Services]\r\n\
include     = netrndis.inf\r\n\
needs       = usbrndis6.ndi.Services\r\n\
\r\n\
; Optional registry settings. You can modify as needed.\r\n\
[RNDIS_AddReg] \r\n\
HKR, NDI\\params\\RndisProperty, ParamDesc,  0, %Rndis_Property%\r\n\
HKR, NDI\\params\\RndisProperty, type,       0, \"edit\"\r\n\
HKR, NDI\\params\\RndisProperty, LimitText,  0, \"12\"\r\n\
HKR, NDI\\params\\RndisProperty, UpperCase,  0, \"1\"\r\n\
HKR, NDI\\params\\RndisProperty, default,    0, \" \"\r\n\
HKR, NDI\\params\\RndisProperty, optional,   0, \"1\"\r\n\
\r\n\
; No sys copyfiles - the sys files are already in-build \r\n\
; (part of the operating system).\r\n\
\r\n\
; Modify these strings for your device as needed.\r\n\
[Strings]\r\n\
Microsoft             = \"Microsoft Corporation\"\r\n\
RndisDevice           = \"Remote NDIS6 based Device\"\r\n\
Rndis_Property         = \"Optional RNDIS Property\"\
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
