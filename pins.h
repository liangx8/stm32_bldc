#ifndef PINS_H
#define PINS_H
#ifdef PIN_NDEBUG
#define PON(port,pin)     do{}while(0)
#define POFF(port,pin)    do{}while(0)
#define PTAGGLE(port,pin) do{}while(0)
#else
#define PON(port,pin)     (port)->BSRR = 1 << (pin)
#define POFF(port,pin)    (port)->BSRR = 1 << (pin+16)
#define PTAGGLE(port,pin) (port)->ODR ^= 1 << (pin)
#endif





#define B14ON           PON(GPIOB,14)
#define B14OFF          POFF(GPIOB,14)
#define B14TAGGLE       PTAGGLE(GPIOB,14)

#define B9ON           PON(GPIOB,9)
#define B9OFF          POFF(GPIOB,9)
#define B9TAGGLE       PTAGGLE(GPIOB,9)

#define B5ON           PON(GPIOB,5)
#define B5OFF          POFF(GPIOB,5)
#define B5TAGGLE       PTAGGLE(GPIOB,5)

// a2 high debug enable,low not debug
#define SWDEBUG        (GPIOA->IDR & 0b100)
#endif
