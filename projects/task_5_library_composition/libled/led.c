#include "led.h"

#define PWM_WIDTH (1000000)
#define PWM_FREQUENCY_HZ (50)
static void
s_led_init_led_pins(void)
{
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    /* Enable clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);

    /* Initialize GPIOs for an alternative function (TIM1 PWM output) */
    gpio_init.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio_init);
}

static void
s_led_init_timers(uint32_t period)
{
    TIM_TimeBaseInitTypeDef tim_init;

    TIM_TimeBaseStructInit(&tim_init);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    tim_init.TIM_Prescaler         = 168 - 1; /* One microsecond resolution */
    tim_init.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_init.TIM_Period            = period / PWM_FREQUENCY_HZ - 1;
    tim_init.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_init.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &tim_init);
}

static void
s_led_init_pwm_mode(void)
{
    TIM_OCInitTypeDef tim_oc_init;

    TIM_OCStructInit(&tim_oc_init);

    tim_oc_init.TIM_OCMode      = TIM_OCMode_PWM1;
    tim_oc_init.TIM_Pulse       = 0;
    tim_oc_init.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init.TIM_OCPolarity  = TIM_OCPolarity_Low;

    TIM_OC1Init(TIM1, &tim_oc_init);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    tim_oc_init.TIM_Pulse       = 0;
    TIM_OC2Init(TIM1, &tim_oc_init);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    tim_oc_init.TIM_Pulse       = 0;
    TIM_OC3Init(TIM1, &tim_oc_init);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
}

int
led_init_peripherials(void)
{
    s_led_init_timers(PWM_WIDTH);
    s_led_init_pwm_mode();
    s_led_init_led_pins();

    return 0;
}

int
led_start_timers(void)
{
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);

    return 0;
}

int
led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t period = PWM_WIDTH / PWM_FREQUENCY_HZ / 255;

    TIM_SetCompare1(TIM1, period * red );
    TIM_SetCompare2(TIM1, period * green );
    TIM_SetCompare3(TIM1, period * blue );

    return 0;
}