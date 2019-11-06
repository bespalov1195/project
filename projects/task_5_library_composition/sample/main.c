#include <stm32f4xx.h>
#include <led.h>

static unsigned
gs_current_color = 0;

static uint8_t
gs_current_color_value[] = {0, 0, 0};

static uint16_t
gs_led_to_color[] = { 0, 0, 0 };

static uint32_t
gs_system_monotonic_time;

static uint32_t
gs_bttn_one_timeout_val_ms = 0;

static uint32_t
gs_bttn_two_timeout_val_ms = 0;

void
EXTI0_IRQHandler(void);

void
EXTI1_IRQHandler(void);

void
SysTickInit(uint16_t frequency)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    gs_system_monotonic_time = 0;
    gs_bttn_one_timeout_val_ms = 0;
    gs_bttn_two_timeout_val_ms = 0;

    (void)SysTick_Config(RCC_Clocks.HCLK_Frequency / frequency);
}

void SysTick_Handler(void)
{
    gs_system_monotonic_time++;

    if (gs_bttn_one_timeout_val_ms && gs_system_monotonic_time - gs_bttn_one_timeout_val_ms > 100)
    {
        gs_bttn_one_timeout_val_ms = 0;
    }
    if (gs_bttn_two_timeout_val_ms && gs_system_monotonic_time - gs_bttn_two_timeout_val_ms > 100)
    {
        gs_bttn_two_timeout_val_ms = 0;
    }
}

int main(void)
{
    NVIC_InitTypeDef interrupt_init;
    GPIO_InitTypeDef gpio_init;
    EXTI_InitTypeDef exti_init;

    /* Init Internal leds for color select indication */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_StructInit(&gpio_init);
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_DOWN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD, &gpio_init);

    gs_led_to_color[0] = GPIO_Pin_14;
    gs_led_to_color[1] = GPIO_Pin_12;
    gs_led_to_color[2] = GPIO_Pin_15;

    /* Init buttons GPIOs */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_StructInit(&gpio_init);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOE, &gpio_init);
    /* GPIO_SetBits(GPIOE, GPIO_Pin_0 | GPIO_Pin_1); */

    /* Init and configure EXTI (external interrupts) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    /* Configure interrupts for E0 button */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);

    EXTI_StructInit(&exti_init);
    exti_init.EXTI_Line    = EXTI_Line0;
    exti_init.EXTI_LineCmd = ENABLE;
    exti_init.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti_init.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&exti_init);

    interrupt_init.NVIC_IRQChannel                   = EXTI0_IRQn;
    interrupt_init.NVIC_IRQChannelPreemptionPriority = 0x05;
    interrupt_init.NVIC_IRQChannelSubPriority        = 0x05;
    interrupt_init.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&interrupt_init);

    /* Configure interrupts for E1 button */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);

    EXTI_StructInit(&exti_init);
    exti_init.EXTI_Line    = EXTI_Line1;
    exti_init.EXTI_LineCmd = ENABLE;
    exti_init.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti_init.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&exti_init);

    interrupt_init.NVIC_IRQChannel                   = EXTI1_IRQn;
    interrupt_init.NVIC_IRQChannelPreemptionPriority = 0x05;
    interrupt_init.NVIC_IRQChannelSubPriority        = 0x05;
    interrupt_init.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&interrupt_init);

    SysTickInit(1000);

    gs_current_color = 0;
    gs_current_color_value[0] = gs_current_color_value[1] = gs_current_color_value[2] = 0;
    GPIO_SetBits(GPIOD, gs_led_to_color[gs_current_color % 3]);

    /* Init LED library */
    led_init_peripherials();
    led_start_timers();

    led_set_color(255, 0, 5);

    while (1)
    {
    }
}

void
EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line0);

        if (gs_bttn_one_timeout_val_ms)
        {
            return;
        }

        gs_current_color_value[gs_current_color % 3] = gs_current_color_value[gs_current_color % 3] + 32;
        led_set_color(gs_current_color_value[0],
                      gs_current_color_value[1],
                      gs_current_color_value[2]);

        gs_bttn_one_timeout_val_ms = gs_system_monotonic_time;
    }
}

void
EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line1);

        if (gs_bttn_two_timeout_val_ms)
        {
            return;
        }

        GPIO_ResetBits(GPIOD, gs_led_to_color[gs_current_color % 3]);
        gs_current_color = (gs_current_color + 1) % 3;
        GPIO_SetBits(GPIOD, gs_led_to_color[gs_current_color % 3]);

        gs_bttn_two_timeout_val_ms = gs_system_monotonic_time;
    }
}