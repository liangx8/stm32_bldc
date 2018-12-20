/*
 * 
 */

#include <stm32f0xx.h>

#include "pins.h"

struct {
  uint32_t rcp;
  uint32_t last_tick;
} rcp_struct;


/**
 *PSC = 999 48Mhz 
 */

void timer14_config(void)
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
  TIM14->DIER  = 0;
  TIM14->CCMR1 = 0;
  TIM14->CCER  = 0;
  /*CYCLE = 1us*/
  TIM14->PSC   = 23;
  // TIM14->ARR   = 0xffff; // default value
  // TIM14->CCR1  = 0; // disable don't care
  TIM14->CR1   = 1;    /* enable timer14 */
  TIM14->SR    = 0;    /* clear event flag */
  TIM14->EGR   = TIM_EGR_UG;
}

/*
1 TI[1/2/3/4] 是一个输入的pin，分别对应CH1,CH2,CH3,CH4
  例外：TIMx.CR2的 TI1S可控制 TI1连到CH1,CH2,CH3(XOR的方式)或者只连到CH1
2 TIMx.CCMRx的CCxS就有限制的定义 捕捉/比较输出 作用在 TIx上，
  CC1/CC2 可选择 TI1/TI2,
  CC3/CC4 可选择 TI3/TI4,
因此，可以把 2个 捕捉通道 定义到同一个引脚，而比较输出通道不能也不需要这种定义
3 alternate function selected 定义CH1,CH2,CH3,CH4的具体在那个pin
*/
/**
 * config timer2 as input capture mode
 */
void timer2_config(void)
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  //NVIC->ISER[0] = 1<<TIM2_IRQn;
  NVIC_EnableIRQ(TIM2_IRQn);
  //Input mode CC1 map to TI1, CC2 map to TI1
  // CH1和CH2都定义到TI1,然后用上升和下降来区分
  TIM2->CCMR1   = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
  // enable capture1 and capture2 interrupts
  TIM2->PSC     = 23; /*prescaler 24 */
  TIM2->ARR     = 0xffffffff;
  TIM2->DIER    = TIM_DIER_CC1IE | TIM_DIER_CC2IE;
  // CC1NP/CC1P 00 rising edge
  // CC2NP/CC2P 01 falling edge
  TIM2->CCER    = TIM_CCER_CC2P | TIM_CCER_CC1E | TIM_CCER_CC2E;
  TIM2->CR1     = TIM_CR1_URS | TIM_CR1_CEN;
  // 所有的设置，要EGR.UG 置位或者等 update event 发生才会有效，
  // 如果没有这个就会导致上电时，TIM2没安设定的参数跑
  TIM2->EGR     = TIM_EGR_UG;
}
/*
int get_rcp_value_c(void)
{
  if (rcp_struct.rcp & 0x80000000){
	rcp_struct.rcp &= ~0x80000000;

	return rcp_struct.rcp;
  }
  return -1;
}

void TIM2_handler1(void)
{
  uint32_t sr=TIM2->SR;
  if (sr & TIM_SR_CC1IF) {
	rcp_struct.last_tick=TIM2->CCR1;
  }
  if (sr & TIM_SR_CC2IF) {
	uint32_t tmp= TIM2->CCR2;
	rcp_struct.rcp=tmp-rcp_struct.last_tick;
	rcp_struct.rcp |= 1 << 31;

  }

}
*/
/*
void TIM1_BRK_UP_TRG_COM_handler(void){
  TIM1->SR &= ~TIM_SR_UIF;
  //C15ON;

}
void TIM1_CC_handler(void)
{
  TIM1->SR &= ~TIM_SR_CC1IF;
  //C15OFF;
}
*/

