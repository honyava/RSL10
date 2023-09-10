; -----------------------------------------------------------------------------
; Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a 
; ON Semiconductor), All Rights Reserved
;
; This code is the property of ON Semiconductor and may not be redistributed
; in any form without prior written permission from ON Semiconductor.
; The terms of use and warranty for this code are covered by contractual
; agreements between ON Semiconductor and the licensee.
;
; This is Reusable Code.
;
; -----------------------------------------------------------------------------
; startup.s
; - CMSIS Cortex-M3 Core Device start-up file for the ARM Cortex-M3 processor
;   (standard ARM development tools)
; -----------------------------------------------------------------------------
; $Revision: 1.9 $
; $Date: 2019/02/06 21:21:50 $
; -----------------------------------------------------------------------------

; -----------------------------------------------------------------------------
; Stack Configuration
; -----------------------------------------------------------------------------
Stack_Size      EQU     0x00000400

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
			
; -----------------------------------------------------------------------------
; Heap Configuration
; -----------------------------------------------------------------------------
Heap_Size       EQU     0x00000400

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

; -----------------------------------------------------------------------------
; Vector Table 
; - Mapped to address 0 at Reset
; -----------------------------------------------------------------------------

                AREA    RESET, CODE, READONLY
				IMPORT  ||Image$$ARM_LIB_STACK$$ZI$$Limit||
                EXPORT  ISR_Vector_Table
                EXPORT  ISR_Vector_Table_End
                EXPORT  ISR_Vector_Table_Size

