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

#ifndef __VSFOHCI_H_INCLUDED__
#define __VSFOHCI_H_INCLUDED__

/*******************************************************
 * OHCI config
 *******************************************************/
#define OHCI_ENABLE_ISO				0
#define OHCI_ISO_PACKET_LIMIT		4

#define MAXPSW					1
#define TD_MAX_NUM				64
#define NUM_INTS				32
#define MAX_EP_NUM_EACH_DEVICE	8
#define TD_MAX_NUM_EACH_URB		8
/* Maximum number of root hub ports. */
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 	3

/*******************************************************
 * OHCI registers
 *******************************************************/
/*
 * HcControl (control) register masks
 */
#define OHCI_CTRL_CBSR			(3 << 0)	/* control/bulk service ratio */
#define OHCI_CTRL_PLE			(1 << 2)	/* periodic list enable */
#define OHCI_CTRL_IE			(1 << 3)	/* isochronous enable */
#define OHCI_CTRL_CLE			(1 << 4)	/* control list enable */
#define OHCI_CTRL_BLE			(1 << 5)	/* bulk list enable */
#define OHCI_CTRL_HCFS			(3 << 6)	/* host controller functional state */
#define OHCI_CTRL_IR			(1 << 8)	/* interrupt routing */
#define OHCI_CTRL_RWC			(1 << 9)	/* remote wakeup connected */
#define OHCI_CTRL_RWE			(1 << 10)	/* remote wakeup enable */

/* For initializing controller (mask in an HCFS mode too) */
#define OHCI_CONTROL_INIT \
	(OHCI_CTRL_CBSR & 0x3) | OHCI_CTRL_IE | OHCI_CTRL_PLE | OHCI_CTRL_CLE

/* pre-shifted values for HCFS */
#define OHCI_USB_RESET			(0 << 6)
#define OHCI_USB_RESUME			(1 << 6)
#define OHCI_USB_OPER			(2 << 6)
#define OHCI_USB_SUSPEND		(3 << 6)

/*
 * HcCommandStatus (cmdstatus) register masks
 */
#define OHCI_HCR				(1 << 0)	/* host controller reset */
#define OHCI_CLF				(1 << 1)	/* control list filled */
#define OHCI_BLF				(1 << 2)	/* bulk list filled */
#define OHCI_OCR				(1 << 3)	/* ownership change request */
#define OHCI_SOC				(3 << 16)	/* scheduling overrun count */

/*
 * masks used with interrupt registers:
 * HcInterruptStatus (intrstatus)
 * HcInterruptEnable (intrenable)
 * HcInterruptDisable (intrdisable)
 */
#define OHCI_INTR_SO			(0x1ul << 0)	/* scheduling overrun */
#define OHCI_INTR_WDH			(0x1ul << 1)	/* writeback of done_head */
#define OHCI_INTR_SF			(0x1ul << 2)	/* start frame */
#define OHCI_INTR_RD			(0x1ul << 3)	/* resume detect */
#define OHCI_INTR_UE			(0x1ul << 4)	/* unrecoverable error */
#define OHCI_INTR_FNO			(0x1ul << 5)	/* frame number overflow */
#define OHCI_INTR_RHSC			(0x1ul << 6)	/* root hub status change */
#define OHCI_INTR_OC			(0x1ul << 30)	/* ownership change */
#define OHCI_INTR_MIE			(0x1ul << 31)	/* master interrupt enable */

// ED States
#define ED_IDLE					0x00
#define ED_UNLINK				0x01
#define ED_OPER					0x02
#define ED_DEL					0x04
#define ED_URB_DEL				0x08

/* TD info field */
#define TD_CC					0xf0000000
#define TD_CC_GET(td_p)			((td_p >>28) & 0x0f)
#define TD_CC_SET(td_p, cc)		(td_p) = ((td_p) & 0x0fffffff) | (((cc) & 0x0f) << 28)
#define TD_EC					0x0C000000
#define TD_T					0x03000000
#define TD_T_DATA0				0x02000000
#define TD_T_DATA1				0x03000000
#define TD_T_TOGGLE				0x00000000
#define TD_R					0x00040000
#define TD_DI					0x00E00000
#define TD_DI_SET(X)			(((X) & 0x07)<< 21)
#define TD_DP					0x00180000
#define TD_DP_SETUP				0x00000000
#define TD_DP_IN				0x00100000
#define TD_DP_OUT				0x00080000

#define TD_ISO					0x00010000
#define TD_DEL					0x00020000

/* CC Codes */
#define TD_CC_NOERROR			0x00
#define TD_CC_CRC				0x01
#define TD_CC_BITSTUFFING		0x02
#define TD_CC_DATATOGGLEM		0x03
#define TD_CC_STALL				0x04
#define TD_DEVNOTRESP			0x05
#define TD_PIDCHECKFAIL			0x06
#define TD_UNEXPECTEDPID		0x07
#define TD_DATAOVERRUN			0x08
#define TD_DATAUNDERRUN			0x09
#define TD_BUFFEROVERRUN		0x0C
#define TD_BUFFERUNDERRUN		0x0D
#define TD_NOTACCESSED			0x0F