/*
name  AF
PA7   TIM1_CH1N
PB0   TIM1_CH2N
PB1   TIM1_CH3N
PA8   TIM1_CH1
PA9   TIM1_CH2
PA10  TIM1_CH3
PA11  TIM1_CH4

CH1,CH2,CH3 定义为PWM输出端
CH1N,CH2N,CH3N 定无PWM输出端

*/
#define DEAD_TIME_FACTOR 20
void timer1_pwm_mode(uint16_t arv)
{
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  /*
   * timer1 as upcounter,
   * pre-scale 1,
   * edge-aligned,
   * Auto-reload preload enable,
   */
  TIM1->CR1 = TIM_CR1_ARPE | TIM_CR1_URS;
  TIM1->CR2=TIM_CR2_CCPC;
  /* (1) Set prescaler to 47, so APBCLK/48 i.e 1MHz */
  /* (2) Set ARR = 8, as timer clock is 1MHz the period is 9 us */
  /* (3) Set CCRx = 4, , the signal will be high during 4 us */
  /* (4) Select PWM mode 1 on OC1 (OC1M = 110),
	 enable preload register on OC1 (OC1PE = 1) */
  /* (5) Select active high polarity on OC1 (CC1P = 0, reset value),
	 enable the output on OC1 (CC1E = 1)*/
  /* (6) Enable output (MOE = 1)*/
  /* (7) Enable counter (CEN = 1)
	 select edge aligned mode (CMS = 00, reset value)
	 select direction as upcounter (DIR = 0, reset value) */
  /* (8) Force update generation (UG = 1) */
  TIM1->PSC = 23; /* (1) */
  TIM1->ARR = arv; /* (2) */
  TIM1->DIER = 0; /*直接控制输出 不需要中断 */
  //  TIM1->CCR1 = 4; /* (3) this value will be updated*/
  TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1
	| TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1
	| TIM_CCMR1_OC2PE
	| TIM_CCMR1_OC1PE; /* (4) */
  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1
	| TIM_CCMR2_OC3PE; /* (4) */
  TIM1->CCER = TIM_CCER_CC1NE | TIM_CCER_CC1E ; /* 使能正反信号，定义正反信号的方向 */
  TIM1->BDTR = TIM_BDTR_OSSI
	| TIM_BDTR_OSSR
	| TIM_BDTR_MOE
	| DEAD_TIME_FACTOR; /* (6) dead time DEAD_TIME_FACTOR * 21ns(48Mhz) */
  TIM1->CR1 |= TIM_CR1_CEN; /* (7) */
  TIM1->EGR = TIM_EGR_COMG|TIM_EGR_UG; /* (8) */
}

/**
 * Description: 
 * 6 步PWM发生设置
 * 运行完成后，要设置TIM->EGR=TIM_EGR_COMG才会生效
 * Backgroud:
 * 下管加载PWM,因此必须把P CHANNEL 联到下管。后面描述的n channel其实是电路上的 上管
 * AP CH1N (上管,无PWM PA7)
 * AN CH1  (下管,有PWM PA8)
 * BP CH2N (上管,无PWM PB0)
 * BN CH2  (下管,有PWM PA9)
 * CP CH3N (上管,无PWM PB1)
 * CN CH3  (下管,有PWM PA10)
 * -----------------
 * AN BP  CH1 CH2N
 * AN CP  CH1 CH3N
 * BN CP  CH2 CH3N
 * BN AP  CH2 CH1N
 * CN AP  CH3 CH1N
 * CN BP  CH3 CH2N

Negative channel open:(上管,无PWM,连接到反通道)
  CCMRx.OCxM=100 mode 保持inactive
  CCMRx.CCxPE=1 preload enable
  CCER.CCxNE=1 complementary output enable
  CCER.CCxE=0 output disable
Positive channel open with complementary PWM:
  CCMRx.OCxM=110 
  CCMRx.CCxPE=1 preload enable
  CCER.CCxNE=1 complementary output enable
  CCER.CCxE=1 output disable
no output
  CCMRx.OCxM=100
  CCMRx.CCxPE=1 preload enable
  CCER.CCxNE=0 complementary output enable
  CCER.CCxE=0 output disable

 * 设置下一步的操作。
 * step 必须在 0 ~ 5
 */
