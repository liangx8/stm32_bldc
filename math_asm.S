/*  -*- mode: arm-asm -*-  */
    .syntax unified
    .text
    .global adiv32
/*
	例如 a-b
	C carry, 如果结果是正或者0，C = 1,否则为0，所以CS 是>=
	*/


	.func	adiv32
	.thumb_func
	.type	adiv32, %function
	.align
adiv32:
	push	{r4,r5,lr}
	cmp		r0,r1
	bcc		1f
	tst		r1,r1
	bpl		9f
1:
	movs	r0,#0
	b		10f
9:
	movs	r2,#32
	movs	r4,#0
	movs	r3,#0
	movs	r5,#0
6:
	lsls	r3,#1
	lsls	r0,#1
	adcs	r3,r5

	cmp		r3,r1
	bcc		4f
	subs	r3,r1
	adds	r4,#1
4:
	subs	r2,#1
	bne		5f
	mov		r0,r4
10:
	pop		{r4,r5,pc}
5:
	lsls	r4,#1
	b		6b

	.size	adiv32, . - adiv32
	.endfunc
