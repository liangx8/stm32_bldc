/*  -*- mode: arm-asm -*-  */
	.syntax unified
	.text
	.global usart1_hex

	# line comment
	
	/*
	 * 用adr 可以直接赋值在代码区的常量。
	 * 如: adr r4,hex_table,实际会被编译成 add r4,pc, #<偏移>
	 */
	
	.func	usart1_hex
	.thumb_func
	.type usart1_hex, %function
	.align
usart1_hex:
	push	{r4}
	ldr		r3,=hex_buf
	movs	r1,#8
	adr		r4,hex_table
1:
	movs	r2,#0x0f
	ands	r2,r0
	lsrs	r0,r0,#4
	ldrb	r2,[r4,r2]
	subs	r1,#1
	strb	r2,[r3,r1]
	bne	1b
	movs	r0,#8
	mov		r1,r3

	pop		{r4}
	b		usart1_puts

	.align
	.size usart1_hex, . - usart1_hex

	.endfunc
	.pool

hex_table:
	.ascii "0123456789abcdef"

	.data
hex_buf:
	.space		8						@ keep 8 bytes,
