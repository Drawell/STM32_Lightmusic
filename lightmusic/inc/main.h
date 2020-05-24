#ifndef MAIN_H
#define MAIN_H
#include <stm32f4xx.h>
#include "led_manager.h"
#include "microphone_driver.h"
#include "controller.h"

/*!
    \brief Init SMD button
    Init GPIOA Pins 0 IN mode and init EXTI_Line0 interruption
 */
void init_button(void);

/*!
    \brief Increase brightness
    Increase brightness for all LEDs or make it zero after maximum 
 */
void increase_brightness(void);


#endif // MAIN_H