#include <stdint.h>
#include <stm32f0xx.h>
#include "config.h"
#include "adc.h"
#include "timer.h"
#include "usart.h"
#include "comp.h"
#include "motor.h"
#include "math.h"
#include "pins.h"

/*
 * PA1 comparator+
 * PA0,PA4,PA5 comparator- MODER设置为模拟输入，禁止所有的上下拉电阻
 * PA6 设置位AF 功能。
 */



// reference PAGE 161 GPIOx AFR0
// datasheet PAGE 37 
#define GPIOA_AFR0_VALUE 0x20000000
//                         |      `--- PA0 map to pending 应该不用alternate function
//                         `---------- PA7 map to TIM1_CH1N        AP

#define GPIOA_AFR1_VALUE 0x20007222
//                         |   |||`--- PA8 map to TIM1_CH1         AN
//                         |   ||`---- PA9 map to TIM1_CH2         BN
//                         |   |`----- PA10 map to TIM1_CH3        CN
//                         |   `------ PA11 map to compare out
//                         `---------- PA15 map to TIM2_CH1_ETR

#define GPIOB_AFR0_VALUE 0x00000022
//                               |`--- PB0 map to TIM1_CH2N        BP
//                               `---- PB1 map to TIM1_CH3N        CP
// PB 6/7 AF上电缺省就是,使用缺省值 0
/*
 * PA13/14 不能改,必须保留缺省值，否则会导致单片机无法被编程
 */

#define GPIOA_MODER_VALUE (0b101010 << 26) | (0b10 << 22) | (0b10101010 << 14) | (0b1111 << 8) | (0b1111)
//                           | | |              |              | | | |           ^^^^^^^^^^^^    ^^^^^^^
//                           | | |              |              | | | |           |               `------ PA0/1 in/out analog
//                           | | |              |              | | | |           `---------------------- PA4/5 in/out analog
//                           | | |              |              | | | `---------- PA7 TIM1_CH1N
//                           | | |              |              | | `------------ PA8 TIM1_CH1
//                           | | |              |              | `-------------- PA9 TIM1_CH2
//                           | | |              |              `---------------- PA10 TIM1_CH3
//                           | | |              `---------------  PA11 compare out
//                           | | `------------------------------- PA13 SWDIO(default setting)
//                           | `--------------------------------- PA14 SWCLK(default setting)
//                           `----------------------------------- PA15 TIM2_CH1 rcp_signal
#define GPIOA_OTYPER_VALUE (0b1 << 15)                 // only PA15 set to open-drain for RCP trigger
// B11 定义为LED
// B6/7 应该被设置位alternate function MODER = 10 (page 149)
// page 157
#define GPIOB_MODER_VALUE (0b01 << GPIO_MODER_MODER11_Pos)|(0b1010 << 12) | 0b1010
//                                                            | |             | `-------- PB0 TIM1_CH2N
//                                                            | |             `---------- PB1 TIM1_CH3N
//                                                            | `------------------------ PB 6 USART1_TX AF
//                                                            `-------------------------- PB 7 USART1_RX AF

#define GPIOB_OTYPER_VALUE (0b10 << 6)
//                            |`------------------------PB 6 USART1_TX PP
//                            `-------------------------PB 7 USART1_RX OD
// 在网上看到一些帖说RX要设置为 INPUT FLOAT MODER=0b00，先用自己理解的模式， TX 用 AF PP, RX 用 AF OD
// idle状态下，RX是高电平, TX是低电平(因为把TX关闭了)

// page 158
#define GPIOB_PUPDR_VALUE 0b0000 << 12
//                          | `-------------------------PB 6 float
//                          `---------------------------PB 7 float



const char msg[] = "welcome\r\n";
const char cr[] = "\r\n";

const char msg_motor_idle[]="motor is idle\r\n";
const char msg_motor_start[]="motor is start\r\n";


void idle(struct MOTOR_CONTROL *); // define in motor.c
/*
void test(void)
{
  uint32_t x=0;

  set_step(7);
  
  while(1){
	int rcp=get_rcp_value();
	if(rcp>0){
	  A3TAGGLE;
	  x++;
	  if((x & 0x3f) == 0){
		usart1_hex(rcp);
		usart1_puts(2,cr);
	  }
	}
  }
}
*/
/****************************************************************
 * 在makefile中定义 -DTEST_FUNC=test 来指定一个要运行的测试函数 *
 * 用于学习使用片上设备实验                                     *
 ****************************************************************/
#ifdef TEST_FUNC
  void TEST_FUNC(void);
#endif

int main(void) __attribute__ ((section(".adv_main")));
int main(void)
{
  uint32_t adc_cal_data;
  uint16_t adc_value[2];
#ifdef TEST_FUNC
  TEST_FUNC();
#endif
  usart1_putsz(msg);
  usart1_hex(VREFINT_CAL);

  usart1_putsz(cr);
  adc_cal_data=adc_enable();
  usart1_hex(adc_cal_data);
  adc_sequence(adc_value);
  usart1_hex(adc_value[0]);
  usart1_hex(adc_value[1]);
  usart1_disable();
  LED_H;
  BEEP_ON;

  struct MOTOR_CONTROL mc;
  mc.action=idle;
  while(1)
    mc.action(&mc);

}

