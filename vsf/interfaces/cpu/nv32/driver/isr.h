#include "interfaces.h"

#ifndef IFS_TICKCLK_NOINT
#	undef  VECTOR_015
#	define VECTOR_015			SysTick_Handler
void SysTick_Handler(void);
#endif

#if IFS_USBDIO_EN
#	undef  VECTOR_023
#	define VECTOR_023			USBDIO_DP_Handler
void USBDIO_DP_Handler(void);
#endif
