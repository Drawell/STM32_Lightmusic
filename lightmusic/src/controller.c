#include "controller.h"

static void init_button(void);
static void init_timer(void);
static void simple_fourier_light(uint16_t *data, uint16_t length);
static void simple_sound_loudness(uint16_t *data, uint16_t length);

static signal_handle_function_t handlers[] = {
    simple_fourier_light,
    simple_sound_loudness};
static uint8_t current_handler_idx;

static uint8_t b_ignore_interrupt;
static uint8_t current_hello_toggle;
static uint8_t b_saying_hello;

static LED_color_t leds[] = {BLUE, GREEN, ORANGE, RED};

void init_controller(void)
{
    current_handler_idx = 0;
    b_ignore_interrupt = 0;
    b_saying_hello = 0;
    init_button();
    init_timer();
}

void say_hello(void)
{
    current_hello_toggle = 0;
    b_saying_hello = 1;
    TIM_Cmd(TIM2, ENABLE);
}

static void say_hallo_timer_handle(void)
{
    if (current_hello_toggle)

    if (current_hello_toggle % 4 >= 2)
        turn_off_led(BLUE | GREEN | ORANGE | RED);
    else
        turn_on_led(BLUE | GREEN | ORANGE | RED);

    if (current_hello_toggle == 12)        
        b_saying_hello = 0;

    current_hello_toggle++;
}

void microphone_interrupt_handler(uint16_t *data, uint16_t length)
{
    if (b_saying_hello)
        return;

    handlers[current_handler_idx](data, length);
}

#define freq_count 4
int32_t freq_magnitudes[freq_count];

static void simple_fourier_light(uint16_t *data, uint16_t length)
{
    memset(freq_magnitudes, 0, freq_count * sizeof(freq_magnitudes[0]));
    FFT(data, length, freq_magnitudes, freq_count);

    int32_t max_fr = 0;
    for (int i = 0; i < freq_count; i++)
        max_fr = max_fr > freq_magnitudes[i] ? max_fr : freq_magnitudes[i];

    int32_t min_fr = max_fr;
    for (int i = 0; i < freq_count; i++)
        min_fr = min_fr < freq_magnitudes[i] ? min_fr : freq_magnitudes[i];

    if (max_fr == 0 || max_fr == min_fr)
        return;

    for (int i = 0; i < freq_count; i++)
    {
        uint8_t brightness = (freq_magnitudes[i] - min_fr) * 100 / (max_fr - min_fr);
        brightness = (brightness * brightness) / 100;
        set_brightness(leds[i], brightness);
    }
}

static void simple_sound_loudness(uint16_t *data, uint16_t length)
{
    uint8_t volume = signal_volume_in_percent(data, length);
    uint8_t quarter_volume;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (volume > 25)
        {
            quarter_volume = 100;
            volume -= 25;
        }
        else
        {
            quarter_volume = volume * 4;
            volume = 0;
        }
        set_brightness(leds[i], quarter_volume);
    }
}

static void init_button(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

static void init_timer(void)
{
    TIM_TimeBaseInitTypeDef tim_struct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    tim_struct.TIM_Period = 200 - 1;
    tim_struct.TIM_Prescaler = 42000 - 1;
    tim_struct.TIM_ClockDivision = 0;
    tim_struct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim_struct);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvic_struct;
    nvic_struct.NVIC_IRQChannel = TIM2_IRQn;
    nvic_struct.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_struct.NVIC_IRQChannelSubPriority = 1;
    nvic_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_struct);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        b_ignore_interrupt = 0;
        if (b_saying_hello)
            say_hallo_timer_handle();
        else
            TIM_Cmd(TIM2, DISABLE);

        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        if (!b_ignore_interrupt)
        {
            current_handler_idx = (current_handler_idx + 1) % (sizeof(handlers) / sizeof(handlers[0]));
            b_ignore_interrupt = 1;
            b_saying_hello = 0;
            TIM_Cmd(TIM2, ENABLE);
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}