#ifndef MAIN_H
#define MAIN_H
#include <stm32f4xx.h>

/*!
    \brief Init LEDs
    Init GPIOD Pins 12, 13, 14, 15 with Out mode
 */
void init_leds(void);

/*!
    \brief Init SMD button
    Init GPIOA Pins 0 IN mode and init EXTI_Line0 interruption
 */
void init_button(void);

/*!
    \brief Init Timer
    Init TIM4 for PWM
 */
void init_pwm_timer(void);

/*!
    \brief Increase brightness
    Increase brightness for all LEDs or make it zero after maximum 
 */
void increase_brightness(void);


#endif // MAIN_H