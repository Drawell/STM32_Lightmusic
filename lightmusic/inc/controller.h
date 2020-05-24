#ifndef CONTROLLER
#define CONTROLLER
#include <stm32f4xx.h>
#include <signal_processor.h>
#include "led_manager.h"

/*!
    \brief Init controlling program part
    Init on-board button, timer TIM2 and interrupts for work controll
*/
void init_controller(void);

/*!
    \brief Handles microphone signal   
    \param data signal data
    \param length data size
*/
void microphone_interrupt_handler(uint16_t *data, uint16_t length);


#endif //CONTROLLER