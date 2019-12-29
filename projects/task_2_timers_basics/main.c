#include <stm32f4xx.h>

#define TIM_PRESC (42)
#define TIM_PRD (1000000)
#define GREEN (GPIO_Pin_12)
#define ORANGE (GPIO_Pin_13)
#define RED (GPIO_Pin_14)
#define BLUE (GPIO_Pin_15)

#define FIRST_COLOR 0
#define LAST_COLOR 3
#define LED_DIRECTION_CHANGED 1

uint8_t colors = 4;
uint16_t colors_order[] = {RED, BLUE, GREEN, ORANGE};
uint16_t color_led = RED;

uint8_t direction_led = 1;
uint32_t on_time = 0;


/* 1. Flash LEDs on extension board in an endless loop with 1 sec pause (order R-B-G) */
/* 2. Each time light only one LED */
/* 3. On user button press change LED flashing direction (G-B-R) */

void SetSysClock_HSE_84(void);
static int Configure_buttons(void);
static int Configure_leds(void);
static int Configure_timers(void);
static void Next_color_LED(void);

int main(void)
{
   SetSysClock_HSE_84();
   Configure_leds();
   Configure_buttons();
   Configure_timers();

   GPIO_SetBits(GPIOD, color_led);
   on_time = TIM_GetCounter(TIM2);


   while (1)
   {
      if (on_time == TIM_GetCounter(TIM2))
      {
         Next_color_LED();
      }

      if (LED_DIRECTION_CHANGED == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
      {
         if (0 == direction_led)
         {
            direction_led = 1;
         }
         else
         {
            direction_led = 0;
         }
      }
   }

   return 0;
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

   //Определяем делители шин
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
   RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

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
   //2WS (3CPU cycles) | 60 < HCLK <= 90 | 2.7V - 3.6V
   FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_2WS;

   // Переключаем системное тактирование на PLL
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));

   RCC->CFGR |= RCC_CFGR_SW_PLL;
   while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL)
   {
   }

}

static int Configure_buttons(void)
{
   GPIO_InitTypeDef GPIOA_InitStructure;
   /* Enable clocking for Buttons */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   GPIOA_InitStructure.GPIO_Pin   = GPIO_Pin_0;
   GPIOA_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
   GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIOA_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOA, &GPIOA_InitStructure);
   return 0;
}

static int Configure_leds(void)
{
   GPIO_InitTypeDef GPIOD_InitStructure;
   /* Enable clocking for LEDS */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
   GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
   GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIOD_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIOD_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOD, &GPIOD_InitStructure);

   return 0;
}

static int Configure_timers(void)
{
   /* Timer  */
   TIM_TimeBaseInitTypeDef timer_init_structure;
   /* Initialize peripheral clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

   /* Initialize timer */
   timer_init_structure.TIM_Period        = TIM_PRD - 1;   /* Gives us a second interval */
   timer_init_structure.TIM_Prescaler     = TIM_PRESC- 1;  /* Scale value to microseconds */
   timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; /* Tell timer to divide clocks */
   timer_init_structure.TIM_CounterMode   = TIM_CounterMode_Up;
   timer_init_structure.TIM_RepetitionCounter = 0;

   TIM_TimeBaseInit(TIM2, &timer_init_structure);
   /* Start timer */
   TIM_Cmd(TIM2, ENABLE);

   return 0;
}

static void Next_color_LED()
{
   static int i = FIRST_COLOR + 1;

   GPIO_ResetBits(GPIOD, color_led);
   if (0 == direction_led)
   {
      if (i > LAST_COLOR)
      {
         i = FIRST_COLOR;
      }

      color_led = colors_order[i++];
   }
   else
   {
      if (i < FIRST_COLOR)
      {
         i = LAST_COLOR;
      }

      color_led = colors_order[i--];
   }

   GPIO_SetBits(GPIOD, color_led);
   on_time = TIM_GetCounter(TIM2);
}
