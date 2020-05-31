#ifndef SIGNAL_PROCESSOR
#define SIGNAL_PROCESSOR
#include <stm32f4xx.h>

#define ARM_MATH_CM4
#define VFP_FP
#include "arm_math.h"

/* FFT settings */
#define SAMPLES 512          /* 256 real parts and 256 imaginary parts */
#define FFT_SIZE SAMPLES / 2 /* FFT size is always the same size as we have samples, so 256 in our case */
// We want to get resolution of 45450 / 256 = 177.5 Hz
// It means we need to get signal value every 23 microsec

#define MAX_POSSIBLE_SIGNAL_VOLUME(length) (16 * (length))
#define HALF_SIGNAL_VOLUME(length) (8 * (length))
#define VOLUME_SCATTER(length) ((length) / 2)
#define MAX_SIGNAL_VOLUME(length) (HALF_SIGNAL_VOLUME(length) + VOLUME_SCATTER(length) * 2)
#define MIN_SIGNAL_VOLUME(length) (HALF_SIGNAL_VOLUME(length) - VOLUME_SCATTER(length) / 4)

/*!
    \brief Calculate signal volume counting ones  
    \param data signal data
    \param length data size
    \return signal volume
*/
uint32_t signal_volume(uint16_t *PDM_data, uint16_t length);

/*!
    \brief Calculate signal volume using maximum and minimum defined volume
    \param data signal data
    \param length data size
    \return signal volume in percent
*/
uint8_t signal_volume_in_percent(uint16_t *PDM_data, uint16_t length);

void FFT(uint16_t *PDM_data, uint16_t length, int32_t *freq_magnitudes, uint16_t freq_count);

#endif // SIGNAL_PROCESSOR