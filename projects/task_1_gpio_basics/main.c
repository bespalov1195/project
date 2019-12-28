#include <stm32f4xx.h>
#include <system_stm32f4xx.h>

/*
* @brief Simple lab -- blink leds in R->B->G order, on button press
*        LEDS blink white once for a short period of time and then
*        blink in a reverse order G->B->R.
*/

#define SWITCH_DELAY    ((uint32_t)200000)

typedef enum blink_mul_e
{
   BLINK_MUL_FAST = 1,
   BLINK_MUL_LONG = 3,
} blink_mul_t;

void blink_led(GPIO_TypeDef * port, uint16_t pins, blink_mul_t multiplier);

void SetSysClock_HSE_84(void);

void SetSysClock_HSI_168(void);

int main(void)
{

   GPIO_InitTypeDef GPIOD_InitStructure;
   GPIO_InitTypeDef GPIOA_InitStructure;

   /* Enable peripheral clock for LEDs and buttons port */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

   /* Init LEDs */
   GPIOD_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
   GPIOD_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
   GPIOD_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIOD_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOD, &GPIOD_InitStructure);

   GPIOA_InitStructure.GPIO_Pin   = GPIO_Pin_0;
   GPIOA_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
   GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIOA_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOA, &GPIOA_InitStructure);

/*   uint8_t flag = 1;

   GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_14);

   while (1)
   {
      if(flag == 255)
      {
         flag = 1;
      }

      if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
      {
         flag += 1;

         if (flag % 2 == 0)
         {
            GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_14);
            GPIO_SetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_15);
         }
         else if (flag % 2 == 1)
         {
            GPIO_ResetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_15);
            GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_14);
         }

      }



   }
}
*/
   uint8_t i = 0;
   uint8_t state;
   uint8_t flag = 1;

   while (1)
   {
      state = 0;
      state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
      if (state == 1)
      {
         flag += 1;
         if (flag == 255)
         {
            flag = 1;
         }

         if (flag % 2 == 0)
         {
            SetSysClock_HSI_168();
         }
         else if (flag % 2 == 1)
         {
            SetSysClock_HSE_84();
         }
      }

      blink_led(GPIOD, GPIO_Pin12 << i, BLINK_MUL_LONG);

      i = (3 + (i - direction) % 3) % 3;
   }

}

void blink_led(GPIO_TypeDef * port, uint16_t pins, blink_mul_t multiplier)
{
  uint32_t k = 0;

  GPIO_ResetBits(port, pins);
  for (k = 0; k < SWITCH_DELAY * multiplier; ++k);
  GPIO_SetBits(port, pins);
}


void SetSysClock_HSE_84(void)
{
   //Сконфигурируем систему тактирование HSI
   //Включаем HSI и дожидаемся его готовности
   RCC->CR |= ((uint32_t)RCC_CR_HSION);
   while(!(RCC->CR & RCC_CR_HSIRDY))
   {
   }

   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   PWR->CR |= PWR_CR_VOS;

   RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;

   //Переключаем на внутренний HSI
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
   while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_HSI)
   {
   }

   //Выключаем PLL для перенастройки
   RCC->CR &= ~RCC_CR_PLLON;

   //Сбрасываем значение PLL->PLLCFGR
   RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;

   //Сконфигурируем систему тактирование HSE
   //Включаем HSE и дожидаемся его готовности
   RCC->CR |= ((uint32_t)RCC_CR_HSEON);
   while(!(RCC->CR & RCC_CR_HSERDY))
   {
   }

   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   PWR->CR |= PWR_CR_VOS;

   //Определяем делители шин
   RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

   //Определяем кф PLL
   uint16_t PLL_M = 4;
   uint16_t PLL_N = 84;
   uint16_t PLL_P = 2;
   uint16_t PLL_Q = 4;

   //Заносим кофигурацию PLL
   RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) | (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

   //Включаем PLL и дожидаемся готовности PLL
   RCC->CR |= RCC_CR_PLLON;
   while((RCC->CR & RCC_CR_PLLRDY) == 0)
   {
   }

   // Настраиваем Flash prefetch, instruction cache, data cache and wait state
   FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_2WS;

   // Переключаем системное тактирование на PLL
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));

   RCC->CFGR |= RCC_CFGR_SW_PLL;
   while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL)
   {
   }

}

void SetSysClock_HSI_168(void)
{

   //Сконфигурируем систему тактирование HSI
   RCC->CR |= ((uint32_t)RCC_CR_HSION);
   while(!(RCC->CR & RCC_CR_HSIRDY))
   {
   }

   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   PWR->CR |= PWR_CR_VOS;

   RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;

   //Переключаем на внутренний HSI
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
   while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_HSI)
   {
   }

   //Выключаем PLL для перенастройки
   RCC->CR &= ~RCC_CR_PLLON;

   //Сбрасываем значение PLL->PLLCFGR
   RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;

   //Cконфигурируем систему тактирование
   //Включаем HSI и дожидаемся его готовности
   RCC->CR |= ((uint32_t)RCC_CR_HSION);
   while(!(RCC->CR & RCC_CR_HSIRDY))
   {
   }

   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   PWR->CR |= PWR_CR_VOS;

   //Определяем делители шин
   RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
   RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
   RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

   //Определяем кф для PLL
   uint16_t PLL_M = 8;
   uint16_t PLL_N = 168;
   uint16_t PLL_P = 2;
   uint16_t PLL_Q = 7;

   // Заносим конфигурацию PLL
   RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) | (RCC_PLLCFGR_PLLSRC_HSI) | (PLL_Q << 24);

   // Включаем PLL и дожидаемся готовности PLL
   RCC->CR |= RCC_CR_PLLON;
   while((RCC->CR & RCC_CR_PLLRDY) == 0)
   {
   }

   //Настраиваем Flash prefetch, instruction chache, data cacha and wait state
   FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_2WS;

   //Переключаем системное тактирование на PLL
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));

   RCC->CFGR |= RCC_CFGR_SW_PLL;
   while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL)
   {
   }

}