ISR_Vector_Table
                DCD     ||Image$$ARM_LIB_STACK$$ZI$$Limit||                     ; Top of Stack
                DCD     Reset_Handler               ;  1 Reset Handler
                DCD     NMI_Handler                 ;  2 NMI Handler
                DCD     HardFault_Handler           ;  3 Hard Fault Handler
                DCD     MemManage_Handler           ;  4 MPU Fault Handler
                DCD     BusFault_Handler            ;  5 Bus Fault Handler
                DCD     UsageFault_Handler          ;  6 Usage Fault Handler
                DCD     0                           ;  7 Reserved
                DCD     0                           ;  8 Reserved
                DCD     0                           ;  9 Reserved
                DCD     0                           ; 10 Reserved
                DCD     SVC_Handler                 ; 11 SVCall Handler
                DCD     DebugMon_Handler            ; 12 Debug Monitor Handler
                DCD     0                           ; 13 Reserved
                DCD     PendSV_Handler              ; 14 PendSV Handler
                DCD     SysTick_Handler             ; 15 SysTick Handler

                ; External Interrupts
                DCD    WAKEUP_IRQHandler            ; 16 Wakeup Handler */
                DCD    RTC_ALARM_IRQHandler         ; 17 RTC Alarm Handler 
                DCD    RTC_CLOCK_IRQHandler         ; 18 RTC Clock Handler 
                DCD    ADC_BATMON_IRQHandler        ; 19 ADC / Battery Monitor Handler 
                DCD    TIMER0_IRQHandler            ; 20 General-Purpose Timer 0 Handler 
                DCD    TIMER1_IRQHandler            ; 21 General-Purpose Timer 1 Handler 
                DCD    TIMER2_IRQHandler            ; 22 General-Purpose Timer 2 Handler 
                DCD    TIMER3_IRQHandler            ; 23 General-Purpose Timer 3 Handler 
                DCD    DMA0_IRQHandler              ; 24 DMA Channel 0 Handler 
                DCD    DMA1_IRQHandler              ; 25 DMA Channel 1 Handler 
                DCD    DMA2_IRQHandler              ; 26 DMA Channel 2 Handler 
                DCD    DMA3_IRQHandler              ; 27 DMA Channel 3 Handler 
                DCD    DMA4_IRQHandler              ; 28 DMA Channel 4 Handler 
                DCD    DMA5_IRQHandler              ; 29 DMA Channel 5 Handler 
                DCD    DMA6_IRQHandler              ; 30 DMA Channel 6 Handler 
                DCD    DMA7_IRQHandler              ; 31 DMA Channel 7 Handler 
                DCD    DIO0_IRQHandler              ; 32 DIO Interrupt 0 Handler
                DCD    DIO1_IRQHandler              ; 33 DIO Interrupt 1 Handler
                DCD    DIO2_IRQHandler              ; 34 DIO Interrupt 2 Handler
                DCD    DIO3_IRQHandler              ; 35 DIO Interrupt 3 Handler
                DCD    WATCHDOG_IRQHandler          ; 36 Watchdog Timer Handler
                DCD    SPI0_RX_IRQHandler           ; 37 SPI0 Receive Buffer Full Handler 
                DCD    SPI0_TX_IRQHandler           ; 38 SPI0 Transmit Buffer Empty Handler 
                DCD    SPI0_ERROR_IRQHandler        ; 39 SPI0 Error Handler 
                DCD    SPI1_RX_IRQHandler           ; 40 SPI1 Receive Buffer Full Handler 
                DCD    SPI1_TX_IRQHandler           ; 41 SPI1 Transmit Buffer Empty Handler 
                DCD    SPI1_ERROR_IRQHandler        ; 42 SPI1 Error Handler 
                DCD    I2C_IRQHandler               ; 43 I2C Handler
                DCD    UART_RX_IRQHandler           ; 44 UART0 Receive Buffer Full Handler
                DCD    UART_TX_IRQHandler           ; 45 UART0 Transmit Buffer Empty Handler 
                DCD    UART_ERROR_IRQHandler        ; 46 UART0 Error Handler 
                DCD    DMIC_OUT_OD_IN_IRQHandler    ; 47 DMIC and Output Driver Data Ready Handler
                DCD    DMIC_OD_ERROR_IRQHandler     ; 48 DMIC Overrun and Output Driver Underrun Detect Handler
                DCD    PCM_RX_IRQHandler            ; 49 PCM Receive Buffer Full Handler
                DCD    PCM_TX_IRQHandler            ; 50 PCM Transmit Buffer Empty Handler
                DCD    PCM_ERROR_IRQHandler         ; 51 PCM Error Handler
                DCD    DSP0_IRQHandler              ; 52 DSP Event 0 Handler 
                DCD    DSP1_IRQHandler              ; 53 DSP Event 1 Handler 
                DCD    DSP2_IRQHandler              ; 54 DSP Event 2 Handler 
                DCD    DSP3_IRQHandler              ; 55 DSP Event 3 Handler 
                DCD    DSP4_IRQHandler              ; 56 DSP Event 4 Handler 
                DCD    DSP5_IRQHandler              ; 57 DSP Event 5 Handler 
                DCD    DSP6_IRQHandler              ; 58 DSP Event 6 Handler 
                DCD    DSP7_IRQHandler              ; 59 DSP Event 7 Handler 
                DCD    BLE_CSCNT_IRQHandler         ; 60 BLE 625us time reference interrupt 
                DCD    BLE_SLP_IRQHandler           ; 61 BLE sleep mode interrupt 
                DCD    BLE_RX_IRQHandler            ; 62 BLE received packet interrupt 
                DCD    BLE_EVENT_IRQHandler         ; 63 BLE event interrupt 
                DCD    BLE_CRYPT_IRQHandler         ; 64 BLE AES ciphering complete interrupt 
                DCD    BLE_ERROR_IRQHandler         ; 65 BLE baseband error interrupt 
                DCD    BLE_GROSSTGTIM_IRQHandler    ; 66 BLE gross timer interrupt 
                DCD    BLE_FINETGTIM_IRQHandler     ; 67 BLE fine timer interrupt 
                DCD    BLE_SW_IRQHandler            ; 68 BLE SW triggered inerrupt 
                DCD    BLE_COEX_RX_TX_IRQHandler    ; 69 Coexistance BLE start/stop RX or TX interrupt
                DCD    BLE_COEX_IN_PROCESS_IRQHandler ; 70 Coexistance BLE in process interrupt
                DCD    RF_TX_IRQHandler             ; 71 BLE transmit interrupt 
                DCD    RF_RXSTOP_IRQHandler         ; 72 BLE receive stop interrupt 
                DCD    RF_IRQ_RECEIVED_IRQHandler   ; 73 BLE received packet interrupt 
                DCD    RF_SYNC_IRQHandler           ; 74 BLE received sync word interrupt 
                DCD    RF_TXFIFO_IRQHandler         ; 75 TX FIFO near underflow detect interrupt 
                DCD    RF_RXFIFO_IRQHandler         ; 76 RX FIFO near overflow detect interrupt 
                DCD    ASRC_ERROR_IRQHandler        ; 77 ASRC error interrupt 
                DCD    ASRC_IN_IRQHandler           ; 78 ASRC data input interrupt 
                DCD    ASRC_OUT_IRQHandler          ; 79 ASRC data output interrupt 
                DCD    AUDIOSINK_PHASE_IRQHandler   ; 80 Audio sink clock phase interrupt 
                DCD    AUDIOSINK_PERIOD_IRQHandler  ; 81 Audio sink clock period interrupt 
                DCD    CLKDET_IRQHandler            ; 82 Clock detection interrupt
                DCD    FLASH_COPY_IRQHandler        ; 83 Flash copy interrupt 
                DCD    FLASH_ECC_IRQHandler         ; 84 Flash ECC event interrupt 
                DCD    MEMORY_ERROR_IRQHandler      ; 85 Memory error event interrupt
                DCD    BLE_AUDIO0_IRQHandler        ; 86 Audio over BLE channel 0 interrupt
                DCD    BLE_AUDIO1_IRQHandler        ; 86 Audio over BLE channel 1 interrupt
                DCD    BLE_AUDIO2_IRQHandler        ; 86 Audio over BLE channel 2 interrupt

