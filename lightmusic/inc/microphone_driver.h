#ifndef MICROPHONE_DRIVER
#define MICROPHONE_DRIVER
#include <stm32f4xx.h>

#define INTERNAL_BUFF_SIZE 256

typedef void (*callback_t)(uint16_t*, uint16_t);


/*!
    \brief Init microphone
    \param interrupt_handler function which called when DMA buffer is full
    Init on-board microphone and DMA for write signal.
    Call interrupt_handler function when DMA buffer is full
*/
void init_microphone_driver(callback_t interrupt_handler);



#endif //MICROPHONE_DRIVER