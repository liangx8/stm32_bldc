#ifndef CONFIG_3498624_H
#define CONFIG_3498624_H
/* 8khz = 48000000/24 * 250 per sec */
#define PWM_MAX 250

#define LED_H    GPIOB->BSRR = GPIO_ODR_11
#define LED_L    GPIOB->BRR  = GPIO_ODR_11
#define BEEP_ON  GPIOC->BSRR = GPIO_ODR_14
#define BEEP_OFF GPIOC->BRR  = GPIO_ODR_14
#endif