ISR_Vector_Table_End

ISR_Vector_Table_Size  EQU     ISR_Vector_Table_End - ISR_Vector_Table

; -----------------------------------------------------------------------------
; Reset Handler
; -----------------------------------------------------------------------------
Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                MOV R0, SP
                BFC R0, #2, #1
                MOV SP, R0
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


; -----------------------------------------------------------------------------
; Place-holder Exception Handlers
; - Weak definitions
; - If not modified or replaced, these handlers implement infinite loops
; -----------------------------------------------------------------------------
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

PendSV_Handler\
                PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP

SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

; -----------------------------------------------------------------------------
; Default Interrupt Request Handlers for External Interrupts
; -----------------------------------------------------------------------------
Default_Handler PROC

                EXPORT WAKEUP_IRQHandler            [WEAK]
                EXPORT RTC_ALARM_IRQHandler         [WEAK]
                EXPORT RTC_CLOCK_IRQHandler         [WEAK]
                EXPORT ADC_BATMON_IRQHandler        [WEAK]
                EXPORT TIMER0_IRQHandler            [WEAK]
                EXPORT TIMER1_IRQHandler            [WEAK]
                EXPORT TIMER2_IRQHandler            [WEAK]
                EXPORT TIMER3_IRQHandler            [WEAK]
                EXPORT DMA0_IRQHandler              [WEAK]
                EXPORT DMA1_IRQHandler              [WEAK]
                EXPORT DMA2_IRQHandler              [WEAK]
                EXPORT DMA3_IRQHandler              [WEAK]
                EXPORT DMA4_IRQHandler              [WEAK]
                EXPORT DMA5_IRQHandler              [WEAK]
                EXPORT DMA6_IRQHandler              [WEAK]
                EXPORT DMA7_IRQHandler              [WEAK]
                EXPORT DIO0_IRQHandler              [WEAK]
                EXPORT DIO1_IRQHandler              [WEAK]
                EXPORT DIO2_IRQHandler              [WEAK]
                EXPORT DIO3_IRQHandler              [WEAK]
                EXPORT WATCHDOG_IRQHandler          [WEAK]
                EXPORT SPI0_RX_IRQHandler           [WEAK]
                EXPORT SPI0_TX_IRQHandler           [WEAK]
                EXPORT SPI0_ERROR_IRQHandler        [WEAK]
                EXPORT SPI1_RX_IRQHandler           [WEAK]
                EXPORT SPI1_TX_IRQHandler           [WEAK]
                EXPORT SPI1_ERROR_IRQHandler        [WEAK]
                EXPORT I2C_IRQHandler               [WEAK]
                EXPORT UART_RX_IRQHandler           [WEAK]
                EXPORT UART_TX_IRQHandler           [WEAK]
                EXPORT UART_ERROR_IRQHandler        [WEAK]
                EXPORT DMIC_OUT_OD_IN_IRQHandler    [WEAK]
                EXPORT DMIC_OD_ERROR_IRQHandler     [WEAK]
                EXPORT PCM_RX_IRQHandler            [WEAK]
                EXPORT PCM_TX_IRQHandler            [WEAK]
                EXPORT PCM_ERROR_IRQHandler         [WEAK]
                EXPORT DSP0_IRQHandler              [WEAK]
                EXPORT DSP1_IRQHandler              [WEAK]
                EXPORT DSP2_IRQHandler              [WEAK]
                EXPORT DSP3_IRQHandler              [WEAK]
                EXPORT DSP4_IRQHandler              [WEAK]
                EXPORT DSP5_IRQHandler              [WEAK]
                EXPORT DSP6_IRQHandler              [WEAK]
                EXPORT DSP7_IRQHandler              [WEAK]
                EXPORT BLE_CSCNT_IRQHandler         [WEAK]
                EXPORT BLE_SLP_IRQHandler           [WEAK]
                EXPORT BLE_RX_IRQHandler            [WEAK]
                EXPORT BLE_EVENT_IRQHandler         [WEAK]
                EXPORT BLE_CRYPT_IRQHandler         [WEAK]
                EXPORT BLE_ERROR_IRQHandler         [WEAK]
                EXPORT BLE_GROSSTGTIM_IRQHandler    [WEAK]
                EXPORT BLE_FINETGTIM_IRQHandler     [WEAK]
                EXPORT BLE_SW_IRQHandler            [WEAK]
                EXPORT BLE_COEX_RX_TX_IRQHandler    [WEAK]
                EXPORT BLE_COEX_IN_PROCESS_IRQHandler   [WEAK]
                EXPORT RF_TX_IRQHandler             [WEAK]
                EXPORT RF_RXSTOP_IRQHandler         [WEAK]
                EXPORT RF_IRQ_RECEIVED_IRQHandler   [WEAK]
                EXPORT RF_SYNC_IRQHandler           [WEAK]
                EXPORT RF_TXFIFO_IRQHandler         [WEAK]
                EXPORT RF_RXFIFO_IRQHandler         [WEAK]
                EXPORT ASRC_ERROR_IRQHandler        [WEAK]
                EXPORT ASRC_IN_IRQHandler           [WEAK]
                EXPORT ASRC_OUT_IRQHandler          [WEAK]
                EXPORT AUDIOSINK_PHASE_IRQHandler   [WEAK]
                EXPORT AUDIOSINK_PERIOD_IRQHandler  [WEAK]
                EXPORT CLKDET_IRQHandler            [WEAK]
                EXPORT FLASH_COPY_IRQHandler        [WEAK]
                EXPORT FLASH_ECC_IRQHandler         [WEAK]
                EXPORT MEMORY_ERROR_IRQHandler      [WEAK]
                EXPORT BLE_AUDIO0_IRQHandler        [WEAK]
                EXPORT BLE_AUDIO1_IRQHandler        [WEAK]
                EXPORT BLE_AUDIO2_IRQHandler        [WEAK]
