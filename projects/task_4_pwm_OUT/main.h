#ifndef MAIN_H__
#define MAIN_H__

#include <stm32f4xx.h>

/* PWM frequency */
#define PWM_FREQUENCY_HZ      50

void TIM2_IRQHandler(void);

void configure_leds(void);
void configure_timers(void);
void configure_interrupts(void);
void configure_pwm(void);
void start_timers(void);

#endif