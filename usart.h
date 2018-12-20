#ifndef USART_H
#define USART_H
//#include <stdint.h>



void usart1_enable(void);
void usart1_hex(uint32_t);
int usart1_get(void);
void usart1_puts(uint32_t size,const char *data);
void usart1_putsz(const char *str);
void usart1_disable(void);

#endif
