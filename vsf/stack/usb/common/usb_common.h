#ifndef __USB_COMMON_H__
#define __USB_COMMON_H__

#include "compiler.h"
#include "usb_type.h"

#include "usb_ch9.h"

#define USB_DT_HID          (USB_TYPE_CLASS | 0x01)
#define USB_DT_REPORT       (USB_TYPE_CLASS | 0x02)
#define USB_DT_PHYSICAL     (USB_TYPE_CLASS | 0x03)
#define USB_DT_HUB          (USB_TYPE_CLASS | 0x09)

/*
 * USB Packet IDs (PIDs)
 */
#define USB_PID_EXT			0xf0	/* USB 2.0 LPM ECN */
#define USB_PID_OUT			0xe1
#define USB_PID_ACK			0xd2
#define USB_PID_DATA0			0xc3
#define USB_PID_PING			0xb4	/* USB 2.0 */
#define USB_PID_SOF			0xa5
#define USB_PID_NYET			0x96	/* USB 2.0 */
#define USB_PID_DATA2			0x87	/* USB 2.0 */
#define USB_PID_SPLIT			0x78	/* USB 2.0 */
#define USB_PID_IN			0x69
#define USB_PID_NAK			0x5a
#define USB_PID_DATA1			0x4b
#define USB_PID_PREAMBLE		0x3c	/* Token mode */
#define USB_PID_ERR			0x3c	/* USB 2.0: handshake mode */
#define USB_PID_SETUP			0x2d
#define USB_PID_STALL			0x1e
#define USB_PID_MDATA			0x0f	/* USB 2.0 */


/* (shifted) direction/type/recipient from the USB 2.0 spec, table 9.2 */
#define DeviceRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_DEVICE)<<8)
#define DeviceOutRequest \
	((USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_DEVICE)<<8)

#define InterfaceRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)

#define EndpointRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)
#define EndpointOutRequest \
	((USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)

/* class requests from the USB 2.0 hub spec, table 11-15 */
/* GetBusState and SetHubDescriptor are optional, omitted */
#define ClearHubFeature		(0x2000 | USB_REQ_CLEAR_FEATURE)
#define ClearPortFeature	(0x2300 | USB_REQ_CLEAR_FEATURE)
#define GetHubDescriptor	(0xa000 | USB_REQ_GET_DESCRIPTOR)
#define GetHubStatus		(0xa000 | USB_REQ_GET_STATUS)
#define GetPortStatus		(0xa300 | USB_REQ_GET_STATUS)
#define SetHubFeature		(0x2000 | USB_REQ_SET_FEATURE)
#define SetPortFeature		(0x2300 | USB_REQ_SET_FEATURE)


/*-------------------------------------------------------------------------*/

/* class requests from USB 3.0 hub spec, table 10-5 */
#define SetHubDepth		(0x3000 | HUB_SET_DEPTH)
#define GetPortErrorCount	(0x8000 | HUB_GET_PORT_ERR_COUNT)

/*
 * Generic bandwidth allocation constants/support
 */
#define FRAME_TIME_USECS	1000L
#define BitTime(bytecount) (7 * 8 * bytecount / 6) /* with integer truncation */
		/* Trying not to use worst-case bit-stuffing
		 * of (7/6 * 8 * bytecount) = 9.33 * bytecount */
		/* bytecount = data payload byte count */

#define NS_TO_US(ns)	DIV_ROUND_UP(ns, 1000L)
			/* convert nanoseconds to microseconds, rounding up */

/*
 * Full/low speed bandwidth allocation constants/support.
 */
#define BW_HOST_DELAY	1000L		/* nanoseconds */
#define BW_HUB_LS_SETUP	333L		/* nanoseconds */
			/* 4 full-speed bit times (est.) */

#define FRAME_TIME_BITS			12000L	/* frame = 1 millisecond */
#define FRAME_TIME_MAX_BITS_ALLOC	(90L * FRAME_TIME_BITS / 100L)
#define FRAME_TIME_MAX_USECS_ALLOC	(90L * FRAME_TIME_USECS / 100L)

/*
 * Ceiling [nano/micro]seconds (typical) for that many bytes at high speed
 * ISO is a bit less, no ACK ... from USB 2.0 spec, 5.11.3 (and needed
 * to preallocate bandwidth)
 */
#define USB2_HOST_DELAY	5	/* nsec, guess */
#define HS_NSECS(bytes) (((55 * 8 * 2083) \
	+ (2083UL * (3 + BitTime(bytes))))/1000 \
	+ USB2_HOST_DELAY)
#define HS_NSECS_ISO(bytes) (((38 * 8 * 2083) \
	+ (2083UL * (3 + BitTime(bytes))))/1000 \
	+ USB2_HOST_DELAY)
#define HS_USECS(bytes)		NS_TO_US(HS_NSECS(bytes))
#define HS_USECS_ISO(bytes)	NS_TO_US(HS_NSECS_ISO(bytes))

