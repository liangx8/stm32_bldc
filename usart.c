#include <stm32f0xx.h>
#define PIN_NDEBUG 1
#include "pins.h"

#define PRIORITY_BYTE 0x00000001 // 有优先输出的字节
#define USART1_BUSY   0x00000002
//                      rx enable      tx enable      tx interrupt      tx complete itru rx no empty intrrupt
//#define USART_CR1_VALUE USART_CR1_RE | USART_CR1_TE | USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RXNEIE
//#define USART_RX_ENABLE USART_CR1_RE
#define USART_RX_ENABLE USART_CR1_RE | USART_CR1_RXNEIE
//#define BUFFER_MAX_SIZE 64
//const char hex_table[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};


uint8_t rx_buff[4];

void usart1_enable(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  NVIC_EnableIRQ(USART1_IRQn);
  //NVIC->ISER[0] = 1<<USART1_IRQn;

  // page 732
  //  USART1->CR1 = USART_CR1_M1|USAR_CR1_M0;
  // 根据 704页的表，用oversampling by 16, 115200 波特率填的数值
  USART1->BRR = 0x1a1;
  // 使能 发送和接受，中断开启
  // 8位宽， 1位停止， 无校验是缺省的设置
  USART1->CR1 = USART_RX_ENABLE;

  //RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
  USART1->CR1 = USART_RX_ENABLE | USART_CR1_UE;
  rx_buff[0]=0;
}
void usart1_disable(void)
{
  RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
  NVIC_DisableIRQ(USART1_IRQn);
  USART1->CR1 = 0;
}
void usart1_putsz(const char *str)
{
  int timeout;
  if(*str){
	USART1->CR1 = USART_RX_ENABLE | USART_CR1_TE | USART_CR1_UE;
	do{
	  timeout=0;
	  while((USART1->ISR & USART_ISR_TXE)==0){
		timeout++;
		if(timeout>1023) goto on_timeout;
	  }
	  USART1->TDR=*str;
	  str++;
	}while(*str);
	timeout=0;
	while((USART1->ISR & USART_ISR_TC)==0){
	  timeout ++;
	  if(timeout > 1023) break;
	}
  on_timeout:
	USART1->CR1 = USART_RX_ENABLE | USART_CR1_UE;                 // 关闭TXE
  }
}
void usart1_puts(uint32_t size,const char *data)
{
  uint32_t timeout;
  USART1->CR1 = USART_RX_ENABLE | USART_CR1_TE | USART_CR1_UE;
  //while((USART1->ISR & USART_ISR_TEACK)==0);
  
  for(uint32_t i=0;i<size;i++){
	timeout=0;
	while((USART1->ISR & USART_ISR_TXE)==0){
	  timeout ++;
	  if (timeout > 1023) goto on_timeout;
	}
	USART1->TDR=data[i];                                        // ISR.TXE 被清零
  }
 
  timeout =0;
  while((USART1->ISR & USART_ISR_TC)==0){
	timeout ++;
	if(timeout > 1023) break;
  }
  on_timeout:
  USART1->CR1 = USART_RX_ENABLE | USART_CR1_UE;                 // 关闭TXE
}
//char hex_buf[9];
/*
void usart1_hex(uint32_t value)
{
  for( int i=0;i<8;i++){
	uint32_t v= (value >> (i * 4)) & 0xf;
	hex_buf[7-i]=hex_table[v];
  }
  hex_buf[8]=' ';
  usart1_puts(9,hex_buf);
}
*/
void USART1_handler(void)
{
  // USART_ISR page 746
  if(USART1->ISR & USART_ISR_RXNE){
	uint8_t c=(uint8_t)USART1->RDR;
	
	if(rx_buff[0] > 0) {
	  // buffer is full,discard it
	  return;
	}
	
	rx_buff[0]=1;
	rx_buff[1]=c;
  }

}
int usart1_get(void)
{
  int idx=rx_buff[0];
  if (idx==0){
	// buffer empty
	return -1;
  }
  rx_buff[0]=0;
  return rx_buff[1];
}
