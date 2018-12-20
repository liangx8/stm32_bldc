#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
void timer2_config(void);
void timer1_pwm_mode(uint16_t arv);
void set_step(uint8_t step);
void timer14_config(void);
uint32_t get_rcp_value(void);
void timer1_commit_pwm(uint16_t duty);
#endif
