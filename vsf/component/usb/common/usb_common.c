// usb_common.c is necessary for shci and sdcd
#include "vsf.h"

static uint16_t usb_crc16_byte(uint16_t crc, uint8_t byte)
{
	uint8_t  i;
	for (i = 0; i < 8; i++)
	{
		if ((crc ^ byte) & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc >>= 1;
		byte >>= 1;
	}
	return crc;
}

uint16_t usb_crc16(uint8_t *data, uint16_t len)
{
	uint16_t result = 0xFFFF;
	while (len--)
		result = usb_crc16_byte(result, *data++);
	return ~result;
}

