;/******************************************************************************
; * @file     startup_NUC505Series.s
; * @version  V1.00
; * $Revision: 1 $
; * $Date: 13/10/28 2:20p $ 
; * @brief    CMSIS Cortex-M4 Core Device Startup File for NUC505 Series MCU
; *
; * @note
; * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
;*****************************************************************************/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000800

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000100

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
            IF      :DEF:__uCOS
                DCD     OS_CPU_PendSVHandler      ; PendSV Handler
            ELSE
                DCD     PendSV_Handler            ; PendSV Handler
            ENDIF
                DCD     SysTick_Handler           ; SysTick Handler

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

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
    IF      :DEF:__uCOS
OS_CPU_PendSVHandler\
                PROC
                EXPORT  OS_CPU_PendSVHandler      [WEAK]
                B       .
                ENDP
    ELSE
PendSV_Handler\
                PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
    ENDIF
SysTick_Handler\
                PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC

		EXPORT  PWR_IRQHandler			[WEAK]
		EXPORT  WDT_IRQHandler          [WEAK]
		EXPORT  APU_IRQHandler		  	[WEAK]
		EXPORT  I2S_IRQHandler          [WEAK]
		EXPORT  EINT0_IRQHandler	  	[WEAK]
		EXPORT  EINT1_IRQHandler	  	[WEAK]
		EXPORT  EINT2_IRQHandler	  	[WEAK]
		EXPORT  EINT3_IRQHandler	  	[WEAK]
		EXPORT  SPIM_IRQHandler      	[WEAK]
		EXPORT  USBD_IRQHandler         [WEAK]
		EXPORT  TMR0_IRQHandler         [WEAK]
		EXPORT  TMR1_IRQHandler         [WEAK]
		EXPORT  TMR2_IRQHandler         [WEAK]
		EXPORT  TMR3_IRQHandler         [WEAK]
		EXPORT  SDH_IRQHandler          [WEAK]
		EXPORT  PWM0_IRQHandler         [WEAK]
		EXPORT  PWM1_IRQHandler         [WEAK]
		EXPORT  PWM2_IRQHandler         [WEAK]
		EXPORT  PWM3_IRQHandler         [WEAK]
		EXPORT  RTC_IRQHandler          [WEAK]
		EXPORT  SPI0_IRQHandler         [WEAK]
		EXPORT  I2C1_IRQHandler         [WEAK]
		EXPORT  I2C0_IRQHandler         [WEAK]
		EXPORT  UART0_IRQHandler        [WEAK]
		EXPORT  UART1_IRQHandler        [WEAK]
		EXPORT  ADC_IRQHandler          [WEAK]
		EXPORT  WWDT_IRQHandler         [WEAK]
		EXPORT  USBH_IRQHandler         [WEAK]
		EXPORT  UART2_IRQHandler        [WEAK]
		EXPORT  LVD_IRQHandler          [WEAK]
		EXPORT  SPI1_IRQHandler         [WEAK]
		EXPORT  Default_Handler		  	[WEAK]

Default__IRQHandler
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

                B       .
                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
;/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