/* USB HUB CONSTANTS (not OHCI-specific; see hub.h) */
/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS			0x0080
#define RH_CLEAR_FEATURE		0x0100
#define RH_SET_FEATURE			0x0300
#define RH_SET_ADDRESS			0x0500
#define RH_GET_DESCRIPTOR		0x0680
#define RH_SET_DESCRIPTOR		0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE			0x0280
#define RH_GET_INTERFACE		0x0A80
#define RH_SET_INTERFACE		0x0B00
#define RH_SYNC_FRAME			0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP		0x2000


/* Hub port features */
#define RH_PORT_CONNECTION		0x00
#define RH_PORT_ENABLE			0x01
#define RH_PORT_SUSPEND			0x02
#define RH_PORT_OVER_CURRENT	0x03
#define RH_PORT_RESET			0x04
#define RH_PORT_POWER			0x08
#define RH_PORT_LOW_SPEED		0x09

#define RH_C_PORT_CONNECTION	0x10
#define RH_C_PORT_ENABLE		0x11
#define RH_C_PORT_SUSPEND		0x12
#define RH_C_PORT_OVER_CURRENT	0x13
#define RH_C_PORT_RESET			0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER	0x00
#define RH_C_HUB_OVER_CURRENT	0x01

#define RH_DEVICE_REMOTE_WAKEUP	0x00
#define RH_ENDPOINT_STALL		0x01

#define RH_ACK					0x01
#define RH_REQ_ERR				-1
#define RH_NACK					0x00

#define RH_OK(x)				len = (x); break
#define WR_RH_STAT(x)			(regs->roothub.status = (x))
#define WR_RH_PORTSTAT(x)		(regs->roothub.portstatus[wIndex-1] = (x))
#define RD_RH_STAT				(regs->roothub.status)
#define RD_RH_PORTSTAT			(regs->roothub.portstatus[wIndex-1])

/* OHCI ROOT HUB REGISTER MASKS */

/* roothub.portstatus [i] bits */
#define RH_PS_CCS				0x00000001		/* current connect status */
#define RH_PS_PES				0x00000002		/* port enable status*/
#define RH_PS_PSS				0x00000004		/* port suspend status */
#define RH_PS_POCI				0x00000008		/* port over current indicator */
#define RH_PS_PRS				0x00000010		/* port reset status */
#define RH_PS_PPS				0x00000100		/* port power status */
#define RH_PS_LSDA				0x00000200		/* low speed device attached */
#define RH_PS_CSC				0x00010000		/* connect status change */
#define RH_PS_PESC				0x00020000		/* port enable status change */
#define RH_PS_PSSC				0x00040000		/* port suspend status change */
#define RH_PS_OCIC				0x00080000		/* over current indicator change */
#define RH_PS_PRSC				0x00100000		/* port reset status change */

/* roothub.status bits */
#define RH_HS_LPS				0x00000001		/* local power status */
#define RH_HS_OCI				0x00000002		/* over current indicator */
#define RH_HS_DRWE				0x00008000		/* device remote wakeup enable */
#define RH_HS_LPSC				0x00010000		/* local power status change */
#define RH_HS_OCIC				0x00020000		/* over current indicator change */
#define RH_HS_CRWE				0x80000000		/* clear remote wakeup enable */

/* roothub.b masks */
#define RH_B_DR					0x0000ffff		/* device removable flags */
#define RH_B_PPCM				0xffff0000		/* port power control mask */

/* roothub.a masks */
#define RH_A_NDP				(0xfful << 0)		/* number of downstream ports */
#define RH_A_PSM				(0x1ul << 8)		/* power switching mode */
#define RH_A_NPS				(0x1ul << 9)		/* no power switching */
#define RH_A_DT					(0x1ul << 10)		/* device type (mbz) */
#define RH_A_OCPM				(0x1ul << 11)		/* over current protection mode */
#define RH_A_NOCP				(0x1ul << 12)		/* no over current protection */
#define RH_A_POTPGT				(0xfful << 24)		/* power on to power good time */

/*******************************hc_start****************
 * structures
 *******************************************************/
/* The HCCA (Host Controller Communications Area) structure
 * Must be aligned to 256 bytes address
 */
struct ohci_hcca_t
{
	uint32_t int_table[NUM_INTS];	/* Interrupt ED table */
	uint16_t frame_no;				/* current frame number */
	uint16_t pad1;					/* set to 0 on each frame_no change */
	uint32_t done_head;				/* onfo returned for an interrupt */
	//uint8_t reserved_for_hc[116];
};

