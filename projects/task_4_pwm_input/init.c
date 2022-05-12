//**************************************************************************************************
// Includes
//**************************************************************************************************
#include "main.h"

RCC_ClocksTypeDef RCC_ClocksTypeDef_user;
uint8_t gFlag;


//**************************************************************************************************
//Procedure SetSysClock_HSE_84() 
//**************************************************************************************************

//* Конфигурация системы тактирования от внешнего осциллятора (HSE 84MHz) *//
void SetSysClock_HSE_84(void)
{
   ErrorStatus HSEStartUpStatus;
   
   RCC_DeInit();
   
   RCC_HSICmd(DISABLE);
   
   RCC_HSEConfig(RCC_HSE_ON);
   HSEStartUpStatus = RCC_WaitForHSEStartUp();

   if(HSEStartUpStatus == SUCCESS)
   {
      RCC_PLLCmd(DISABLE);
      RCC_PLLConfig(RCC_PLLSource_HSE, 4, 84, 2, 2);

      FLASH_PrefetchBufferCmd(ENABLE);
      FLASH_SetLatency(FLASH_Latency_2);    //Flash 2 wait state (2WS(3CPU cycle)  |60 < HCLK <= 90)
      RCC_HCLKConfig(RCC_SYSCLK_Div1);      //HCLK = SYSCLK
      RCC_PCLK2Config(RCC_HCLK_Div1);       //PCLK2 = HCLK
      RCC_PCLK1Config(RCC_HCLK_Div1);       //PCLK1 = HCLK
      
      RCC_PLLCmd(ENABLE);
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
      
      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
      while(RCC_GetSYSCLKSource()!=0x08);
   }

}


//**************************************************************************************************
// Procedure Configure_TIM3()
//**************************************************************************************************

//* Конфигурация TIM3 PC6 Channel:1*//
void Configure_TIM3(void)
{
   //GPIO PC6 "Настройка вывода PC6 на вход"
   GPIO_InitTypeDef gpio_init_structure;

   /* Enable clocking for gpio */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Initialize gpio */
   gpio_init_structure.GPIO_Pin = GPIO_Pin_6;
   gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;
   gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;
   gpio_init_structure.GPIO_OType = GPIO_OType_PP;
   gpio_init_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   
   GPIO_Init(GPIOA, &gpio_init_structure);

   //Подключение вывода к TIM2
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
   
   /* Timer  */
   TIM_TimeBaseInitTypeDef timer_init_structure;
   
   RCC_GetClocksFreq(&RCC_ClocksTypeDef_user);
   gFlag =  RCC_GetSYSCLKSource();

   /* Initialize peripheral clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
   /* Initialize timer */
   timer_init_structure.TIM_Prescaler     = 0;//65535 - 1;  /* Scale value to microseconds */
   timer_init_structure.TIM_CounterMode   = TIM_CounterMode_Up;
   timer_init_structure.TIM_Period        = 0xFFFF;   /* Gives us a second interval */
   timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; /* Tell timer to divide clocks */
   TIM_TimeBaseInit(TIM3, &timer_init_structure);

   /* Initialize Input Capture */
   TIM_ICInitTypeDef TIM_ICInitStruct;

   TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
   TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
   TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
   TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
   TIM_ICInitStruct.TIM_ICFilter = 0x05;
   
   /* Эта функция настроит канал 1 для захвата периода,
     а канал 2 - для захвата заполнения. */ 
   TIM_PWMIConfig(TIM3, &TIM_ICInitStruct);
   
   /* Выбираем источник для триггера: вход 1 (PA6) */
   TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
   
   /* По событию от триггера счётчик будет сбрасываться. */
   TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
   
   /* Включаем события от триггера */
   TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);

   TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
   
   TIM_Cmd(TIM3, ENABLE);
   
   NVIC_EnableIRQ(TIM3_IRQn);


   // TIM_ICInit(TIM3, &TIM_ICInitStruct);

   /* General interrupt enable */
   // NVIC_EnableIRQ(TIM2_IRQn);

   /* Configure timer interrupts */
   // NVIC_InitTypeDef        TIM_NVIC_INIT_Structure;

   // TIM_NVIC_INIT_Structure.NVIC_IRQChannel = TIM3_IRQn;
   // TIM_NVIC_INIT_Structure.NVIC_IRQChannelPreemptionPriority = 0x01;
   // TIM_NVIC_INIT_Structure.NVIC_IRQChannelSubPriority = 1;
   // TIM_NVIC_INIT_Structure.NVIC_IRQChannelCmd = ENABLE;
   // NVIC_Init(&TIM_NVIC_INIT_Structure);

   // /* Interrupt by input capture */
   // TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

   // /* Start timer */
   // TIM_Cmd(TIM3, ENABLE);
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


//**************************************************************************************************
// Procedure Configure_USART3()
//**************************************************************************************************
void Configure_DMA1_USART3(void)
{
   DMA_InitTypeDef dma_init_structure;
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

   dma_init_structure.DMA_Channel = DMA_Channel_4; 
   dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &(USART3->DR);
   dma_init_structure.DMA_Memory0BaseAddr = (uint32_t) res;
   dma_init_structure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
   dma_init_structure.DMA_BufferSize = 15;
   dma_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma_init_structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   dma_init_structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   dma_init_structure.DMA_Mode = DMA_Mode_Normal;
   dma_init_structure.DMA_Priority = DMA_Priority_Medium;
   dma_init_structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma_init_structure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
   dma_init_structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma_init_structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA1_Stream3, &dma_init_structure);

   USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);

   NVIC_EnableIRQ(DMA1_Stream3_IRQn);
   DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
}