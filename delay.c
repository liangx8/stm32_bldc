//#include "delay.h"

#ifndef CPUCLK
#define CPUCLK 1000000
#endif

void delay_5us(void)
{
  /* 这里的指令大概用了6个CPU时钟 */
  for(int i=0;i<CPUCLK/1000/200/6;i++){
	asm(
		"nop"
		);
  }
}
