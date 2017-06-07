;/**
;  ******************************************************************************
;  * @file    startup_gd32f1x0.s
;  * @author  MCU SD
;  * @version V1.0.1   
;  * @date    6-Sep-2014
;  * @brief   GD32F1x0 startup code.
;  ******************************************************************************
;  */

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler                  ; Reset Handler

        DCD     NMI_Handler                    ; NMI Handler
        DCD     HardFault_Handler              ; Hard Fault Handler
        DCD     MemManage_Handler              ; MPU Fault Handler//by denvice
        DCD     BusFault_Handler               ; Bus Fault Handler//by denvice
        DCD     UsageFault_Handler             ; Usage Fault Handler//by denvice
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     SVC_Handler                    ; SVCall Handler
        DCD     DebugMon_Handler              ; Debug Monitor Handler//by denvice
        DCD     0                              ; Reserved
        DCD     PendSV_Handler                 ; PendSV Handler
        DCD     SysTick_Handler                ; SysTick Handler

        ; External Interrupts
        DCD     WWDG_IRQHandler                ; Window Watchdog
        DCD     LVD_IRQHandler                 ; LVD through EXTI Line detect
        DCD     RTC_IRQHandler                 ; RTC through EXTI Line
        DCD     FMC_IRQHandler                 ; FMC
        DCD     RCC_IRQHandler                 ; RCC
        DCD     EXTI0_1_IRQHandler             ; EXTI Line 0 and 1
        DCD     EXTI2_3_IRQHandler             ; EXTI Line 2 and 3
        DCD     EXTI4_15_IRQHandler            ; EXTI Line 4 to 15
        DCD     TS_IRQHandler                  ; TS
        DCD     DMA1_Channel1_IRQHandler       ; DMA1 Channel 1
        DCD     DMA1_Channel2_3_IRQHandler     ; DMA1 Channel 2 and Channel 3
        DCD     DMA1_Channel4_5_IRQHandler     ; DMA1 Channel 4 and Channel 5
        DCD     ADC1_CMP_IRQHandler           ; ADC1, CMP1 and CMP2 
        DCD     TIM1_BRK_UP_TRG_COM_IRQHandler ; TIM1 Break, Update, Trigger and Commutation
        DCD     TIM1_CC_IRQHandler             ; TIM1 Capture Compare
        DCD     TIM2_IRQHandler                ; TIM2
        DCD     TIM3_IRQHandler                ; TIM3
        DCD     TIM6_DAC_IRQHandler            ; TIM6 and DAC
        DCD     0                              ; Reserved
        DCD     TIM14_IRQHandler               ; TIM14
        DCD     TIM15_IRQHandler               ; TIM15
        DCD     TIM16_IRQHandler               ; TIM16
        DCD     TIM17_IRQHandler               ; TIM17
        DCD     I2C1_EV_IRQHandler             ; I2C1 Event by denvice
        DCD     I2C2_EV_IRQHandler             ; I2C2 Event by denvice
        DCD     SPI1_IRQHandler                ; SPI1
        DCD     SPI2_IRQHandler                ; SPI2
        DCD     USART1_IRQHandler              ; USART1
        DCD     USART2_IRQHandler              ; USART2
        DCD     0                              ; Reserved
        DCD     CEC_IRQHandler                 ; CEC
        DCD     0                              ; Reserved
        
        DCD     I2C1_ER_IRQHandler             ; I2C1 Error  by denvice
        DCD     0                              ; Reserved    by denvice
        DCD     I2C2_ER_IRQHandler             ; I2C2 Error  by denvice
        DCD     I2C3_EV_IRQHandler             ; I2C3 Event  by denvice
        DCD     I2C3_ER_IRQHandler             ; I2C3 Error  by denvice
        DCD     USB_LP_IRQHandler              ; USB Low  Priority by denvice
        DCD     USB_HP_IRQHandler              ; USB High Priority by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     USBWakeUp_IRQHandler           ; USB Wakeup from suspend  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     DMA1_Channel6_7_IRQHandler     ; DMA1 Channel 6 and Channel 7  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     0                              ; Reserved  by denvice
        DCD     SPI3_IRQHandler                ; SPI3     by denvice
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:NOROOT:REORDER(2)
Reset_Handler
        LDR     R0, =__iar_program_start
        BX      R0
        
        PUBWEAK NMI_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
NMI_Handler
        B NMI_Handler
        
        
        PUBWEAK HardFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
HardFault_Handler
        B HardFault_Handler
       
       
        PUBWEAK MemManage_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
UsageFault_Handler
        B UsageFault_Handler
        
        PUBWEAK SVC_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SVC_Handler
        B SVC_Handler
       
         PUBWEAK DebugMon_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
