/*  -*- mode: arm-asm -*-  */
    .syntax unified
    .text
    .global instant_start,instant_stop

	@ struct MOTOR_CONTROL
@	.equ	action,    0
@	.equ	bflag,     0
@	.equ	state,     4
@	.equ	init_comm, 12

	.equ	STK, 0xe000e010
	.equ	CTRL,0
	.equ	LOAD,4
	.equ	VAL,8
	.equ	STK_CTRL_CLKSOURCE_MSK,4
	.equ	STK_CTRL_TICKINT_MSK  ,1
	.equ	STK_CTRL_ENABLE_MSK   ,2

	@ struct RUN_DATA
	.equ	 bflag, 0
	.equ	 state, 4

	.equ	 LAST_RUN_DIR_MSK, 0x00000002
	.equ	 STARTUP_STAGE_MSK, 0x00000010
/*
 * bflag 保留LAST_RUN_DIR_MSK 标记，设置STARTUP_STAGE_MSK标记，其他的清零
 */
    .func   instant_start
    .thumb_func
    .type instant_start, %function
    .align
instant_start:
	movs	r2,#STK_CTRL_CLKSOURCE_MSK | STK_CTRL_TICKINT_MSK | STK_CTRL_ENABLE_MSK
	ldr	r1,=STK
	str	r2,[r1,#CTRL]		@ STK->CTRL=STK_CTRL_CLKSOURCE_MSK | STK_CTRL_TICKINT_MSK | STK_CTRL_ENABLE_MSK
	ldr	r3,=rd
@	ldr	r2,[r3,#init_comm]	@ r2=rd.init_comm
	str	r0,[r1,#LOAD]		@ STK->LOAD= rd.init_comm
	str	r0,[r1,#VAL]		@ STK->VAL= rd.init_comm
	movs	r1,#LAST_RUN_DIR_MSK
	ldr	r2,[r3,#bflag]
	ands	r2,r1
	movs	r1,#STARTUP_STAGE_MSK
	orrs	r2,r1
	str	r2,[r3,#bflag]
	movs	r0,#0
	str	r0,[r3,#state]
	b	set_step			@ call set_step(0)
	.size instant_start, . - instant_start
	.endfunc

    .func   instant_stop
    .thumb_func
    .type instant_stop, %function
    .align
instant_stop:
	movs	r1,#STK_CTRL_CLKSOURCE_MSK
	ldr	r0,=STK
	str	r1,[r0,#CTRL]		@ STK->CTRL = STK_CTRL_CLKSOURCE_MSK
	movs	r1,#0
	str	r1,[r0,#LOAD]		@ STK->LOAD=0
	movs	r0,#7
	b	set_step		@ call set_step(7)
	.size instant_stop, . - instant_stop
	.endfunc



	/*
SysTick_handler:
	ldr		r1,=rd
	ldr		r0,[r1,#20]
	movs	r2,#4			@ r2 = COM_DONE_MSK
	orrs	r0,r2
	str		r0,[r1,#20]		@ rd.bflag |= COM_DONE_MSK
	ldr		r1,=TIM1
	movs	r0,#TIM_EGR_COMG
	str		r0,[r1,#EGR]	@ TIM1->EGR = TIM_EGR_COMG @ 令换相设置生效
	bx		lr
	.align
	*/
	.pool
	/*

void instant_start(void)
{
  SysTick->CTRL=SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
  SysTick->LOAD=STARTUP_COMM;
  SysTick->VAL=STARTUP_COMM;
  rd.calc_comm=STARTUP_COMM;
  rd.real_comm=STARTUP_COMM;
  rd.state=0;
  rd.action=motor_run;
  set_step(0);
}
void instant_stop1(void)
{
  set_step(7);
  SysTick->CTRL=SysTick_CTRL_CLKSOURCE_Msk;
  rd.action=idle;
  SysTick->LOAD=    0;
  SysTick->VAL=     0;
  SysTick->CTRL=    0;
}

	
	*/

	
