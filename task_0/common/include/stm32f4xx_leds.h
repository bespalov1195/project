/*
file zb_cortex_sub_ghz_leds.h
PURPOSE: LED's control
*/


#ifndef ZB_CORTEX_SUB_GHZ_LEDS_H
#define ZB_CORTEX_SUB_GHZ_LEDS_H




#ifndef Led_TypeDef
typedef zb_uint16_t     ed_TypeDef;
#endif


void ZB_LED_GPIO_Configuration(void);
void zb_cortexm4_led_test(uint16_t led);
void zb_cortexm4_set_led(uint16_t led);
void zb_cortexm4_clear_led(uint16_t led);


#endif  /* ZB_CORTEX_SUB_GHZ_LEDS_H */
