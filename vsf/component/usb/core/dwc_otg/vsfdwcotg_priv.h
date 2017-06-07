#include "dwcotg_regs.h"

#define MAX_HC_NUM_EACH_DEVICE	8

/* USB HUB CONSTANTS */
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

enum hc_state_t
{
	HC_IDLE = 0,
	HC_WAIT,
	HC_START,
	HC_XFRC,
	HC_HALTED,
	HC_NAK,
	HC_NYET,
	HC_STALL,
	HC_XACTERR,
	HC_BBLERR,
	HC_DATATGLERR
};
enum hc_dpid_t
{
	HC_DPID_DATA0 = 0,
	HC_DPID_DATA2,
	HC_DPID_DATA1,
	HC_DPID_SETUP,
};
struct hc_t
{
	uint8_t hc_num : 7;
	uint8_t alloced : 1;

	uint8_t dev_addr : 7;
	uint8_t dir_o0_i1 : 1;

	uint8_t ep_num : 4;
	uint8_t speed : 2;
	uint8_t err_cnt : 2;

	uint8_t hc_state : 4;
	uint8_t dpid : 2;
	uint8_t : 1;

	uint32_t transfer_size;

	struct dwcotg_device_t *owner_dev;
	struct urb_priv_t *owner_priv;
};

enum urb_priv_type_t
{
	URB_PRIV_TYPE_ISO = 0,
	URB_PRIV_TYPE_INT,
	URB_PRIV_TYPE_CTRL,
	URB_PRIV_TYPE_BULK,
};
enum urb_priv_phase_t
{
	URB_PRIV_PHASE_IDLE = 0,
	URB_PRIV_PHASE_PERIOD_WAIT,
	URB_PRIV_PHASE_SETUP_WAIT,
	URB_PRIV_PHASE_DATA_WAIT,
	URB_PRIV_PHASE_STATE_WAIT,
	URB_PRIV_PHASE_DONE,
};
enum urb_priv_state_t
{
	URB_PRIV_STATE_IDLE = 0,
	URB_PRIV_STATE_NAK,
	URB_PRIV_STATE_DONE,
	URB_PRIV_STATE_NOTREADY,
	URB_PRIV_STATE_NYET,
	URB_PRIV_STATE_ERROR,
	URB_PRIV_STATE_STALL,
};
struct urb_priv_t
{
	uint8_t type : 2;
	uint8_t phase : 3;
	uint8_t state : 3;

	uint8_t toggle_start : 1;
	uint8_t toggle_next : 1;
	uint8_t do_ping : 1;
	uint8_t dir_o0_i1 : 1;
	uint8_t discarded : 1;
	uint8_t  : 3;
	uint16_t actual_length;

	void *transfer_buffer;
	uint16_t transfer_length;
	uint16_t packet_size;

	struct hc_t *hc;
};

struct dwcotg_device_t
{
	struct hc_t *hc[MAX_HC_NUM_EACH_DEVICE];
	uint32_t hc_num;

	struct vsfusbh_device_t *vsfdev;
};

enum dwcotg_state_t
{
	DWCOTG_DISABLE = 0,
	DWCOTG_WORKING,
};
struct dwcotg_t
{
	// common config
	uint8_t dwcotg_state;
	uint8_t speed;	// USB_SPEED_LOW / USB_SPEED_FULL / USB_SPEED_HIGH
	uint8_t dma_en;
	uint8_t ulpi_en;
	uint8_t external_vbus_en;

	uint8_t ep_in_amount;
	uint8_t ep_out_amount;
	uint8_t hc_amount;

	uint16_t retry;

	struct hc_t *hc_pool;

	volatile uint32_t softick;

	// Core Global registers starting at offset 000h
	struct dwcotg_core_global_regs_t *global_reg;
	// Host Global Registers starting at offset 400h.
	struct dwcotg_host_global_regs_t *host_global_regs;
	// Host Port 0 Control and Status Register at offset 440h.
	volatile uint32_t *hprt0;
	// Host Channel Specific Registers at offsets 500h-5FCh.
	struct dwcotg_hc_regs_t *hc_regs;
	// Device Global Registers starting at offset 800h
	struct dwcotg_dev_global_regs_t *dev_global_regs;
	// Device Logical IN Endpoint-Specific Registers 900h-AFCh
	struct dwcotg_dev_in_ep_regs_t *in_ep_regs;
	// Device Logical OUT Endpoint-Specific Registers B00h-CFCh
	struct dwcotg_dev_out_ep_regs_t *out_ep_regs;

	uint32_t *dfifo;
};
