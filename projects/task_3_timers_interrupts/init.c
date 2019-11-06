#include "main.h"

void configure_leds()
{
   GPIO_InitTypeDef leds_init_structure;

   /* Enable clocking for LEDS */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

   GPIO_StructInit(&leds_init_structure);
   leds_init_structure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
   leds_init_structure.GPIO_Mode = GPIO_Mode_OUT;
   leds_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
   leds_init_structure.GPIO_OType = GPIO_OType_PP;
   GPIO_Init(GPIOA, &leds_init_structure);

   GPIO_SetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);
}

void configure_buttons()
{
   GPIO_InitTypeDef buttons_init_structure;
   NVIC_InitTypeDef buttons_nvic_init_structure = { 0x0 };
   EXTI_InitTypeDef buttons_exti_init_structure;

   /* Enable clocking for Buttons */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
   /* Init pins */
   buttons_init_structure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
   /* Set mode to input */
   GPIO_StructInit(&buttons_init_structure);
   buttons_init_structure.GPIO_Mode  = GPIO_Mode_IN;
   buttons_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
   buttons_init_structure.GPIO_OType = GPIO_OType_PP;
   buttons_init_structure.GPIO_PuPd  = GPIO_PuPd_UP;
   GPIO_Init(GPIOE, &buttons_init_structure);

   /* Configure button interrupts (only left button)*/
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
   /* Connect EXTI Line0 to PE0 pin */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);

   /* Configure EXTI Line0 */
   EXTI_StructInit(&buttons_exti_init_structure);
   buttons_exti_init_structure.EXTI_Line    = EXTI_Line0;
   buttons_exti_init_structure.EXTI_Mode    = EXTI_Mode_Interrupt;
   buttons_exti_init_structure.EXTI_Trigger = EXTI_Trigger_Rising;
   buttons_exti_init_structure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&buttons_exti_init_structure);

   /* Set interrupts to NVIC */
   buttons_nvic_init_structure.NVIC_IRQChannel = EXTI0_IRQn;
   buttons_nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 0x01;
   buttons_nvic_init_structure.NVIC_IRQChannelSubPriority = 0x01;
   buttons_nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&buttons_nvic_init_structure);
}

void configure_timers()
{
   /* Timer  */
   TIM_TimeBaseInitTypeDef timer_init_structure;
   NVIC_InitTypeDef        timer_nvic_init_structure = { 0x0 };

   /* Initialize peripheral clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
   /* Initialize timer */
   TIM_TimeBaseStructInit(&timer_init_structure);
   timer_init_structure.TIM_Prescaler     = 84;  /* Scale value to microseconds */
   timer_init_structure.TIM_CounterMode   = TIM_CounterMode_Up;
   timer_init_structure.TIM_Period        = 1000000;   /* Gives us a second interval */
   timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; /* Tell timer to divide clocks */
   timer_init_structure.TIM_RepetitionCounter = 0;
   TIM_TimeBaseInit(TIM2, &timer_init_structure);

   /* Configure timer interrupts */
   TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
   timer_nvic_init_structure.NVIC_IRQChannel = TIM2_IRQn;
   timer_nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 0;
   timer_nvic_init_structure.NVIC_IRQChannelSubPriority = 1;
   timer_nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&timer_nvic_init_structure);

   /* Start timer */
   TIM_Cmd(TIM2, ENABLE);
}