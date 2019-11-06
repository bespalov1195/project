#include "main.h"

/* 1. Flash LEDs on extension board from 1 to 3 in an endless loop with 1 sec pause */
/* 2. Each time light only one LED */
/* 3. On user button press change LED flashing direction */

static int8_t leds_counter;
static int8_t leds_direction;

void EXTI0_IRQHandler(void)
{
   if (EXTI_GetITStatus(EXTI_Line0) != RESET)
   {
      EXTI_ClearITPendingBit(EXTI_Line0);

      leds_direction = -leds_direction;
   }
}

void TIM2_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

      GPIO_SetBits(GPIOA, GPIO_Pin_8 << leds_counter);
      leds_counter = (3 + (leds_counter - leds_direction) % 3) % 3;
      GPIO_ResetBits(GPIOA, GPIO_Pin_8 << leds_counter);
   }
}

int main(void)
{
   leds_counter = 0;
   leds_direction = 1;

   configure_leds();
   configure_buttons();
   configure_timers();

   for (;;)
   {
   }
}
