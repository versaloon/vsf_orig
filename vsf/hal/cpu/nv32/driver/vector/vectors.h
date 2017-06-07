/******************************************************************************
* @brief provide interrupt vector table for NV32Fxxx
*
*******************************************************************************/


#ifndef __VECTORS_H
#define __VECTORS_H     1

// function prototype for default_isr in vectors.c
void default_isr(void);
void abort_isr(void);


void hard_fault_handler_c(unsigned int * hardfault_args);

/* Interrupt Vector Table Function Pointers */
typedef void pointer(void);

extern void __startup(void);
extern unsigned long __BOOT_STACK_ADDRESS[];
extern unsigned long __initial_sp[];
extern void Reset_Handler( void );
#if (defined(__GNUC__))
extern unsigned long _estack;
extern void __thumb_startup(void);
#define VECTOR_000      (pointer*)&_estack	//          ARM core        Initial Supervisor SP
#define VECTOR_001      __thumb_startup	// 0x0000_0004 1 -          ARM core        Initial Program Counter
//#define VECTOR_001      __startup //__thumb_startup	// 0x0000_0004 1 -          ARM core        Initial Program Counter
#elif (defined(KEIL))
#define VECTOR_000      (pointer*)__initial_sp	//          ARM core        Initial Supervisor SP
#define VECTOR_001      Reset_Handler						// 0x0000_0004 1 -          ARM core        Initial Program Counter
#else
    																						// Address     Vector IRQ   Source module   Source description
