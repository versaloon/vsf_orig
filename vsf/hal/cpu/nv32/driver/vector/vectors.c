/******************************************************************************
*
* @brief 提供NV32的中断框架.
*
*******************************************************************************/

#include "vectors.h"
#include "../isr.h"
#include "NV32.h"

/******************************************************************************
* 中断向量表
******************************************************************************/
typedef void (*vector_entry)(void);

#ifdef KEIL
const vector_entry  __vector_table[] __attribute__((at(0x00))) =
#elif (defined(__GNUC__))
//void (* const __vector_table[])() __attribute__ ((section(".vectortable"))) = 
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = 
#else
#pragma location = ".intvec"
__root const vector_entry  __vector_table[] = //@ ".intvec" =
#endif
{
   VECTOR_000,           /* Initial SP           */
   VECTOR_001,           /* Initial PC           */
   VECTOR_002,
   VECTOR_003,
   VECTOR_004,
   VECTOR_005,
   VECTOR_006,
   VECTOR_007,
   VECTOR_008,
   VECTOR_009,
   VECTOR_010,
   VECTOR_011,
   VECTOR_012,
   VECTOR_013,
   VECTOR_014,
   VECTOR_015,
   VECTOR_016,
   VECTOR_017,
   VECTOR_018,
   VECTOR_019,
   VECTOR_020,
   VECTOR_021,
   VECTOR_022,
   VECTOR_023,
   VECTOR_024,
   VECTOR_025,
   VECTOR_026,
   VECTOR_027,
   VECTOR_028,
   VECTOR_029,
   VECTOR_030,
   VECTOR_031,
   VECTOR_032,
   VECTOR_033,
   VECTOR_034,
   VECTOR_035,
   VECTOR_036,
   VECTOR_037,
   VECTOR_038,
   VECTOR_039,
   VECTOR_040,
   VECTOR_041,
   VECTOR_042,
   VECTOR_043,
   VECTOR_044,
   VECTOR_045,
   VECTOR_046,
   VECTOR_047          // END of real vector table
};
// VECTOR_TABLE end
#ifndef KEIL
#ifndef USE_BOOTLOADER
#ifdef KEIL
const uint32_t  __flash_config[] __attribute__((at(0x400))) =
#elif (defined(__GNUC__))
const uint32_t __flash_config[] __attribute__ ((section(".cfmconfig"))) = 
#else
#pragma location = 0x400
__root const uint32_t  __flash_config[] = //@ ".intvec" =
#endif
{
   CONFIG_1,
   CONFIG_2,
   CONFIG_3,
   CONFIG_4,
};	
#endif
#endif
/******************************************************************************
* default_isr(void)
*
* 定义默认中断.
*
* In:  n/a
* Out: n/a
******************************************************************************/
void default_isr(void)
{
   #define VECTORNUM                     (*(volatile uint32_t*)(0xE000ED04))

  //printf("\n****default_isr entered on vector %d*****\r\n\n",VECTORNUM);
   while (1);
}
/******************************************************************************/
/* Generic interrupt handler for a GPIO interrupt connected to an 
 * abort switch. 
 * NOTE: For true abort operation this handler should be modified
 * to jump to the main process instead of executing a return.
 */
void abort_isr(void)
{  
   //printf("\n****Abort button interrupt****\n\n");
   return;
}


/******************************************************************************/
/* Exception frame without floating-point storage 
 * hard fault handler in C,
 * with stack frame location as input parameter
 */
void 
hard_fault_handler_c(unsigned int * hardfault_args)
{
    /*  
    unsigned int stacked_r0;
    unsigned int stacked_r1;
    unsigned int stacked_r2;
    unsigned int stacked_r3;
    unsigned int stacked_r12;
    unsigned int stacked_lr;
    unsigned int stacked_pc;
    unsigned int stacked_psr;
    
    //Exception stack frame
    stacked_r0 = ((unsigned long) hardfault_args[0]);
    stacked_r1 = ((unsigned long) hardfault_args[1]);
    stacked_r2 = ((unsigned long) hardfault_args[2]);
    stacked_r3 = ((unsigned long) hardfault_args[3]);
    
    stacked_r12 = ((unsigned long) hardfault_args[4]);
    stacked_lr = ((unsigned long) hardfault_args[5]);
    stacked_pc = ((unsigned long) hardfault_args[6]);
    stacked_psr = ((unsigned long) hardfault_args[7]);

    printf ("[Hard fault handler]\n");
    printf ("R0 = %x\n", stacked_r0);
    printf ("R1 = %x\n", stacked_r1);
    printf ("R2 = %x\n", stacked_r2);
    printf ("R3 = %x\n", stacked_r3);
    printf ("R12 = %x\n", stacked_r12);
    printf ("LR = %x\n", stacked_lr);
    printf ("PC = %x\n", stacked_pc);
    printf ("PSR = %x\n", stacked_psr);
    printf ("BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));
    printf ("CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));
    printf ("HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));
    printf ("DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));
    printf ("AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));
   */ 
    //for(;;)
    //;/*infinite loop*/
} 


/* End of "vectors.c" */