#define ALL_CHA TIM_CCER_CC1E | TIM_CCER_CC1NE | TIM_CCER_CC2E | TIM_CCER_CC2NE | TIM_CCER_CC3E	| TIM_CCER_CC3NE
void set_step(uint8_t step)
{
  uint32_t ccmr1,ccmr2,ccer;

  switch(step){
	// OC2M = 101 force inactive level,根据互补，2 PFET(N channel) 就会高
  case 0: // AN BP
	ccmr1= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1                    // 1PFET
	  | TIM_CCMR1_OC2M_2                                          // 2NFET
	  | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	ccmr2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;                   // 3 N/PFET no output
	ccer = ALL_CHA | TIM_CCER_CC3NP;
	break;
  case 1:// AN CP
	ccmr1= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1                    // 1PFET
	  | TIM_CCMR1_OC2M_2                                          // 2 NP no output
  	  | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	ccmr2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;                   // 3NFET
	ccer = ALL_CHA | TIM_CCER_CC2NP;
	break;
  case 2:// BN CP
	ccmr1= TIM_CCMR1_OC1M_2                                       // 1 NP no output
	  | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1                       // 2PFET
	  | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	ccmr2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;                   // 3NFET
	ccer = ALL_CHA | TIM_CCER_CC1NP;
	break;
  case 3:// BN AP
	ccmr1= TIM_CCMR1_OC1M_2                                  // 1NFET
	  | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1                  // 2PFET
	  | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	ccmr2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;              // 3 NP no output
	ccer = ALL_CHA | TIM_CCER_CC3NP;
	break;
  case 4:// CN AP
	ccmr1= TIM_CCMR1_OC1M_2                                  // 1NFET
	  | TIM_CCMR1_OC2M_2                                     // 2 NP no output
	  | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	ccmr2 = TIM_CCMR2_OC3M_2 |TIM_CCMR2_OC3M_1
	  | TIM_CCMR2_OC3PE;                                     // 3PFET
	ccer = ALL_CHA | TIM_CCER_CC2NP;
	break;
  case 5:// CN BP
	ccmr1= TIM_CCMR1_OC1M_2                                  // 1 NP no output
	  | TIM_CCMR1_OC2M_2                                     // 2NFET
	  | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	  ccmr2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1
		| TIM_CCMR2_OC3PE;                               // 3PFET
	  ccer = ALL_CHA | TIM_CCER_CC1NP;
	break;
  case 6: // break motor
  default: // no output
	ccmr1 = 0;
	ccmr2 = 0;
	ccer =0;
	break;
  }
  TIM1->CCMR1=ccmr1;
  TIM1->CCMR2=ccmr2;
  TIM1->CCER=ccer;
/*
CR2.CCPC 控制 CCxE,CCxNE OCxM 的值是否是被缓存。等待COM事件发生才被更新
电机换相
把3个PWM引脚联到 3个通道(例如:CH1,CH2,CH3),然后分别改变OCxM的值(CCMRx)来控制那个CHANNEL有效，
再然后用COM事件来让设置在换相的时候才生效。

其实跟 6STEP这里的 “6”没半毛钱关系

reference page 384


BDTR.MOE Main output enable
This bit is cleared asynchronously by hardware as soon as the break input is active. It is set by
software or automatically depending on the AOE bit. It is acting only on the channels which are
configured in output.
0: OC and OCN outputs are disabled or forced to idle state.
1: OC and OCN outputs are enabled if their respective enable bits are set (CCxE, CCxNE in
TIMx_CCER register).


Bit 0 CC1E: Capture/Compare 1 output enable
CC1 channel configured as output:
0: Off - OC1 is not active. OC1 level is then function of MOE, OSSI, OSSR, OIS1, OIS1N
and CC1NE bits.
1: On - OC1 signal is output on the corresponding output pin depending on MOE, OSSI,
OSSR, OIS1, OIS1N and CC1NE bits.

*/
}
void timer1_commit_pwm(uint16_t duty)
{
  TIM1->CCR1=duty;
  TIM1->CCR2=duty;
  TIM1->CCR3=duty;
}