/* OHCI register defination */
struct ohci_regs_t
{
	/* control and status registers */
	uint32_t revision;
	uint32_t control;
	uint32_t cmdstatus;
	uint32_t intrstatus;
	uint32_t intrenable;
	uint32_t intrdisable;
	/* memory pointers */
	uint32_t hcca;
	uint32_t ed_periodcurrent;
	uint32_t ed_controlhead;
	uint32_t ed_controlcurrent;
	uint32_t ed_bulkhead;
	uint32_t ed_bulkcurrent;
	uint32_t donehead;
	/* frame counters */
	uint32_t fminterval;
	uint32_t fmremaining;
	uint32_t fmnumber;
	uint32_t periodicstart;
	uint32_t lsthresh;
	/* Root hub ports */
	struct ohci_roothub_regs
	{
		uint32_t a;
		uint32_t b;
		uint32_t status;
		uint32_t portstatus[CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS];
	} roothub;
};

/* usb OHCI ed, must be aligned to 16 bytes */
struct ed_t
{
	uint32_t hwINFO;
#define ED_DEQUEUE				(0x1ul << 27)
#define ED_ISO					(0x1ul << 15)
#define ED_SKIP					(0x1ul << 14)
#define ED_LOWSPEED				(0x1ul << 13)
#define ED_OUT					(0x1ul << 11)
#define ED_IN					(0x2ul << 11)
	uint32_t hwTailP;
	uint32_t hwHeadP;
#define ED_C					0x02ul
#define ED_H					0x01ul
	uint32_t hwNextED;

	int8_t branch;
	uint32_t load : 10;
	uint32_t interval : 6;

	uint32_t rm_frame :1;
	uint32_t busy : 1;
	uint32_t type : 2;
	uint32_t : 4;

	struct ed_t *prev;
	struct ed_t *ed_next;
	struct td_t *dummy_td;
	//struct urb_priv_t *urb_priv;

	//struct vsfusbh_device_t *vsfdev;
	//struct td_t *dummy_td;
	//struct td_t *td_list;
};

/* usb OHCI td, must be aligned to 32 bytes */
struct td_t
{
	uint32_t hwINFO;
	uint32_t hwCBP;		/* Current Buffer Pointer */
	uint32_t hwNextTD;	/* Next TD Pointer */
	uint32_t hwBE;		/* Memory Buffer End Pointer */
#if OHCI_ENABLE_ISO
	uint32_t hwPSW[4];
#endif // OHCI_ENABLE_ISO

	uint8_t busy : 1;
	uint8_t :7;
	uint8_t index;
	uint16_t num;

	struct urb_priv_t *urb_priv;
	struct td_t *next_dl_td;
	struct ed_t *ed_dummy;
#if OHCI_ENABLE_ISO
	uint32_t dummy[4];
#endif // OHCI_ENABLE_ISO
};

/* Full ohci controller descriptor */
struct vsfohci_t;
struct ohci_t
{
	struct ohci_hcca_t *hcca;
	uint8_t disabled;
	uint8_t working :1;
	uint8_t restart_work :1;
	uint8_t :6;
	uint16_t resume_count;

	uint16_t load[32];		/* load of the 32 Interrupt Chains (for load balancing)*/
	struct ed_t *ed_rm_list;
	struct ed_t *ed_bulktail;
	struct ed_t *ed_controltail;
	struct ed_t *ed_isotail;
	struct td_t *dl_start;

	uint32_t hc_control;			/* copy of the hc control reg */

	struct ohci_regs_t *regs;
	struct vsfohci_t *vsfohci;
};

struct vsfohci_t
{
	struct ohci_t *ohci;

	struct td_t *td_pool;

	struct vsfsm_t *sm;

	uint16_t loops;
};

struct urb_priv_t
{
	struct ed_t *ed;
	uint16_t length;			/* number of tds associated with this request */
	uint16_t td_cnt;			/* number of tds already serviced */
	uint16_t state;
#define URB_PRIV_INIT				0
#define URB_PRIV_EDLINK				(0x1 << 1)
#define URB_PRIV_EDSKIP				(0x1 << 2)
#define URB_PRIV_TDALLOC			(0x1 << 3)
#define URB_PRIV_TDLINK				(0x1 << 4)
#define URB_PRIV_WAIT_COMPLETE		(0x1 << 5)
#define URB_PRIV_WAIT_DELETE		(0x1 << 6)
	struct td_t *td[TD_MAX_NUM_EACH_URB];	/* list pointer to all corresponding TDs associated with this request */
#if OHCI_ENABLE_ISO
	uint32_t number_of_packets;		/*!< number of packets (iso)		*/
	uint32_t error_count;			/*!< number of errors (iso only)	*/
	struct iso_packet_descriptor_t iso_frame_desc[OHCI_ISO_PACKET_LIMIT];
#endif // OHCI_ENABLE_ISO
};

#endif	// __VSFOHCI_H_INCLUDED__
