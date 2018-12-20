
# cross-compiler environment

    pacman -S arm-none-eabi-gcc

# resources
https://community.arm.com/processors/b/blog/posts/writing-your-own-startup-code-for-cortex-m?pi353792392=3

st-util 用 4242
openocd 用 3333
运行openocd服务器端
# stm32f051
openocd -f /usr/share/openocd/scripts/interface/stlink-v2.cfg -f /usr/share/openocd/scripts/target/stm32f0xx.cfg
# stm32f1xx
openocd -f /usr/share/openocd/scripts/interface/stlink-v2.cfg -f /usr/share/openocd/scripts/target/stm32f1xx.cfg

# gdb 的使用

  arm-none-eabi-gdb -tui blink.elf # 打开窗口界面

首先编译时要用 -g参数

运行命令
    gdb target.elf
    (gdb)target extend-remote localhost:4242

烧录固件到芯片
    (gdb)load


When an update event occurs, all the registers are updated and the update flag (UIF bit in
TIMx_SR register) is set (depending on the URS bit):
   The repetition counter is reloaded with the content of TIMx_RCR register,
   The auto-reload shadow register is updated with the preload value (TIMx_ARR),
   The buffer of the prescaler is reloaded with the preload value (content of the TIMx_PSC register).


中断：
  core:  设置NVIC->ISER,
  peripheral:


repetition counter是一个隐形寄存器，每次从TIMX_RCR装载，这样看来TIMx_RCR不会改变，
疑问：如果 update event被禁止，是不是repetition counter就永远是0？
auto-reload shadow register,名称就是很好的说明，

# TIM 的标志 EGR.UG 需要十分注意
 上电设置完成以后，必须要置位。否则会引起设定要等待一次update event才生效

# SysTick 实验总结
    1. 系统不保证SysTick 中断会先于 SysTick->CTRL.COUNTFLAG被置位
	2. 读SysTick->CTRL COUNTFLAG会清除
	3. SysTick 中断根据VAL由1变0会产生一次,不依赖COUNTFLAG


# cortex-m? 关于 ASR,LSL,LSR,ROR，RRX(参考http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204j/Cjacbgca.html)
    具体指令的效果看 PM0215 PAGE 38
    ASR{S} Rd,Rm,Rs|imm5 带符号右移, thumb指令实现架构:全部,如果是thumb指令必须带S
    LSL{S} Rd,Rm,Rs|imm5 逻辑左移, thumb指令实现架构:全部,如果是thumb指令必须带S
	LSR{S} Rd,Rm,Rs|imm5 逻辑右移, thumb指令实现架构:全部,如果是thumb指令必须带S
	ROR{S} Rd,rm,Rs      循环右移, thumb指令实现架构:全部,如果是thumb指令必须带S
	RRX{S} ????          ???, thumb指令实现架构:ARMv6T2 和以上
# STM32F1xx 关于TIMER 中 CMMRx.OCxPE 的设置有这样的描述(RM0008 PAGE 349)

    Note: 1: These bits can not be modified as long as LOCK level 3 has been programmed
    (LOCK bits in TIMx_BDTR register) and CC1S='00' (the channel is configured in
    output).

    我理解的翻译: LOCK level 3时，不能被修改。同时CC1S='00'(通道被设置为输出)
    文档中提到的 CK_INT,根据  P472, CK_INT 来自 RCC 的 TIMxCLK, 而 TIMxCLK 中RCC的说明中来自于系统时钟。因此可以理解为。CK_INT等于
    单片机运行的频率

    CCMRx 中的某些选项，必须要 CCER.CCxE为0时才能修改。设置完成后，必须把CCxE恢复为1