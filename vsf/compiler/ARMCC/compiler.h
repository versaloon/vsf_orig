#ifndef __COMPILER_H_INCLUDED__
#define __COMPILER_H_INCLUDED__

#include <string.h>

#define __VSF_FUNCNAME__				__func__

#define ROOTFUNC	
#define PACKED_HEAD	
#define PACKED_MID	__attribute__ ((packed))
#define PACKED_TAIL	

#define vsf_enter_critical()			__disable_irq()
#define vsf_leave_critical()			__enable_irq()

#endif	// __COMPILER_H_INCLUDED__
