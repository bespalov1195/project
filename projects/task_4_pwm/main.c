#include "main.h"

#include <math.h>

uint32_t time;

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        time++;
        uint16_t period1    = 2000;
        uint16_t period2    = 1500;
        uint16_t period3    = 2200;

        uint16_t pwm_value1 = (cos(time * 6.28 / period1) + 1) / 2.0f * (100000 / PWM_FREQUENCY_HZ - 1);
        uint16_t pwm_value2 = (cos(time * 6.28 / period2 + 3) + 1) / 2.0f * (100000 / PWM_FREQUENCY_HZ - 1);
        uint16_t pwm_value3 = (cos(time * 6.28 / period3 - 10) + 1) / 2.0f * (100000 / PWM_FREQUENCY_HZ - 1);

        TIM_SetCompare1(TIM1, pwm_value1);
        TIM_SetCompare2(TIM1, pwm_value2);
        TIM_SetCompare3(TIM1, pwm_value3);
    }
}

int main(void)
{
    time = 0;

    configure_timers();
    configure_interrupts();
    configure_pwm();

    configure_leds();
    start_timers();

    for (;;);
}
