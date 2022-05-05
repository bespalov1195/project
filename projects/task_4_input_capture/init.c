//**************************************************************************************************
// Includes
//**************************************************************************************************
#include "main.h"


//**************************************************************************************************
//Procedure SetSysClock_HSE_84() 
//**************************************************************************************************

//* Конфигурация системы тактирования от внешнего осциллятора (HSE 84MHz) *//
void SetSysClock_HSE_84(void)
{
   //* Сконфигурируем систему тактирование HSI *//
   RCC->CR |= ((uint32_t)RCC_CR_HSION); //включаем HSI и дожидаемся его готовности
   while(!(RCC->CR & RCC_CR_HSIRDY))
   {
   }

   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   PWR->CR |= PWR_CR_VOS;

   //* Определяем делители шин *//
   RCC->CFGR |= RCC_CFGR_HPRE_DIV1; //system clock not divided
   RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; //AHB clock divided by 1 for APB2
   RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; //AHB clock divided by 2 for APB1

   //* Переключаем на внутренний HSI *//
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
   while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_HSI)
   {
   }

   RCC->CR &= ~RCC_CR_PLLON; //выключаем PLL для перенастройки

   RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM; //сбрасываем значение PLL->PLLCFGR

   //* Сконфигурируем систему тактирование HSE *//
   RCC->CR |= ((uint32_t)RCC_CR_HSEON); //включаем HSE и дожидаемся его готовности
   while(!(RCC->CR & RCC_CR_HSERDY))
   {
   }

   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   PWR->CR |= PWR_CR_VOS;

   //* Определяем делители шин *//
   RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;

   //* Определяем кф PLL *//
   uint16_t PLL_M = 4;
   uint16_t PLL_N = 84;
   uint16_t PLL_P = 2;
   uint16_t PLL_Q = 4;

   RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) | (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24); //заносим кофигурацию PLL

   //* Включаем PLL и дожидаемся готовности PLL *//
   RCC->CR |= RCC_CR_PLLON;
   while((RCC->CR & RCC_CR_PLLRDY) == 0)
   {
   }

   //* Настраиваем Flash prefetch, instruction cache, data cache and wait state (2WS (3CPU cycles) | 60 < HCLK <= 90 | 2.7V - 3.6V) *//
   FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_2WS;

   //* Переключаем системное тактирование на PLL *//
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));

   RCC->CFGR |= RCC_CFGR_SW_PLL;
   while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL)
   {
   }

}


//**************************************************************************************************
// Procedure Configure_TIM2()
//**************************************************************************************************

//* Конфигурация TIM2 PA2 Channel:3*//
void Configure_TIM2(void)
{
   //GPIO PA2 "Настройка вывода PA2 на вход"
   GPIO_InitTypeDef gpio_init_structure;

   /* Enable clocking for gpio */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Initialize gpio */
   gpio_init_structure.GPIO_Pin = GPIO_Pin_2;
   gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;
   gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;
   gpio_init_structure.GPIO_OType = GPIO_OType_PP;
   gpio_init_structure.GPIO_PuPd = GPIO_PuPd_DOWN;
   
   GPIO_Init(GPIOA, &gpio_init_structure);

   //Подключение вывода к TIM2
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);
   
   /* Timer  */
   TIM_TimeBaseInitTypeDef timer_init_structure;
   
   /* Initialize peripheral clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
   /* Initialize timer */
   timer_init_structure.TIM_Prescaler     = 0;  /* Scale value to microseconds */
   timer_init_structure.TIM_CounterMode   = TIM_CounterMode_Up;
   timer_init_structure.TIM_Period        = 0xFFFF;   /* Gives us a second interval */
   timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; /* Tell timer to divide clocks */
   TIM_TimeBaseInit(TIM2, &timer_init_structure);

   /* Initialize Input Capture */
   TIM_ICInitTypeDef TIM_ICInitStruct;

   TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;
   TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
   TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
   TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
   TIM_ICInitStruct.TIM_ICFilter = 0x00;
   TIM_ICInit(TIM2, &TIM_ICInitStruct);

   /* General interrupt enable */
   // NVIC_EnableIRQ(TIM2_IRQn);

   /* Configure timer interrupts */
   NVIC_InitTypeDef        timer_nvic_init_structure;

   timer_nvic_init_structure.NVIC_IRQChannel = TIM2_IRQn;
   timer_nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 0x01;
   timer_nvic_init_structure.NVIC_IRQChannelSubPriority = 1;
   timer_nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&timer_nvic_init_structure);

   /* Interrupt by input capture */
   TIM_ITConfig(TIM2, TIM_IT_CC3, ENABLE);

   /* Start timer */
   TIM_Cmd(TIM2, ENABLE);
}


//**************************************************************************************************
// Procedure Configure_USART3()
//**************************************************************************************************

//* Конфигурация USART3: USART3_TX - PC10 *//
void Configure_USART3(void)
{
   GPIO_InitTypeDef GPIO_Init_USART3;
   USART_InitTypeDef USART3_Init;
   // NVIC_InitTypeDef USART3_NVIC_Init_Structure;

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //включили тактирование порта GPIOA
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //включили тактирование порта USART3

   //*Настраиваем ножки контроллера*//
   GPIO_Init_USART3.GPIO_Pin = GPIO_Pin_10;
   GPIO_Init_USART3.GPIO_Mode = GPIO_Mode_AF;
   GPIO_Init_USART3.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init_USART3.GPIO_OType = GPIO_OType_PP;
   GPIO_Init_USART3.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOC, &GPIO_Init_USART3); //инициализировали настроенную структуру

   GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3); //изменили преобразование указанного пина

   
   USART3_Init.USART_BaudRate = 9600; //скорость передачи данных
   USART3_Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //без аппаратного управления контроллером
   USART3_Init.USART_Mode = USART_Mode_Tx; //настройка на передачу данных
   USART3_Init.USART_Parity = USART_Parity_No; //без чётного бита
   USART3_Init.USART_StopBits = USART_StopBits_1; // 1 стоп-бит
   USART3_Init.USART_WordLength = USART_WordLength_8b; //длина передаваемого слова из 8 бит
   USART_Init(USART3, &USART3_Init); //инициализировали структуру


   //* Конфгурация прерывания USART3 *//
   // USART3_NVIC_Init_Structure.NVIC_IRQChannel = USART3_IRQn; //разрешили глобальное прерывание USART3
   // USART3_NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority = 0x00; //приоритет
   // USART3_NVIC_Init_Structure.NVIC_IRQChannelSubPriority = 1;
   // USART3_NVIC_Init_Structure.NVIC_IRQChannelCmd = ENABLE;
   // NVIC_Init(&USART3_NVIC_Init_Structure); //инициализировали структуру

   USART_Cmd(USART3, ENABLE); //запустили USART2
}