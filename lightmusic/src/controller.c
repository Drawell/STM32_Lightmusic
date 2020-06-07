#include "controller.h"

static void hello_said(void);
static void say_hello_timer_handle();
static void init_button(void);
static void init_timer(void);

static signal_process_function_t signal_processors[] = {
    //burn_all_divide_by_led_count,
    //simple_sound_loudness,
    burn_after_treshold,
    three_tresholds,
    smooth_changing,
    burn_all_divide_by_led_count};

static uint8_t current_func_idx;

static uint8_t b_ignore_interrupt;
static uint8_t say_hello_counter;
static uint8_t b_saying_hello;
static uint8_t b_in_process;

static LED_color_t leds[] = {BLUE, GREEN, ORANGE, RED};
static uint8_t brightness_per_led[LED_COUNT];

void init_controller(void)
{
    current_func_idx = 0;
    b_ignore_interrupt = 0;
    b_saying_hello = 0;
    b_in_process = 0;
    init_button();
    init_timer();
    init_LED_manager();
    init_microphone_driver(&microphone_interrupt_handler);
    say_hello(3);
}

void say_hello(uint8_t times)
{
    say_hello_counter = times * 4;
    b_saying_hello = 1;
    b_in_process = 1;
    TIM_Cmd(TIM2, ENABLE);
}

static void hello_said(void)
{
    b_saying_hello = 0;
    b_in_process = 0;
    TIM_Cmd(TIM2, DISABLE);
    
    start_record();
}

static void say_hello_timer_handle()
{
    if (say_hello_counter % 4 >= 2)
        turn_off_led(BLUE | GREEN | ORANGE | RED);
    else
        turn_on_led(BLUE | GREEN | ORANGE | RED);

    if (say_hello_counter == 0)
    {
        hello_said();
    }

    say_hello_counter--;
}

void microphone_interrupt_handler(uint16_t *data, uint16_t length)
{
    if (b_saying_hello)
        return;
    if (b_in_process)
        return;

    b_in_process = 1;
    if (signal_processors[current_func_idx](data, length, brightness_per_led, LED_COUNT))
        for (uint8_t i = 0; i < LED_COUNT; i++)
            set_brightness(leds[i], brightness_per_led[i]);
    b_in_process = 0;
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
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        b_ignore_interrupt = 0;
        if (b_saying_hello)
            say_hello_timer_handle();
        else
            TIM_Cmd(TIM2, DISABLE);
    }
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line0);
        if (!b_ignore_interrupt)
        {
            switch_mod();
            current_func_idx = (current_func_idx + 1) % (sizeof(signal_processors) / sizeof(signal_processors[0]));
            b_ignore_interrupt = 1;
            b_saying_hello = 0;
            TIM_Cmd(TIM2, ENABLE);
        }
    }
}