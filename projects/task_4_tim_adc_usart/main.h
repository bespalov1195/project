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
#define DEBUG
#define SIZE 6


//**************************************************************************************************
// Declarations and definitions
//**************************************************************************************************

void SetSysClock_HSE_142(void);
void Configure_USART2(void);
void Configure_TIM3(void);
void Configure_ADC1(void);
void Configure_DMA1(void);
void Configure_DMA2(void);

extern char res[6];
extern uint16_t resADC1;

#endif