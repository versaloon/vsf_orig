#ifndef __COMPILER_H_INCLUDED__
#define __COMPILER_H_INCLUDED__

#include <string.h>
#include <intrinsics.h>

#define __VSF_FUNCNAME__				"cur_function"

#define ROOTFUNC	__root
#define PACKED_HEAD	__packed
#define PACKED_MID	
#define PACKED_TAIL	

#define vsf_gint_t						__istate_t
#define vsf_set_gint(gint)				__set_interrupt_state(gint)
#define vsf_get_gint()					__get_interrupt_state()
#define vsf_enter_critical()			__disable_interrupt()
#define vsf_leave_critical()			__enable_interrupt()

unsigned char * compiler_get_heap(void);
unsigned char * compiler_get_stack(void);

#define compiler_set_pc(reg)			asm("MOV pc, %0" : :"r"(reg))
#define compiler_get_lr(reg)			asm("MOV %0, lr" : "=r"(reg))

#endif	// __COMPILER_H_INCLUDED__
