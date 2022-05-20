//**************************************************************************************************
// Includes
//**************************************************************************************************
#include "main.h"

#ifdef DEBUG
   RCC_ClocksTypeDef RCC_ClocksTypeDef_user; 
   uint8_t gFlag;
#endif


//**************************************************************************************************
//Procedure SetSysClock_HSE_142() 
//**************************************************************************************************

//* Конфигурация системы тактирования от внешнего осциллятора (HSE 142MHz) *//
void SetSysClock_HSE_142(void)
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
// Procedure Configure_TIM3()
//**************************************************************************************************

//* Конфигурация TIM3 (16-bit) *//
void Configure_TIM3(void)
{

   #ifdef DEBUG
      RCC_GetClocksFreq(&RCC_ClocksTypeDef_user);
      gFlag =  RCC_GetSYSCLKSource();
   #endif

   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
   
   TIM_TimeBaseInitTypeDef Timer_Init_Structure;
   TIM_TimeBaseStructInit(&Timer_Init_Structure);
   Timer_Init_Structure.TIM_Prescaler     = 142-1;  /* 1MHz -> 1us, 0.0000001 */
   Timer_Init_Structure.TIM_CounterMode   = TIM_CounterMode_Up;
   Timer_Init_Structure.TIM_Period        = 0xFFFF;   /* Gives us a (0,065535s) interval */
   Timer_Init_Structure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseInit(TIM3, &Timer_Init_Structure);

   /* Trigger Update Timer, will be directed to ADC*/
   TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
   
   /* Start timer */
   TIM_Cmd(TIM3, ENABLE);
}


//**************************************************************************************************
// Procedure Configure_ADC()
//**************************************************************************************************

//* Configuration ADC1 PA1 IN1*//
void Configure_ADC1(void)
{
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   
   GPIO_InitTypeDef GPIO_Init_ADC;
   GPIO_Init_ADC.GPIO_Pin = GPIO_Pin_1;
   GPIO_Init_ADC.GPIO_Mode = GPIO_Mode_AN;
   GPIO_Init_ADC.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init_ADC.GPIO_OType = GPIO_OType_PP;
   GPIO_Init_ADC.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOA, &GPIO_Init_ADC);

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
   
   ADC_InitTypeDef ADC_Init_User;
   ADC_Init_User.ADC_Resolution = ADC_Resolution_12b;
   ADC_Init_User.ADC_ScanConvMode = DISABLE;
   ADC_Init_User.ADC_ContinuousConvMode = DISABLE;
   ADC_Init_User.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
   ADC_Init_User.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
   ADC_Init_User.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_Init_User.ADC_NbrOfConversion = 1;
   ADC_Init(ADC1, &ADC_Init_User);

   ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_56Cycles);

   ADC_Cmd(ADC1, ENABLE); //Start ADC
}


//**************************************************************************************************
// Procedure Configure_USART2()
//**************************************************************************************************

//* Configuration USART2: USART2_TX - PA2 *//
void Configure_USART2(void)
{
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 

   GPIO_InitTypeDef GPIO_Init_USART2;
   GPIO_Init_USART2.GPIO_Pin = GPIO_Pin_2;
   GPIO_Init_USART2.GPIO_Mode = GPIO_Mode_AF;
   GPIO_Init_USART2.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init_USART2.GPIO_OType = GPIO_OType_PP;
   GPIO_Init_USART2.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOA, &GPIO_Init_USART2);

   GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); //Provided AF
   
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
   
   USART_InitTypeDef USART2_Init;
   USART2_Init.USART_BaudRate = 9600;
   USART2_Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART2_Init.USART_Mode = USART_Mode_Tx;
   USART2_Init.USART_Parity = USART_Parity_No;
   USART2_Init.USART_StopBits = USART_StopBits_1;
   USART2_Init.USART_WordLength = USART_WordLength_8b;
   USART_Init(USART2, &USART2_Init);

   USART_Cmd(USART2, ENABLE);
}


//**************************************************************************************************
// Procedure Configure_DMA1()
//**************************************************************************************************

void Configure_DMA1(void)
{
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
   
   DMA_InitTypeDef Dma_Usart2_Init_Structure;
   Dma_Usart2_Init_Structure.DMA_Channel = DMA_Channel_4; 
   Dma_Usart2_Init_Structure.DMA_PeripheralBaseAddr = (uint32_t) &(USART2->DR);
   Dma_Usart2_Init_Structure.DMA_Memory0BaseAddr = (uint32_t) res;
   Dma_Usart2_Init_Structure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
   Dma_Usart2_Init_Structure.DMA_BufferSize = SIZE;
   Dma_Usart2_Init_Structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   Dma_Usart2_Init_Structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   Dma_Usart2_Init_Structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   Dma_Usart2_Init_Structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   Dma_Usart2_Init_Structure.DMA_Mode = DMA_Mode_Normal;
   Dma_Usart2_Init_Structure.DMA_Priority = DMA_Priority_Medium;
   Dma_Usart2_Init_Structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   Dma_Usart2_Init_Structure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
   Dma_Usart2_Init_Structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   Dma_Usart2_Init_Structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA1_Stream6, &Dma_Usart2_Init_Structure);

   USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

   NVIC_EnableIRQ(DMA1_Stream6_IRQn);
   DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
}


//**************************************************************************************************
// Procedure Configure_DMA2()
//**************************************************************************************************

void Configure_DMA2(void)
{
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

   DMA_InitTypeDef DMA_ADC1In1_Init_Structure;
   DMA_StructInit(&DMA_ADC1In1_Init_Structure);
   DMA_ADC1In1_Init_Structure.DMA_Channel = DMA_Channel_0; 
   DMA_ADC1In1_Init_Structure.DMA_PeripheralBaseAddr = (uint32_t) &(ADC1->DR);
   DMA_ADC1In1_Init_Structure.DMA_Memory0BaseAddr = (uint32_t) &resADC1;
   DMA_ADC1In1_Init_Structure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   DMA_ADC1In1_Init_Structure.DMA_BufferSize = 1;
   DMA_ADC1In1_Init_Structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_ADC1In1_Init_Structure.DMA_MemoryInc = DMA_MemoryInc_Disable;
   DMA_ADC1In1_Init_Structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
   DMA_ADC1In1_Init_Structure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
   DMA_ADC1In1_Init_Structure.DMA_Mode = DMA_Mode_Circular;
   DMA_ADC1In1_Init_Structure.DMA_Priority = DMA_Priority_Medium;
   DMA_ADC1In1_Init_Structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   DMA_ADC1In1_Init_Structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
   DMA_ADC1In1_Init_Structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   DMA_ADC1In1_Init_Structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA2_Stream0, &DMA_ADC1In1_Init_Structure);

   ADC_DMACmd(ADC1, ENABLE);
   ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

   NVIC_EnableIRQ(DMA2_Stream0_IRQn);
   DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);

   DMA_Cmd(DMA2_Stream0, ENABLE);
}