DebugMon_Handler
        B DebugMon_Handler
        
        
        PUBWEAK PendSV_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
PendSV_Handler
        B PendSV_Handler
        
        
        PUBWEAK SysTick_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SysTick_Handler
        B SysTick_Handler
        
        
        PUBWEAK WWDG_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
WWDG_IRQHandler
        B WWDG_IRQHandler
        
                
        PUBWEAK LVD_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LVD_IRQHandler
        B LVD_IRQHandler
        
                
        PUBWEAK RTC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTC_IRQHandler
        B RTC_IRQHandler
        
                
        PUBWEAK FMC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FMC_IRQHandler
        B FMC_IRQHandler
        
                
        PUBWEAK RCC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RCC_IRQHandler
        B RCC_IRQHandler
        
                
        PUBWEAK EXTI0_1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI0_1_IRQHandler
        B EXTI0_1_IRQHandler
        
                
        PUBWEAK EXTI2_3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI2_3_IRQHandler
        B EXTI2_3_IRQHandler
        
                
        PUBWEAK EXTI4_15_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI4_15_IRQHandler
        B EXTI4_15_IRQHandler
        
                
        PUBWEAK TS_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TS_IRQHandler
        B TS_IRQHandler
        
                
        PUBWEAK DMA1_Channel1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel1_IRQHandler
        B DMA1_Channel1_IRQHandler
        
                
        PUBWEAK DMA1_Channel2_3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel2_3_IRQHandler
        B DMA1_Channel2_3_IRQHandler
        
                
        PUBWEAK DMA1_Channel4_5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel4_5_IRQHandler
        B DMA1_Channel4_5_IRQHandler
        
        PUBWEAK DMA1_Channel6_7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel6_7_IRQHandler
        B DMA1_Channel6_7_IRQHandler
                
        PUBWEAK ADC1_CMP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ADC1_CMP_IRQHandler
        B ADC1_CMP_IRQHandler
        
                 
        PUBWEAK TIM1_BRK_UP_TRG_COM_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_BRK_UP_TRG_COM_IRQHandler
        B TIM1_BRK_UP_TRG_COM_IRQHandler
        
                
        PUBWEAK TIM1_CC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_CC_IRQHandler
        B TIM1_CC_IRQHandler
        
                
        PUBWEAK TIM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM2_IRQHandler
        B TIM2_IRQHandler
        
                
        PUBWEAK TIM3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM3_IRQHandler
        B TIM3_IRQHandler
        
                
        PUBWEAK TIM6_DAC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM6_DAC_IRQHandler
        B TIM6_DAC_IRQHandler
        
                
        PUBWEAK TIM14_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM14_IRQHandler
        B TIM14_IRQHandler
        
                
        PUBWEAK TIM15_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM15_IRQHandler
        B TIM15_IRQHandler
        
                
        PUBWEAK TIM16_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM16_IRQHandler
        B TIM16_IRQHandler
        
                
        PUBWEAK TIM17_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM17_IRQHandler
        B TIM17_IRQHandler
        
                
       PUBWEAK I2C1_EV_IRQHandler
       SECTION .text:CODE:NOROOT:REORDER(1)
I2C1_EV_IRQHandler
       B I2C1_EV_IRQHandler
        
                
        PUBWEAK I2C2_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C2_EV_IRQHandler
        B I2C2_EV_IRQHandler
                
        PUBWEAK SPI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI1_IRQHandler
        B SPI1_IRQHandler
        
                
        PUBWEAK SPI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI2_IRQHandler
        B SPI2_IRQHandler
        
                
        PUBWEAK USART1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART1_IRQHandler
        B USART1_IRQHandler
        
                
        PUBWEAK USART2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART2_IRQHandler
        B USART2_IRQHandler
        
                
        PUBWEAK CEC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CEC_IRQHandler
        B CEC_IRQHandler
        
          PUBWEAK I2C1_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C1_ER_IRQHandler
        B I2C1_ER_IRQHandler
        
        PUBWEAK I2C2_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C2_ER_IRQHandler
        B I2C2_ER_IRQHandler
        
        PUBWEAK I2C3_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C3_EV_IRQHandler
        B I2C3_EV_IRQHandler

        PUBWEAK I2C3_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C3_ER_IRQHandler
        B I2C3_ER_IRQHandler
        
        PUBWEAK SPI3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI3_IRQHandler
        B SPI3_IRQHandler
        
         PUBWEAK USBWakeUp_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USBWakeUp_IRQHandler
        B USBWakeUp_IRQHandler
        
        PUBWEAK USB_HP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USB_HP_IRQHandler
        B USB_HP_IRQHandler

        PUBWEAK USB_LP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USB_LP_IRQHandler
        B USB_LP_IRQHandler

        END

