//**************************************************************************************************
// Includes
//**************************************************************************************************
#include "main.h"

#ifdef DEBUG
   RCC_ClocksTypeDef RCC_ClocksTypeDef_user; 
   uint8_t gFlag;
#endif


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
      RCC_PLLConfig(RCC_PLLSource_HSE, 4, 142, 2, 2); //142MHz

      FLASH_PrefetchBufferCmd(ENABLE);
      FLASH_SetLatency(FLASH_Latency_4);    //Flash 4 wait state 4WS(5CPU cycle)  [120 < HCLK <= 150]
      RCC_HCLKConfig(RCC_SYSCLK_Div1);      //HCLK = SYSCLK
      RCC_PCLK2Config(RCC_HCLK_Div2);       //PCLK2 72MHz
      RCC_PCLK1Config(RCC_HCLK_Div2);       //PCLK1 = 72MHz
      
      RCC_PLLCmd(ENABLE);
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
      
      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
      while(RCC_GetSYSCLKSource()!=0x08);
   }

}


//**************************************************************************************************
// Procedure Configure_TIM5()
//**************************************************************************************************

//* Конфигурация TIM5 (64-bit) PA0 Channel:1*//
void Configure_TIM5(void)
{
   //GPIO PA0 "Настройка вывода PA0 на вход"
   GPIO_InitTypeDef gpio_init_structure;

   /* Enable clocking for GPIO */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Initialize gpio */
   gpio_init_structure.GPIO_Pin = GPIO_Pin_0;
   gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;
   gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;
   gpio_init_structure.GPIO_OType = GPIO_OType_PP;
   gpio_init_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   
   GPIO_Init(GPIOA, &gpio_init_structure);

   //Подключение вывода к TIM5
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);
   
   /* Timer  */
   TIM_TimeBaseInitTypeDef timer_init_structure;

   #ifdef DEBUG
      RCC_GetClocksFreq(&RCC_ClocksTypeDef_user);
      gFlag =  RCC_GetSYSCLKSource();
   #endif

   /* Initialize peripheral clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
   /* Initialize timer */
   timer_init_structure.TIM_Prescaler     = 0;  /* 142MHz -> 7.04ns, [1Hz;1000kHz] with margin of error */
   timer_init_structure.TIM_CounterMode   = TIM_CounterMode_Up;
   timer_init_structure.TIM_Period        = 0xFFFFFFFF;   /* Gives us a second interval */
   timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; /* Tell timer to divide clocks */
   TIM_TimeBaseInit(TIM5, &timer_init_structure);

   /* Initialize Input Capture */
   TIM_ICInitTypeDef TIM_ICInitStruct;

   TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
   TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
   TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
   TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
   TIM_ICInitStruct.TIM_ICFilter = 0x00;
   
   /* Эта функция настроит канал 1 для захвата периода,
     а канал 2 - для захвата заполнения. */ 
   TIM_PWMIConfig(TIM5, &TIM_ICInitStruct);
   
   /* Выбираем источник для триггера: вход 1 (PA0) */
   TIM_SelectInputTrigger(TIM5, TIM_TS_TI1FP1);
   
   /* По событию от триггера счётчик будет сбрасываться. */
   TIM_SelectSlaveMode(TIM5, TIM_SlaveMode_Reset);
   
   /* Включаем события от триггера */
   TIM_SelectMasterSlaveMode(TIM5, TIM_MasterSlaveMode_Enable);

   // /* Interrupt by input capture */
   // TIM_ITConfig(TIM5, TIM_IT_CC1, ENABLE);

   // NVIC_EnableIRQ(TIM5_IRQn);

   // /* Start timer */
   // TIM_Cmd(TIM5, ENABLE);
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

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //включили тактирование порта GPIOC
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

   USART_Cmd(USART3, ENABLE); //запустили USART3
}


//**************************************************************************************************
// Procedure Configure_DMA1()
//**************************************************************************************************

