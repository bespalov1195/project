#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>
#include <stm32f4xx.h>

int led_init_peripherials(void);
int led_start_timers(void);
int led_set_color(uint8_t red, uint8_t green, uint8_t blue);

#endif