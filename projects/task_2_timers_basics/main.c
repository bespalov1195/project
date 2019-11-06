#include <stm32f4xx.h>

/* 1. Flash LEDs on extension board in an endless loop with 1 sec pause (order R-B-G) */
/* 2. Each time light only one LED */
/* 3. On user button press change LED flashing direction (G-B-R) */
static
int configure_leds(void)
{
   GPIO_InitTypeDef leds_init_structure;
   /* Enable clocking for LEDS */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   leds_init_structure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
   leds_init_structure.GPIO_Mode = GPIO_Mode_OUT;
   leds_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
   leds_init_structure.GPIO_OType = GPIO_OType_PP;
   GPIO_Init(GPIOA, &leds_init_structure);
   GPIO_SetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);

   return 0;
}

static
int configure_buttons(void)
{
   GPIO_InitTypeDef buttons_init_structure;
   /* Enable clocking for Buttons */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
   buttons_init_structure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
   buttons_init_structure.GPIO_Mode  = GPIO_Mode_IN;
   buttons_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
   buttons_init_structure.GPIO_OType = GPIO_OType_PP;
   buttons_init_structure.GPIO_PuPd  = GPIO_PuPd_UP;
   GPIO_Init(GPIOE, &buttons_init_structure);
   return 0;
}

static
int configure_timers(void)
{
   /* Timer  */
   TIM_TimeBaseInitTypeDef timer_init_structure;
   /* Initialize peripheral clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
   /* Initialize timer */
   timer_init_structure.TIM_Prescaler     = 84;  /* Scale value to microseconds */
   timer_init_structure.TIM_CounterMode   = TIM_CounterMode_Up;
   timer_init_structure.TIM_Period        = 1000000;   /* Gives us a second interval */
   timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; /* Tell timer to divide clocks */
   timer_init_structure.TIM_RepetitionCounter = 0;
   TIM_TimeBaseInit(TIM2, &timer_init_structure);
   /* Start timer */
   TIM_Cmd(TIM2, ENABLE);

   return 0;
}

int main(void)
{
   uint32_t timer_value = 0;

   int8_t   led_counter   = 0;
   int8_t   led_direction = 1;

   uint8_t  button_state          = Bit_RESET;
   uint8_t  previous_button_state = Bit_RESET;

   configure_leds();
   configure_buttons();
   configure_timers();

   for (;;)
   {
      timer_value = TIM_GetCounter(TIM2);
      if (timer_value == 0)
      {
         GPIO_ResetBits(GPIOA, GPIO_Pin_8 << led_counter);
      }
      else if (timer_value == 1000000)
      {
         GPIO_SetBits(GPIOA, GPIO_Pin_8 << led_counter);
         led_counter = (3 + (led_counter - led_direction) % 3) % 3;
      }

      /* Handle button press */
      button_state = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0);
      /* Trigger event on button release */
      if (previous_button_state == Bit_SET && button_state == Bit_RESET)
      {
         led_direction = -led_direction;
      }
      previous_button_state = button_state;
   }
}
