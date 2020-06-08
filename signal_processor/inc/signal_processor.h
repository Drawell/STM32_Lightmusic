#ifndef SIGNAL_PROCESSOR
#define SIGNAL_PROCESSOR
#include <stm32f4xx.h>

#define ARM_MATH_CM4
#define VFP_FP
#include "arm_math.h"

/* FFT settings */
#define SAMPLES 512          /* 256 real parts and 256 imaginary parts */
#define FFT_SIZE SAMPLES / 2 /* FFT size is always the same size as we have samples, so 256 in our case */
#define FREQ_COUNT 41// only 20HZ - 5kHZ make sense

#define MAX_POSSIBLE_SIGNAL_VOLUME(length) (16 * (length))
#define HALF_SIGNAL_VOLUME(length) (8 * (length))
#define VOLUME_SCATTER(length) ((length) / 2)
#define MAX_SIGNAL_VOLUME(length) (HALF_SIGNAL_VOLUME(length) + VOLUME_SCATTER(length) * 2)
#define MIN_SIGNAL_VOLUME(length) (HALF_SIGNAL_VOLUME(length) - VOLUME_SCATTER(length) / 4)

#define FADING_RATE 2
#define ARTIFICIAL_TRESHOLD 30

/*!
    \brief Use for correct mod switching
*/
void switch_mod(void);

/*!
    \brief Then more loud then more LEDs be bright
    \param data PDM signal data
    \param length data size
    \param brightness_per_led pointer to data to fill with percent of LED brightness
    \param led_count LED count
    \return always 1
*/
uint8_t simple_sound_loudness(uint16_t *PDM_data, uint16_t length, uint8_t* brightness_per_led, uint8_t led_count);

/*!
    \brief Divide all frequency diapason by led_count and calculate brightness for each LED
    \param data PDM signal data
    \param length data size
    \param brightness_per_led pointer to data to fill with percent of LED brightness
    \param led_count LED count
    \return returns 0 if data size is not enough for FFT and need call function one more time
*/
uint8_t burn_all_divide_by_led_count(uint16_t *PDM_data, uint16_t length, uint8_t* brightness_per_led, uint8_t led_count);


/*!
    \brief Set 100% brightness when treshold is passed and hold it for a time
    \param data PDM signal data
    \param length data size
    \param brightness_per_led pointer to data to fill with percent of LED brightness
    \param led_count LED count
    \return returns 0 if data size is not enough for FFT and need call function one more time

    LED burn after treshold which is 3 * median of spectral components
*/

uint8_t burn_after_adapt_median_treshold(uint16_t *PDM_data, uint16_t length, uint8_t* brightness_per_led, uint8_t led_count);

/*!
    \brief Set 100% brightness when treshold is passed and hold it for a time
    \param data PDM signal data
    \param length data size
    \param brightness_per_led pointer to data to fill with percent of LED brightness
    \param led_count LED count
    \return returns 0 if data size is not enough for FFT and need call function one more time

    LED burn after treshold which constant value ARTIFICIAL_TRESHOLD
*/
uint8_t burn_after_artificial_treshold(uint16_t *PDM_data, uint16_t length, uint8_t* brightness_per_led, uint8_t led_count);

/*!
    \brief Increase or decrease brightness considering spectral component power
    \param data PDM signal data
    \param length data size
    \param brightness_per_led pointer to data to fill with percent of LED brightness
    \param led_count LED count
    \return returns 0 if data size is not enough for FFT and need call function one more time

    LED burn after treshold which is 3.5 * median of spectral components
*/
uint8_t smooth_changing_adapt_treshold(uint16_t *PDM_data, uint16_t length, uint8_t* brightness_per_led, uint8_t led_count);

/*!
    \brief Increase or decrease brightness considering spectral component power
    \param data PDM signal data
    \param length data size
    \param brightness_per_led pointer to data to fill with percent of LED brightness
    \param led_count LED count
    \return returns 0 if data size is not enough for FFT and need call function one more time

    LED burn after treshold which is 2 * pre-most high of spectral components
*/
uint8_t smooth_changing_high_treshold(uint16_t *PDM_data, uint16_t length, uint8_t* brightness_per_led, uint8_t led_count);

#endif // SIGNAL_PROCESSOR