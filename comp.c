#include <stm32f0xx.h>
/*
 * 比较器的设置很简单，只有1个配置寄存器 PAGE 297
 * 比较器 只能用固定的4个脚 PA0,PA4,PA5 负端， PA1 正端
 * 可以将比较器输出结果定义到PA11
 */

void comp_config(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN | COMP_CSR_COMP1INSEL_1 | COMP_CSR_COMP1INSEL_2;
  COMP->CSR = COMP_CSR_COMP2LOCK | COMP_CSR_COMP1EN;
  //                   Lock COMP2 setting
  //                                        enable comp1

}



/*
uint32_t comp1_check(void)
{
  return COMP12_COMMON->CSR & COMP_CSR_COMP1OUT;
}
*/
