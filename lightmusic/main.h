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
    Init GPIOA Pins 0 IN mode and init EXTI_Line0 interruption
 */
void init_timer(void);

/*!
    \brief Turn on next LED    
 */
void switch_LED();



#endif // MAIN_H