/* Endpoint descriptor */
PACKED_HEAD struct PACKED_MID usb_endpoint_desc_t {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned char	bEndpointAddress;
	unsigned char	bmAttributes;
	unsigned short	wMaxPacketSize;
	unsigned char	bInterval;
	unsigned char	bRefresh;
	unsigned char	bSynchAddress;
	
	unsigned char	dummy;
	
	unsigned short	extralen;
	const unsigned char	*extra;
}; PACKED_TAIL

/* Interface descriptor */
PACKED_HEAD struct PACKED_MID usb_interface_desc_t
{
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned char	bInterfaceNumber;
	unsigned char	bAlternateSetting;
	unsigned char	bNumEndpoints;
	unsigned char	bInterfaceClass;
	unsigned char	bInterfaceSubClass;
	unsigned char	bInterfaceProtocol;
	unsigned char	iInterface;
	
	unsigned char	dummy;
	
	unsigned short	extralen;
	const unsigned char	*extra;
	
	struct usb_endpoint_desc_t *ep_desc;
}; PACKED_TAIL

PACKED_HEAD struct PACKED_MID usb_interface_t
{
	struct usb_interface_desc_t *altsetting;
	unsigned char act_altsetting;			/* active alternate setting */
	unsigned char num_altsetting;			/* number of alternate settings */
	unsigned char max_altsetting;			/* total memory allocated */
	unsigned char dummy;
	const struct vsfusbh_class_drv_t *driver;				/* driver */
	void *private_data;
}; PACKED_TAIL

PACKED_HEAD struct PACKED_MID usb_config_t
{
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned short	wTotalLength;
	unsigned char	bNumInterfaces;
	unsigned char	bConfigurationValue;
	unsigned char	iConfiguration;
	unsigned char	bmAttributes;
	unsigned char	MaxPower;
	
	unsigned char	dummy;
	
	//unsigned short	extralen;
	//const unsigned char	*extra;

	struct usb_interface_t *interface;
	
	uint8_t *config_buffer;
};

#define URB_OK				VSFERR_NONE
#define URB_FAIL			VSFERR_FAIL
#define URB_PENDING			VSFERR_NOT_READY


/* ----------------------------------------------------------------------- */

/*
 * For various legacy reasons, Linux has a small cookie that's paired with
 * a struct usb_device to identify an endpoint queue.  Queue characteristics
 * are defined by the endpoint's descriptor.  This cookie is called a "pipe",
 * an unsigned int encoded as:
 *
 *  - direction:	bit 7		(0 = Host-to-Device [Out],
 *					 1 = Device-to-Host [In] ...
 *					like endpoint bEndpointAddress)
 *  - device address:	bits 8-14       ... bit positions known to uhci-hcd
 *  - endpoint:		bits 15-18      ... bit positions known to uhci-hcd
 *  - pipe type:	bits 30-31	(00 = isochronous, 01 = interrupt,
 *					 10 = control, 11 = bulk)
 *
 * Given the device address and endpoint descriptor, pipes are redundant.
 */

/* NOTE:  these are not the standard USB_ENDPOINT_XFER_* values!! */
/* (yet ... they're the values used by usbfs) */
#define PIPE_ISOCHRONOUS		0ul
#define PIPE_INTERRUPT			1ul
#define PIPE_CONTROL			2ul
#define PIPE_BULK			3ul

#define usb_pipein(pipe)	(((pipe) & USB_DIR_IN) >> 7)
#define usb_pipeout(pipe)	(!usb_pipein(pipe))

#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)

#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	(usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	(usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	(usb_pipetype((pipe)) == PIPE_BULK)

#define usb_pipespeed(pipe)	(((pipe) >> 26) & 3)
#define usb_pipeslow(pipe)	(usb_pipespeed(pipe) == USB_SPEED_LOW)

#define __create_pipe(dev, endpoint) (((dev)->devnum << 8) | ((endpoint) << 15) | (((uint32_t)(dev)->slow) << 26))

/* Create various pipes... */
#define usb_sndctrlpipe(dev, endpoint)	\
		((PIPE_CONTROL << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvctrlpipe(dev, endpoint)	\
		((PIPE_CONTROL << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)
#define usb_sndisocpipe(dev, endpoint)	\
		((PIPE_ISOCHRONOUS << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvisocpipe(dev, endpoint)	\
		((PIPE_ISOCHRONOUS << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)
#define usb_sndbulkpipe(dev, endpoint)	\
		((PIPE_BULK << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvbulkpipe(dev, endpoint)	\
		((PIPE_BULK << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)
#define usb_sndintpipe(dev, endpoint)	\
		((PIPE_INTERRUPT << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvintpipe(dev, endpoint)	\
		((PIPE_INTERRUPT << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)

#define default_pipe(dev)				((dev)->speed << 26)
#define usb_snddefctrl(dev)				\
		((PIPE_CONTROL << 30) | default_pipe(dev) | USB_DIR_OUT)
#define usb_rcvdefctrl(dev)				\
		((PIPE_CONTROL << 30) | default_pipe(dev) | USB_DIR_IN)
/*-------------------------------------------------------------------------*/

// CRC
uint8_t usb_crc5(uint16_t bit11);
uint16_t usb_crc16(uint8_t *data, uint16_t len);


#endif // __USB_COMMON_H__
