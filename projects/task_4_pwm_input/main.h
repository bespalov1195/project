#ifndef MAIN_H
#define MAIN_H

//**************************************************************************************************
// Includes
//**************************************************************************************************

#include <stm32f4xx.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_flash.h>
#include <stm32f4xx_dma.h>
#include <misc.h>

#define FALSE 0
#define TRUE !FALSE


//**************************************************************************************************
// Declarations and definitions
//**************************************************************************************************

//* Перечисляем логи *//


void SetSysClock_HSE_84(void);
void Configure_TIM3(void);
void Configure_USART3(void);
void Configure_DMA1_USART3(void);

extern char res[15];


#endif