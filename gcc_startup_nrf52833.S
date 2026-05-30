.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.global g_pfnVectors
.global Default_Handler

.word _estack

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
.size g_pfnVectors, .-g_pfnVectors
g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler

  /* External Interrupts */
  .word POWER_CLOCK_IRQHandler
  .word RADIO_IRQHandler
  .word UARTE0_UART0_IRQHandler
  .word SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
  .word SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
  .word NFCT_IRQHandler
  .word GPIOTE_IRQHandler
  .word SAADC_IRQHandler
  .word TIMER0_IRQHandler
  .word TIMER1_IRQHandler
  .word TIMER2_IRQHandler
  .word RTC0_IRQHandler
  .word TEMP_IRQHandler
  .word RNG_IRQHandler
  .word ECB_IRQHandler
  .word CCM_AAR_IRQHandler
  .word WDT_IRQHandler
  .word RTC1_IRQHandler
  .word QDEC_IRQHandler
  .word COMP_LPCOMP_IRQHandler
  .word SWI0_EGU0_IRQHandler
  .word SWI1_EGU1_IRQHandler
  .word SWI2_EGU2_IRQHandler
  .word SWI3_EGU3_IRQHandler
  .word SWI4_EGU4_IRQHandler
  .word SWI5_EGU5_IRQHandler
  .word TIMER3_IRQHandler
  .word TIMER4_IRQHandler
  .word PWM0_IRQHandler
  .word PDM_IRQHandler
  .word 0
  .word 0
  .word MWU_IRQHandler
  .word PWM1_IRQHandler
  .word PWM2_IRQHandler
  .word SPIM2_SPIS2_SPI2_IRQHandler
  .word RTC2_IRQHandler
  .word I2S_IRQHandler
  .word FPU_IRQHandler
  .word USBD_IRQHandler
  .word UARTE1_IRQHandler
  .word QSPI_IRQHandler
  .word 0
  .word PWM3_IRQHandler
  .word SPIM3_IRQHandler

.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
  ldr r0, =_estack
  mov sp, r0
  
  /* Copy data section */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b .LoopCopyDataInit

.CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

.LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc .CopyDataInit

  /* Zero bss section */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b .LoopFillZerobss

.FillZerobss:
  str r3, [r2]
  adds r2, r2, #4

.LoopFillZerobss:
  cmp r2, r4
  bcc .FillZerobss

  /* Call system config and main */
  bl SystemInit
  bl main

.LoopForever:
  b .LoopForever

.weak Default_Handler
Default_Handler:
  b .

.macro def_irq handler
.weak \handler
.set \handler, Default_Handler
.endm

def_irq NMI_Handler
def_irq HardFault_Handler
def_irq MemManage_Handler
def_irq BusFault_Handler
def_irq UsageFault_Handler
def_irq SVC_Handler
def_irq DebugMon_Handler
def_irq PendSV_Handler
def_irq SysTick_Handler
def_irq POWER_CLOCK_IRQHandler
def_irq RADIO_IRQHandler
def_irq UARTE0_UART0_IRQHandler
def_irq SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
def_irq SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
def_irq NFCT_IRQHandler
def_irq GPIOTE_IRQHandler
def_irq SAADC_IRQHandler
def_irq TIMER0_IRQHandler
def_irq TIMER1_IRQHandler
def_irq TIMER2_IRQHandler
def_irq RTC0_IRQHandler
def_irq TEMP_IRQHandler
def_irq RNG_IRQHandler
def_irq ECB_IRQHandler
def_irq CCM_AAR_IRQHandler
def_irq WDT_IRQHandler
def_irq RTC1_IRQHandler
def_irq QDEC_IRQHandler
def_irq COMP_LPCOMP_IRQHandler
def_irq SWI0_EGU0_IRQHandler
def_irq SWI1_EGU1_IRQHandler
def_irq SWI2_EGU2_IRQHandler
def_irq SWI3_EGU3_IRQHandler
def_irq SWI4_EGU4_IRQHandler
def_irq SWI5_EGU5_IRQHandler
def_irq TIMER3_IRQHandler
def_irq TIMER4_IRQHandler
def_irq PWM0_IRQHandler
def_irq PDM_IRQHandler
def_irq MWU_IRQHandler
def_irq PWM1_IRQHandler
def_irq PWM2_IRQHandler
def_irq SPIM2_SPIS2_SPI2_IRQHandler
def_irq RTC2_IRQHandler
def_irq I2S_IRQHandler
def_irq FPU_IRQHandler
def_irq USBD_IRQHandler
def_irq UARTE1_IRQHandler
def_irq QSPI_IRQHandler
def_irq PWM3_IRQHandler
def_irq SPIM3_IRQHandler