#define VECTOR_000      (pointer*)__BOOT_STACK_ADDRESS	//          ARM core        Initial Supervisor SP
void __iar_program_start(void);
#define VECTOR_001      __iar_program_start	// 0x0000_0004 1 -          ARM core        Initial Program Counter
#endif
#define VECTOR_002      default_isr     // 0x0000_0008 2 -          ARM core        Non-maskable Interrupt (NMI)
#define VECTOR_003      default_isr     // 0x0000_000C 3 -          ARM core        Hard Fault
#define VECTOR_004      default_isr     // 0x0000_0010 4 -
#define VECTOR_005      default_isr     // 0x0000_0014 5 -          ARM core         Bus Fault
#define VECTOR_006      default_isr     // 0x0000_0018 6 -          ARM core         Usage Fault
#define VECTOR_007      default_isr     // 0x0000_001C 7 -                           
#define VECTOR_008      default_isr     // 0x0000_0020 8 -                           
#define VECTOR_009      default_isr     // 0x0000_0024 9 -
#define VECTOR_010      default_isr     // 0x0000_0028 10 -
#define VECTOR_011      default_isr     // 0x0000_002C 11 -         ARM core         Supervisor call (SVCall)
#define VECTOR_012      default_isr     // 0x0000_0030 12 -         ARM core         Debug Monitor
#define VECTOR_013      default_isr     // 0x0000_0034 13 -                          
#define VECTOR_014      default_isr     // 0x0000_0038 14 -         ARM core         Pendable request for system service (PendableSrvReq)
#define VECTOR_015      default_isr     // 0x0000_003C 15 -         ARM core         System tick ETMer (SysTick)
#define VECTOR_016      default_isr     // 0x0000_0040 16     0     Reserved DMA     DMA Channel 0 transfer complete
#define VECTOR_017      default_isr     // 0x0000_0044 17     1     Reserved DMA     DMA Channel 1 transfer complete
#define VECTOR_018      default_isr     // 0x0000_0048 18     2     Reserved DMA     DMA Channel 2 transfer complete
#define VECTOR_019      default_isr     // 0x0000_004C 19     3     Reserved DMA     DMA Channel 3 transfer complete
#define VECTOR_020      default_isr     // 0x0000_0050 20     4     Reserved MCM     MCM
#define VECTOR_021      default_isr     // 0x0000_0054 21     5     NVM              ETMRH flash memory command complete,ECC fault
#define VECTOR_022      default_isr     // 0x0000_0058 22     6     PMC              LVD,LVW interrupt
#define VECTOR_023      default_isr     // 0x0000_005C 23     7     LLWU             LLWU/IRQ
#define VECTOR_024      default_isr     // 0x0000_0060 24     8     I2C0             I2C
#define VECTOR_025      default_isr     // 0x0000_0064 25     9     -             --
#define VECTOR_026      default_isr     // 0x0000_0068 26    10     SPI0             SPI0
#define VECTOR_027      default_isr     // 0x0000_006C 27    11     SPI1             SPI1
#define VECTOR_028      default_isr     // 0x0000_0070 28    12     SCI0             UART0
#define VECTOR_029      default_isr     // 0x0000_0074 29    13     SCI1             UART1
#define VECTOR_030      default_isr     // 0x0000_0078 30    14     SCI2             UART2
#define VECTOR_031      default_isr     // 0x0000_007C 31    15     ADC0             ADC conversion complete
#define VECTOR_032      default_isr     // 0x0000_0080 32    16     ACMP0            ACMP0
#define VECTOR_033      default_isr     // 0x0000_0084 33    17     ETM0             FlexETMer0
#define VECTOR_034      default_isr     // 0x0000_0088 34    18     ETM1             FlexETMer1
#define VECTOR_035      default_isr     // 0x0000_008C 35    19     ETM2             FlexETMer2
#define VECTOR_036      default_isr     // 0x0000_0090 36    20     RTC              RTC overflow
#define VECTOR_037      default_isr     // 0x0000_0094 37    21     ACMP1            ACMP1
#define VECTOR_038      default_isr     // 0x0000_0098 38    22     PIT_CH0          PIT_CH0 overflow
#define VECTOR_039      default_isr     // 0x0000_009C 39    23	    PIT_CH1          PIT_CH1 overflow
#define VECTOR_040      default_isr     // 0x0000_00A0 40    24     KBI0             Keyboard0 interrupt
#define VECTOR_041      default_isr     // 0x0000_00A4 41    25     KBI1             Keyboard1 interrupt
#define VECTOR_042      default_isr     // 0x0000_00A8 42    26     Reserved         ---
#define VECTOR_043      default_isr     // 0x0000_00AC 43    27     ICS              ICS loss of lock
#define VECTOR_044      default_isr     // 0x0000_00B0 44    28     WDOG             Watchdog ETMeout
#define VECTOR_045      default_isr     // 0x0000_00B4 45    29     Reserved         
#define VECTOR_046      default_isr     // 0x0000_00B8 46    30     Reserved         
#define VECTOR_047      default_isr     // 0x0000_00BC 47    31     Reserved         // END of real vector table
/********************************************************************************************************************/
#define VECTOR_048      default_isr     // 0x0000_00C0 48    32     Reserved        
#define VECTOR_049      default_isr     // 0x0000_00C4 49    33     Reserved        
#define VECTOR_050      default_isr     // 0x0000_00C8 50    34     Reserved           
#define VECTOR_051      default_isr     // 0x0000_00CC 51    35     Reserved          
#define VECTOR_052      default_isr     // 0x0000_00D0 52    36     Reserved          
#define VECTOR_053      default_isr     // 0x0000_00D4 53    37     Reserved          
#define VECTOR_054      default_isr     // 0x0000_00D8 54    38     Reserved        
#define VECTOR_055      default_isr     // 0x0000_00DC 55    39     Reserved            
#define VECTOR_056      default_isr     // 0x0000_00E0 56    40     Reserved         
#define VECTOR_057      default_isr     // 0x0000_00E4 57    41     Reserved          
#define VECTOR_058      default_isr     // 0x0000_00E8 58    42     Reserved        
#define VECTOR_059      default_isr     // 0x0000_00EC 59    43     Reserved        
#define VECTOR_060      default_isr     // 0x0000_00F0 60    44     Reserved             
#define VECTOR_061      default_isr     // 0x0000_00F4 61    45     Reserved            Single interrupt vector for SCI status sources
#define VECTOR_062      default_isr     // 0x0000_00F8 62    46     Reserved            Single interrupt vector for SCI error sources
#define VECTOR_063      default_isr     // 0x0000_00FC 63    47     Reserved            Single interrupt vector for SCI status sources
#define VECTOR_064      default_isr     // 0x0000_0100 64    48     Reserved            Single interrupt vector for SCI error sources
#define VECTOR_065      default_isr     // 0x0000_0104 65    49     Reserved            Single interrupt vector for SCI status sources
#define VECTOR_066      default_isr     // 0x0000_0108 66    50     Reserved            Single interrupt vector for SCI error sources
#define VECTOR_067      default_isr     // 0x0000_010C 67    51     Reserved            Single interrupt vector for SCI status sources
#define VECTOR_068      default_isr     // 0x0000_0110 68    52     Reserved            Single interrupt vector for SCI error sources
#define VECTOR_069      default_isr     // 0x0000_0114 69    53     Reserved            Single interrupt vector for SCI status sources
#define VECTOR_070      default_isr     // 0x0000_0118 70    54     Reserved            Single interrupt vector for SCI error sources
#define VECTOR_071      default_isr     // 0x0000_011C 71    55     Reserved            Single interrupt vector for SCI status sources
#define VECTOR_072      default_isr     // 0x0000_0120 72    56     Reserved            Single interrupt vector for SCI error sources
#define VECTOR_073      default_isr     // 0x0000_0124 73    57     Reserved
#define VECTOR_074      default_isr     // 0x0000_0128 74    58     Reserved
#define VECTOR_075      default_isr     // 0x0000_012C 75    59     Reserved             
#define VECTOR_076      default_isr     // 0x0000_0130 76    60     Reserved
#define VECTOR_077      default_isr     // 0x0000_0134 77    61     Reserved
#define VECTOR_078      default_isr     // 0x0000_0138 78    62     Reserved 			 Single interrupt vector for all sources
#define VECTOR_079      default_isr     // 0x0000_013C 79    63     Reserved 			 Single interrupt vector for all sources
#define VECTOR_080      default_isr     // 0x0000_0140 80    64     Reserved 			 Single interrupt vector for all sources
#define VECTOR_081      default_isr     // 0x0000_0144 81    65     Reserved
#define VECTOR_082      default_isr     // 0x0000_0148 82    66     Reserved
#define VECTOR_083      default_isr     // 0x0000_014C 83    67
#define VECTOR_084      default_isr     // 0x0000_0150 84    68      
#define VECTOR_085      default_isr     // 0x0000_0154 85    69      
#define VECTOR_086      default_isr     // 0x0000_0158 86    70      
#define VECTOR_087      default_isr     // 0x0000_015C 87    71     
#define VECTOR_088      default_isr     // 0x0000_0160 88    72     
#define VECTOR_089      default_isr     // 0x0000_0164 89    73      
#define VECTOR_090      default_isr     // 0x0000_0168 90    74     
#define VECTOR_091      default_isr     // 0x0000_016C 91    75		 					 
#define VECTOR_092      default_isr     // 0x0000_0170 92    76					 
#define VECTOR_093      default_isr     // 0x0000_0174 93    77					
#define VECTOR_094      default_isr     // 0x0000_0178 94    78					 
#define VECTOR_095      default_isr     // 0x0000_017C 95    79     
#define VECTOR_096      default_isr     // 0x0000_0180 96    80     
#define VECTOR_097      default_isr     // 0x0000_0184 97    81     
#define VECTOR_098      default_isr     // 0x0000_0188 98    82     
#define VECTOR_099      default_isr     // 0x0000_018C 99    83      			

#ifdef USE_BOOTLOADER
#else
#define CONFIG_1		0xffffffff 
#define CONFIG_2		0xffffffff 
#define CONFIG_3		0xffffffff
#define CONFIG_4		0xfffeffff
#endif
#endif /*__VECTORS_H*/

/* End of "vectors.h" */
