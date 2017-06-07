#include "vsfhal.h"

#if VSFHAL_USBDIO_EN
#	undef  VECTOR_023
#	define VECTOR_023			USBDIO_DP_Handler
void USBDIO_DP_Handler(void);
#endif
