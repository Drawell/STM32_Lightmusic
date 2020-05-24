#ifndef SIGNAL_PROCESSOR
#define SIGNAL_PROCESSOR
#include <stm32f4xx.h>

/*!
    \brief Calculate signal volume counting ones  
    \param data signal data
    \param length data size
    \return signal volume
*/
uint32_t signal_volume(uint16_t *data, uint16_t length);

/*!
    \brief Calculate signal volume using maximum and minimum defined volume
    \param data signal data
    \param length data size
    \return signal volume in percent
*/
uint8_t signal_volume_in_percent(uint16_t *data, uint16_t length);


#endif // SIGNAL_PROCESSOR