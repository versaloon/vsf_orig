;/******************************************************************************
; * @file     startup_NUC505Series.s
; * @version  V1.00
; * $Revision: 9 $
; * $Date: 14/09/23 5:06p $
; * @brief    CMSIS ARM Cortex-M4 Core Device Startup File
; *
; * @note
; * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
;*****************************************************************************/

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        ;EXTERN  SystemInit
        PUBLIC  __vector_table
        PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA

__iar_init$$done                    ; Anything reachable from the program entry label __vector_table is considered
                                    ; needed for initialization unless reached via a section fragment with a label
                                    ; starting with __iar_init$$done. This makes interrupt functions and all reachable
                                    ; from them can be located on RAM through copy initialization.
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     MemManage_Handler
        DCD     BusFault_Handler
        DCD     UsageFault_Handler
__vector_table_0x1c
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     DebugMon_Handler
        DCD     0

#ifdef __uCOS
        DCD     OS_CPU_PendSVHandler
#else
        DCD     PendSV_Handler
#endif

        DCD     SysTick_Handler

        ; External Interrupts
        DCD     PWR_IRQHandler            ;  0: Power On Interrupt
        DCD     WDT_IRQHandler            ;  1: Watch Dog Interrupt
		DCD     APU_IRQHandler		  	  ;  2: Audio Process Unit Interrupt
        DCD     I2S_IRQHandler            ;  3: GPIO Port F
		DCD     EINT0_IRQHandler		  ;  4: External GPIO Group 0
		DCD     EINT1_IRQHandler		  ;  5: External GPIO Group 1
		DCD     EINT2_IRQHandler	 	  ;  6: External GPIO Group 2
		DCD     EINT3_IRQHandler	      ;  7: External GPIO Group 3
        DCD     SPIM_IRQHandler           ;  8: SPIM Interrupt
        DCD     USBD_IRQHandler           ;  9: USB Device 2.0 Interrupt
        DCD     TMR0_IRQHandler           ; 10: Timer 0 Interrupt
        DCD     TMR1_IRQHandler           ; 11: Timer 1 Interrupt
        DCD     TMR2_IRQHandler           ; 12: Timer 2 Interrupt
        DCD     TMR3_IRQHandler           ; 13: Timer 3 Interrupt
        DCD     SDH_IRQHandler            ; 14: SD Host Interrupt
        DCD     PWM0_IRQHandler           ; 15: PWM0 Interrupt
        DCD     PWM1_IRQHandler           ; 16: PWM1 Interrupt
        DCD     PWM2_IRQHandler           ; 17: PWM2 Interrupt
        DCD     PWM3_IRQHandler           ; 18: PWM2 Interrupt
        DCD     RTC_IRQHandler            ; 19: Real Time Clock Interrupt
        DCD     SPI0_IRQHandler           ; 20: SPI0 Interrupt
        DCD     I2C1_IRQHandler           ; 21: I2C1 Interrupt
        DCD     I2C0_IRQHandler           ; 22: I2C0 Interrupt
        DCD     UART0_IRQHandler          ; 23: UART0 Interrupt
        DCD     UART1_IRQHandler          ; 24: UART1 Interrupt
        DCD     ADC_IRQHandler            ; 25: ADC  Interrupt
        DCD     WWDT_IRQHandler           ; 26: Window Watch Dog Timer Interrupt
        DCD     USBH_IRQHandler           ; 27: USB Host 1.1 Interrupt
        DCD     UART2_IRQHandler          ; 28: UART2 Interrupt
        DCD     LVD_IRQHandler            ; 29: Low Voltage Detection Interrupt
        DCD     SPI1_IRQHandler           ; 30: SPI1 Interrupt
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
Reset_Handler
;       LDR     R0, =SystemInit
;       BLX     R0

        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
DebugMon_Handler
        B DebugMon_Handler

#ifdef __uCOS
        PUBWEAK OS_CPU_PendSVHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
OS_CPU_PendSVHandler
        B OS_CPU_PendSVHandler
#else
        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B PendSV_Handler
#endif

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B SysTick_Handler

		PUBWEAK  PWR_IRQHandler
		PUBWEAK  WDT_IRQHandler
		PUBWEAK  APU_IRQHandler
		PUBWEAK  I2S_IRQHandler
		PUBWEAK  EINT0_IRQHandler
		PUBWEAK  EINT1_IRQHandler
		PUBWEAK  EINT2_IRQHandler
		PUBWEAK  EINT3_IRQHandler
		PUBWEAK  SPIM_IRQHandler
		PUBWEAK  USBD_IRQHandler
		PUBWEAK  TMR0_IRQHandler
		PUBWEAK  TMR1_IRQHandler
		PUBWEAK  TMR2_IRQHandler
		PUBWEAK  TMR3_IRQHandler
		PUBWEAK  SDH_IRQHandler
		PUBWEAK  PWM0_IRQHandler
		PUBWEAK  PWM1_IRQHandler
		PUBWEAK  PWM2_IRQHandler
		PUBWEAK  PWM3_IRQHandler
		PUBWEAK  RTC_IRQHandler
		PUBWEAK  SPI0_IRQHandler
		PUBWEAK  I2C1_IRQHandler
		PUBWEAK  I2C0_IRQHandler
		PUBWEAK  UART0_IRQHandler
		PUBWEAK  UART1_IRQHandler
		PUBWEAK  ADC_IRQHandler
		PUBWEAK  WWDT_IRQHandler
		PUBWEAK  USBH_IRQHandler
		PUBWEAK  UART2_IRQHandler
		PUBWEAK  LVD_IRQHandler
		PUBWEAK  SPI1_IRQHandler
		PUBWEAK  Default_Handler


PWR_IRQHandler
WDT_IRQHandler
APU_IRQHandler
I2S_IRQHandler
EINT0_IRQHandler
EINT1_IRQHandler
EINT2_IRQHandler
EINT3_IRQHandler
SPIM_IRQHandler
USBD_IRQHandler
TMR0_IRQHandler
TMR1_IRQHandler
TMR2_IRQHandler
TMR3_IRQHandler
SDH_IRQHandler
PWM0_IRQHandler
PWM1_IRQHandler
PWM2_IRQHandler
PWM3_IRQHandler
RTC_IRQHandler
SPI0_IRQHandler
I2C1_IRQHandler
I2C0_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
ADC_IRQHandler
WWDT_IRQHandler
USBH_IRQHandler
UART2_IRQHandler
LVD_IRQHandler
SPI1_IRQHandler

Default_Handler
        B Default_Handler




        END
;/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