void Configure_DMA1(void)
{
   DMA_InitTypeDef dma_usart3_init_structure;
   DMA_InitTypeDef dma_tim2ch3_init_structure;
   DMA_InitTypeDef dma_tim2ch4_init_structure;
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

   dma_usart3_init_structure.DMA_Channel = DMA_Channel_4; 
   dma_usart3_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &(USART3->DR);
   dma_usart3_init_structure.DMA_Memory0BaseAddr = (uint32_t) res;
   dma_usart3_init_structure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
   dma_usart3_init_structure.DMA_BufferSize = 15;
   dma_usart3_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma_usart3_init_structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   dma_usart3_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   dma_usart3_init_structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   dma_usart3_init_structure.DMA_Mode = DMA_Mode_Normal;
   dma_usart3_init_structure.DMA_Priority = DMA_Priority_Medium;
   dma_usart3_init_structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma_usart3_init_structure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
   dma_usart3_init_structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma_usart3_init_structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA1_Stream3, &dma_usart3_init_structure);

   dma_tim2ch3_init_structure.DMA_Channel = DMA_Channel_6; 
   dma_tim2ch3_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &(TIM5->CCR1);
   dma_tim2ch3_init_structure.DMA_Memory0BaseAddr = (uint32_t) &IC1Value;
   dma_tim2ch3_init_structure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   dma_tim2ch3_init_structure.DMA_BufferSize = 1;
   dma_tim2ch3_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma_tim2ch3_init_structure.DMA_MemoryInc = DMA_MemoryInc_Disable;
   dma_tim2ch3_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   dma_tim2ch3_init_structure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word;
   dma_tim2ch3_init_structure.DMA_Mode = DMA_Mode_Circular;
   dma_tim2ch3_init_structure.DMA_Priority = DMA_Priority_High;
   dma_tim2ch3_init_structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma_tim2ch3_init_structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
   dma_tim2ch3_init_structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma_tim2ch3_init_structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA1_Stream2, &dma_tim2ch3_init_structure);
   DMA_Cmd(DMA1_Stream2, ENABLE);

   dma_tim2ch4_init_structure.DMA_Channel = DMA_Channel_6; 
   dma_tim2ch4_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &(TIM5->CCR2);
   dma_tim2ch4_init_structure.DMA_Memory0BaseAddr = (uint32_t) &IC2Value;
   dma_tim2ch4_init_structure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   dma_tim2ch4_init_structure.DMA_BufferSize = 1;
   dma_tim2ch4_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma_tim2ch4_init_structure.DMA_MemoryInc = DMA_MemoryInc_Disable;
   dma_tim2ch4_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   dma_tim2ch4_init_structure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word;
   dma_tim2ch4_init_structure.DMA_Mode = DMA_Mode_Circular;
   dma_tim2ch4_init_structure.DMA_Priority = DMA_Priority_Medium;
   dma_tim2ch4_init_structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma_tim2ch4_init_structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
   dma_tim2ch4_init_structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma_tim2ch4_init_structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA1_Stream4, &dma_tim2ch4_init_structure);
   DMA_Cmd(DMA1_Stream4, ENABLE);

   
   TIM_DMAConfig(TIM5, TIM_DMABase_CCR1, TIM_DMABurstLength_4Transfers);
   TIM_DMAConfig(TIM5, TIM_DMABase_CCR2, TIM_DMABurstLength_4Transfers);

   NVIC_EnableIRQ(DMA1_Stream3_IRQn);
   // NVIC_EnableIRQ(DMA1_Stream4_IRQn);
   // NVIC_EnableIRQ(DMA1_Stream5_IRQn);

   DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
   // DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
   // DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);

   USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
   TIM_DMACmd(TIM5, TIM_DMA_CC1,ENABLE);
   TIM_DMACmd(TIM5, TIM_DMA_CC2,ENABLE);
   
}