WAKEUP_IRQHandler
RTC_ALARM_IRQHandler
RTC_CLOCK_IRQHandler
ADC_BATMON_IRQHandler
TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
TIMER3_IRQHandler
DMA0_IRQHandler
DMA1_IRQHandler
DMA2_IRQHandler
DMA3_IRQHandler
DMA4_IRQHandler
DMA5_IRQHandler
DMA6_IRQHandler
DMA7_IRQHandler
DIO0_IRQHandler
DIO1_IRQHandler
DIO2_IRQHandler
DIO3_IRQHandler
WATCHDOG_IRQHandler
SPI0_RX_IRQHandler
SPI0_TX_IRQHandler
SPI0_ERROR_IRQHandler
SPI1_RX_IRQHandler
SPI1_TX_IRQHandler
SPI1_ERROR_IRQHandler
I2C_IRQHandler
UART_RX_IRQHandler
UART_TX_IRQHandler
UART_ERROR_IRQHandler
DMIC_OUT_OD_IN_IRQHandler
DMIC_OD_ERROR_IRQHandler
PCM_RX_IRQHandler
PCM_TX_IRQHandler
PCM_ERROR_IRQHandler
DSP0_IRQHandler
DSP1_IRQHandler
DSP2_IRQHandler
DSP3_IRQHandler
DSP4_IRQHandler
DSP5_IRQHandler
DSP6_IRQHandler
DSP7_IRQHandler
BLE_CSCNT_IRQHandler
BLE_SLP_IRQHandler
BLE_RX_IRQHandler
BLE_EVENT_IRQHandler
BLE_CRYPT_IRQHandler
BLE_ERROR_IRQHandler
BLE_GROSSTGTIM_IRQHandler
BLE_FINETGTIM_IRQHandler
BLE_SW_IRQHandler
BLE_COEX_RX_TX_IRQHandler
BLE_COEX_IN_PROCESS_IRQHandler
RF_TX_IRQHandler
RF_RXSTOP_IRQHandler
RF_IRQ_RECEIVED_IRQHandler
RF_SYNC_IRQHandler
RF_TXFIFO_IRQHandler
RF_RXFIFO_IRQHandler
ASRC_ERROR_IRQHandler
ASRC_IN_IRQHandler
ASRC_OUT_IRQHandler
AUDIOSINK_PHASE_IRQHandler
AUDIOSINK_PERIOD_IRQHandler
CLKDET_IRQHandler
FLASH_COPY_IRQHandler
FLASH_ECC_IRQHandler
MEMORY_ERROR_IRQHandler
BLE_AUDIO0_IRQHandler
BLE_AUDIO1_IRQHandler
BLE_AUDIO2_IRQHandler
                B       .
                ENDP

                ALIGN

; -----------------------------------------------------------------------------
; Setup the User Stack and Heap
; -----------------------------------------------------------------------------

                IF      :DEF:__MICROLIB

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

