@ -*- mode:arm-asm -*- 
	.syntax unified
	.text
	.global get_rcp_value,TIM2_handler


	.equ    TIM2   ,0x40000000
	.equ	SR     ,0x10
	.equ	CCR1   ,0x34
	.equ	CCR2   ,0x38

	.equ	GPIOA  ,0x48000000
	.equ	BSRR   ,0x18
	.equ    BRR    ,0x28
@ struct of rcp_struct
	.equ	rcp       ,0
	.equ    last_tick ,4
	/*
	 * 功能:读取TIMER2的捕捉信号(RCP)
	 * 这段是根据C的反汇编参考而写成的。SIZE缩小10字节，是为了熟悉汇编指令的练习
	 *
	 */
	.func	TIM2_handler
	.thumb_func
	.type TIM2_handler, %function
	.align
TIM2_handler:

	movs	r2,#0x80
	lsls	r2,#23					@ r2 = TIM2
	ldr	r3,[r2,#SR]
	lsls	r0,r3,#30				@ TIM2->SR.CC1IF
	ldr	r1,=rcp_struct
	bpl	1f
	ldr	r0,[r2,#CCR1]
	str	r0,[r1,#last_tick]			@ rcp_struct.last_tick
1:
	lsls	r3,#29					@ TIM2->SR.CC2IF
	bpl	2f
	ldr	r0,[r2,#CCR2]
	ldr	r3,[r1,#last_tick]			@ rcp_struct.last_tick
	subs	r0,r0,r3
	movs	r3,#0x80
	lsls	r3,#24					@ r3 = 0x80000000
	orrs	r0,r3
	str	r0,[r1,#rcp]				@ rcp_struct.rcp
2:
	bx		lr
	.align
	.size TIM2_handler, . - TIM2_handler
	.endfunc

	.func	get_rcp_value
	.thumb_func
	.type get_rcp_value, %function
	.align
	/*
	 * 功能: 读rcp信号
	 */
get_rcp_value:
	ldr	r1,=rcp_struct
	ldr	r0,[r1,#rcp]
	lsls	r0,#1
	bcc	1f						@ 判断最 31位是否是1
	lsrs	r0,#1						@ 清除最高位(就绪标记)
	str	r0,[r1,#rcp]
	bx	lr
1:
	@ rcp还没就绪
	movs	r0,#0
	bx		lr
	.align
	.size get_rcp_value, . - get_rcp_value
	.endfunc
	.pool

	/*
TIM2_handler:
	movs	r2,#0x80
	lsls	r2,#23					@ r2 = TIM2
	ldr		r1,=rcp_struct
	ldr		r3,[r2,#SR]
	lsls	r0,r3,#30				@ TIM2->SR.CC1IF
	bpl		1f
@ ===============
	push	{r0-r3}
	bl		a2on
	pop		{r0-r3}
@ ===============
/	
1:
	lsls	r3,#29					@ TIM2->SR.CC2IF
	bpl		2f
@ ===============
	push	{r0-r3}
	bl		a2off
	pop		{r0-r3}
@ ===============
	ldr		r0,[r2,#CCR2]			@ TIM2->CCR2
	subs	r0,r0,r3
2:
	bx		lr

	

	
	*/
