	.syntax unified
	.cpu cortex-m0
	.thumb


	.global Default_handler

	.section .init
/*
	.weak ic_init
	.thumb_set ic_init,0
*/
	.weak Reset_handler
	.func Reset_handler
	.type Reset_handler, %function
Reset_handler:
	bl	ic_init
/*
	ldr r0,=main
	bx  r0 */
	b main
	.size Reset_handler, . - Reset_handler
	.endfunc

	.func	Default_handler
	.type	Default,%function
	.thumb_func
	.align
Default_handler:
	b Default_handler
	.size Default_handler, . - Default_handler
	.endfunc
	.pool

	.section .isr_vector,"a",%progbits
.long	_estack
.long	Reset_handler					/* 0x0800 0004 */
.long	NMI_handler						/* 0x0800 0008 */
.long	HardFault_handler				/* 0x0800 000c */
.long	0								/* 0x0800 0010 Reserved*/
.long	0								/* 0x0800 0014 Reserved*/
.long	0								/* 0x0800 0018 Reserved*/
.long	0								/* 0x0800 001c Reserved*/
.long	0								/* 0x0800 0020 Reserved*/
.long	0								/* 0x0800 0024 Reserved*/
.long	0								/* 0x0800 0028 Reserved*/
.long	SVCall_handler					/* 0x0800 002c */
.long	0								/* 0x0800 0030 Reserved*/
.long	0								/* 0x0800 0034 Reserved*/
.long	PendSV_handler					/* 0x0800 0038 */
.long	SysTick_handler				/* 0x0800 003c */

.long	WWDG_handler					/*  0 0x0800 0040 */
.long	PVD_VDDIO2_handler				/*  1 0x0800 0044 */
.long	RTC_handler						/*  2 0x0800 0048 */
.long	FLASH_handler					/*  3 0x0800 004c */
.long	RCC_CRS_handler					/*  4 0x0800 0050 */
.long	EXTI0_1_handler					/*  5 0x0800 0054 */
.long	EXTI2_3_handler					/*  6 0x0800 0058 */
.long	EXTI4_I5_handler				/*  7 0x0800 005c */
.long	TSC_handler						/*  8 0x0800 0060 */
.long	DMA1_CHN1_handler				/*  9 0x0800 0064 */
.long	DMA1_CHN2_3_handler				/* 10 0x0800 0068 */
.long	DMA1_CHN4_5_handler				/* 11 0x0800 006c */
.long	ADC1_COMP_handler				/* 12 0x0800 0070 */
.long	TIM1_BRK_UP_TRG_COM_handler		/* 13 0x0800 0074 */
.long	TIM1_CC_handler					/* 14 0x0800 0078 */
.long	TIM2_handler					/* 15 0x0800 007c */
.long	TIM3_handler					/* 16 0x0800 0080 */
.long	TIM6_DAC_handler				/* 17 0x0800 0084 */
.long	0								/* 18 0x0800 0088 Reserved*/
.long	TIM14_handler					/* 19 0x0800 008c */
.long	TIM15_handler					/* 20 0x0800 0090 */
.long	TIM16_handler					/* 21 0x0800 0094 */
.long	TIM17_handler					/* 22 0x0800 0098 */
.long	I2C1_handler					/* 23 0x0800 009c */
.long	I2C2_handler					/* 24 0x0800 00a0 */
.long	SPI1_handler					/* 25 0x0800 00a4 */
.long	SPI2_handler					/* 26 0x0800 00a8 */
.long	USART1_handler					/* 27 0x0800 00ac */
.long	USART2_handler					/* 28 0x0800 00b0 */
.long	0								/* 29 0x0800 00b4 Reserved*/
.long	CEC_CAN_handler					/* 30 0x0800 00b8 */
.long	0								/* 31 0x0800 00bc Reserved*/

//	.weak Reset_handler
/* .thumb_set Reset_handler,Default_handler */
.weak NMI_handler
.thumb_set NMI_handler,Default_handler
.weak HardFault_handler
.thumb_set HardFault_handler,Default_handler
.weak MemManage_handler
.thumb_set MemManage_handler,Default_handler
.weak BusFault_handler
.thumb_set BusFault_handler,Default_handler
.weak UsageFault_handler
.thumb_set UsageFault_handler,Default_handler
.weak SVCall_handler
.thumb_set SVCall_handler,Default_handler
.weak DebugMonitor_handler
.thumb_set DebugMonitor_handler,Default_handler
.weak PendSV_handler
.thumb_set PendSV_handler,Default_handler

.weak SystTick_handler
.thumb_set SystTick_handler,Default_handler

.weak WWDG_handler
.thumb_set WWDG_handler,Default_handler
.weak PVD_VDDIO2_handler
.thumb_set PVD_VDDIO2_handler,Default_handler
.weak RTC_handler
.thumb_set RTC_handler,Default_handler
.weak FLASH_handler
.thumb_set FLASH_handler,Default_handler
.weak RCC_CRS_handler
.thumb_set RCC_CRS_handler,Default_handler
.weak EXTI0_1_handler
.thumb_set EXTI0_1_handler,Default_handler
.weak EXTI2_3_handler
.thumb_set EXTI2_3_handler,Default_handler
.weak EXTI4_I5_handler
.thumb_set EXTI4_I5_handler,Default_handler
.weak TSC_handler
.thumb_set TSC_handler,Default_handler
.weak DMA1_CHN1_handler
.thumb_set DMA1_CHN1_handler,Default_handler
.weak DMA1_CHN2_3_handler
.thumb_set DMA1_CHN2_3_handler,Default_handler
.weak DMA1_CHN4_5_handler
.thumb_set DMA1_CHN4_5_handler,Default_handler
.weak ADC1_COMP_handler
.thumb_set ADC1_COMP_handler,Default_handler
.weak TIM1_BRK_UP_TRG_COM_handler
.thumb_set TIM1_BRK_UP_TRG_COM_handler,Default_handler
.weak TIM1_CC_handler
.thumb_set TIM1_CC_handler,Default_handler
.weak TIM2_handler
.thumb_set TIM2_handler,Default_handler
.weak TIM3_handler
.thumb_set TIM3_handler,Default_handler
.weak TIM6_DAC_handler
.thumb_set TIM6_DAC_handler,Default_handler
.weak TIM14_handler
.thumb_set TIM14_handler,Default_handler
.weak TIM15_handler
.thumb_set TIM15_handler,Default_handler
.weak TIM16_handler
.thumb_set TIM16_handler,Default_handler
.weak TIM17_handler
.thumb_set TIM17_handler,Default_handler
.weak I2C1_handler
.thumb_set I2C1_handler,Default_handler
.weak I2C2_handler
.thumb_set I2C2_handler,Default_handler
.weak SPI1_handler
.thumb_set SPI1_handler,Default_handler
.weak SPI2_handler
.thumb_set SPI2_handler,Default_handler
.weak USART1_handler
.thumb_set USART1_handler,Default_handler
.weak USART2_handler
.thumb_set USART2_handler,Default_handler
.weak CEC_CAN_handler
.thumb_set CEC_CAN_handler,Default_handler
	/*stm32f101/2/3/5xxx */