/*------------------------------------------------------------
                      选择系统时钟
------------------------------------------------------------*/
void RCC_Init(void)
{
  /* disable all interruption */
  /* NVIC_ICER0 */
  NVIC->ICER[0]=0xffffffff;

  /* enable all interruption */
  //NVIC->ISER[0]=0xffffffff;
  //NVIC->ISER[1]=0xffffffff;
  //NVIC->ISER[2]=0xffffffff;
  /* SysTick setting */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0xffffff;
  /* example code from reference manual
   *[PLL configuration modification code example]
   * 内部8M晶震，48M PLL输出
   */
  /* (1) Test if PLL is used as System clock */
  /* (2) Select HSI as system clock */
  /* (3) Wait for HSI switched */
  /* (4) Disable the PLL */
  /* (5) Wait until PLLRDY is cleared */
  /* (6) Set the PLL multiplier to 12 */
  /* (7) Enable the PLL */
  /* (8) Wait until PLLRDY is set */
  /* (9) Select PLL as system clock */
  /* (10) Wait until the PLL is switched on */
  if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) /* (1) */
	{
	  RCC->CFGR &= (uint32_t) (~RCC_CFGR_SW); /* (2) */
	  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) /* (3) */
		{
		  /* For robust implementation, add here time-out management */
		}
	}
  RCC->CR &= (uint32_t)(~RCC_CR_PLLON);/* (4) */
  while((RCC->CR & RCC_CR_PLLRDY) != 0) /* (5) */
	{
	  /* For robust implementation, add here time-out management */
	}
  RCC->CFGR = (RCC->CFGR & (~RCC_CFGR_PLLMUL)) | (RCC_CFGR_PLLMUL12); /* (6) */
  RCC->CR |= RCC_CR_PLLON; /* (7) */
  while((RCC->CR & RCC_CR_PLLRDY) == 0) /* (8) */
	{
	  /* For robust implementation, add here time-out management */
	}
  // 选择PLL 源,同时切换系统时钟到PLL RCC_CFGR_PLLSRC_HSI_DIV2
  // 对于F051,PLLSRC[0] 被强制保持0，因此只能选择 HSI/2 和 HSE/PREDIV
  RCC->CFGR |= (uint32_t) (RCC_CFGR_SW_PLL) | RCC_CFGR_PLLSRC_HSI_DIV2; /* (9) */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) /* (10) */
	{
	  /* For robust implementation, add here time-out management */
	}
  RCC->CIR = 0x00007f00;     //清除所有中断标记
}

#define GPIOA_MODER_VALUE_DEBUG 0
#define GPIOA_OTYPER_VALUE_DEBUG 0
#define GPIOA_PUPDR_VALUE_DEBUG (0b01 << 4)                                 // a2 pull-up
#define GPIOB_MODER_VALUE_DEBUG (0b01 << 28) | (0b01 << 18) | (0b01 << 10)  // b14 b9 b5 GP OUTPUT
#define GPIOB_OTYPER_VALUE_DEBUG (0b00 << 14) | (0b00 << 9)                 // b14 b9 Push-Pull TYPE

void ic_init(void)
{
  RCC_Init();

  RCC->AHBENR   |= (RCC_AHBENR_GPIOAEN
	| RCC_AHBENR_GPIOBEN
	| RCC_AHBENR_GPIOCEN
	| RCC_AHBENR_GPIOFEN
	| RCC_AHBENR_SRAMEN);//先使能外设IO PORTa,b,c,f时钟, SRAM 时钟
  
  //RCC->APB2ENR   = 0 ;        // 关闭全部外设的时钟
  //RCC->APB1ENR   = 0 ;        // 关闭全部外设的时钟
  //  i=0x55555555;
  GPIOA->AFR[0]  = GPIOA_AFR0_VALUE;
  GPIOA->AFR[1]  = GPIOA_AFR1_VALUE;
  GPIOA->MODER   = GPIOA_MODER_VALUE | GPIOA_MODER_VALUE_DEBUG;
  GPIOA->OTYPER  = GPIOA_OTYPER_VALUE | GPIOA_OTYPER_VALUE_DEBUG;
  GPIOA->OSPEEDR = 0xcfffffff;// all pin but PA13/14 high speed
  GPIOA->PUPDR   = GPIOA_PUPDR_VALUE_DEBUG;

  GPIOB->AFR[0]  = GPIOB_AFR0_VALUE;
  GPIOB->MODER   = GPIOB_MODER_VALUE | GPIOB_MODER_VALUE_DEBUG;
  GPIOB->OTYPER  = GPIOB_OTYPER_VALUE | GPIOB_OTYPER_VALUE_DEBUG;
  //  GPIOB->OSPEEDR = 0xffffffff;// high speed
  GPIOB->PUPDR   = GPIOB_PUPDR_VALUE;

  GPIOC->MODER   = 0x55555555;
  GPIOC->OTYPER  = 0;         // 推挽输出
  GPIOC->OSPEEDR = 0xffffffff;// high speed

  GPIOF->MODER   = 0x55555555;
  GPIOF->OTYPER  = 0;         // 推挽输出
  GPIOF->OSPEEDR = 0xffffffff;// high speed
  timer14_config();
  timer1_pwm_mode(PWM_MAX);
  timer2_config();
  usart1_enable();
  comp_config();
}

