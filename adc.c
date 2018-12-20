#include <stm32f0xx.h>


//#define VREFINT_CAL *((volatile uint16_t*)0x1ffff7ba)



void adc_chn_select(void);
uint32_t adc_enable(void)
{
  uint32_t adc_cal_val;
  RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
  /*configure pa2 to analog mode*/
  GPIOA->MODER |= 0x300;
  /* (1) Ensure that ADEN = 0 */
  /* (2) Clear ADEN by setting ADDIS*/
  /* (3) Clear DMAEN */
  /* (4) Launch the calibration by setting ADCAL */
  /* (5) Wait until ADCAL=0 */
  ADC->CCR = ADC_CCR_VREFEN;
  if ((ADC1->CR & ADC_CR_ADEN) != 0) /* (1) */
	{
	  ADC1->CR |= ADC_CR_ADDIS; /* (2) */
	}
  while ((ADC1->CR & ADC_CR_ADEN) != 0)
	{
	  /* For robust implementation, add here time-out management */
	}
  ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN; /* (3) */
  ADC1->CR |= ADC_CR_ADCAL; /* (4) */
  while ((ADC1->CR & ADC_CR_ADCAL) != 0) /* (5) */
	{
	  /* For robust implementation, add here time-out management */
	}
  adc_cal_val=ADC1->DR;/* aquires calibration value */
  /* (1) Ensure that ADRDY = 0 */
  /* (2) Clear ADRDY */
  /* (3) Enable the ADC */
  /* (4) Wait until ADC ready */
  if ((ADC1->ISR & ADC_ISR_ADRDY) != 0) /* (1) */
	{
	  ADC1->ISR |= ADC_ISR_ADRDY; /* (2) */
	}
  ADC1->CR |= ADC_CR_ADEN; /* (3) */
  return adc_cal_val;
}

void adc_sequence(uint16_t seqs[2])
{
  while(ADC1->CR & ADC_CR_ADSTART); /* wait current conversion finish */

  ADC1->CFGR1 = ADC_CFGR1_WAIT ;
  ADC1->CHSELR = ADC_CHSELR_CHSEL2|ADC_CHSELR_CHSEL17;
  ADC1->SMPR=ADC_SMPR_SMP_0|ADC_SMPR_SMP_1|ADC_SMPR_SMP_2;      /*sampling time selection 
								  000 1.5 ADC clock cycles 
								  ... 111 239.5 adc clock cycles*/

  ADC1->CR |= ADC_CR_ADSTART;
  while((ADC1-> ISR & ADC_ISR_EOC)==0);
  seqs[0]=ADC1->DR;
  while((ADC1-> ISR & ADC_ISR_EOC)==0);
  seqs[1]=ADC1->DR;
}
void adc_dma(uint16_t *data,uint16_t len)
{
  uint32_t tmp=0;
  /*made sure bit ADC_DMA_RMP of SYSCFG->CFGR1 is clear*/
  //SYSCFG->CFGR1 = 0;
  
  /* (1) Enable the peripheral clock on DMA */
  /* (2) Enable DMA transfer on ADC - DMACFG is kept at 0
	 for one shot mode */
  /* (3) Configure the peripheral data register address */
  /* (4) Configure the memory address */
  /* (5) Configure the number of DMA tranfer to be performs
	 on DMA channel 1 */
  /* (6) Configure increment, size and interrupts */
  /* (7) Enable DMA Channel 1 */
  RCC->AHBENR |= RCC_AHBENR_DMA1EN; /* (1) */
  ADC1->CFGR1 = ADC_CFGR1_DMAEN; /* (2) */
  DMA1_Channel1->CPAR = (uint32_t) (&(ADC1->DR)); /* (3) */
  DMA1_Channel1->CMAR = (uint32_t)data; /* (4) */
  DMA1_Channel1->CNDTR = len; /* (5) */
  /*memory in 16bits mode, peripheral in 16bits mode*/
  tmp = DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0
	| DMA_CCR_TEIE | DMA_CCR_TCIE ; /* (6) */
  DMA1_Channel1->CCR = tmp;
  DMA1_Channel1->CCR |= DMA_CCR_EN; /* (7) */
}
void DMA1_CHN1_handler(void) 
{
  if((DMA1->ISR & DMA_ISR_TCIF1)==0)return;
  DMA1->IFCR = DMA_IFCR_CTCIF1;
}
