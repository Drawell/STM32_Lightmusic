#include "signal_processor.h"

//static uint32_t maximum_volume = 570; //1;
//static uint32_t minimum_volume = 530; //MAX_SIGNAL_VOLUME(64);
static uint32_t volume_scatter = 50;

static const uint8_t calc_ones_arr[256] =
    {0, 1, 1, 3, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
     1, 2, 2, 4, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     1, 2, 2, 4, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     3, 4, 4, 6, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     1, 2, 2, 4, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     2, 3, 3, 5, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     2, 3, 3, 5, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     3, 4, 4, 6, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     1, 2, 2, 4, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     2, 3, 3, 5, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     2, 3, 3, 5, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     3, 4, 4, 6, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     2, 3, 3, 5, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     3, 4, 4, 6, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     3, 4, 4, 6, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     4, 5, 5, 7, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

uint32_t signal_volume(uint16_t *PDM_data, uint16_t length)
{
    if (PDM_data == 0)
        return 0;

    uint32_t result = 0;
    for (uint16_t i = 0; i < length; i++)
        result += calc_ones_arr[(uint8_t)(PDM_data[i] >> 8)] + calc_ones_arr[(uint8_t)(PDM_data[i])];

    return result;
}

uint8_t signal_volume_in_percent(uint16_t *PDM_data, uint16_t length)
{
    uint32_t volume = signal_volume(PDM_data, length);
    if (volume < MIN_SIGNAL_VOLUME(length))
        return 0;

    uint8_t percent = (volume - MIN_SIGNAL_VOLUME(length)) * 100 / (MAX_SIGNAL_VOLUME(length) - MIN_SIGNAL_VOLUME(length));
    return percent;
}


/* Global variables */
float32_t Input[SAMPLES];
float32_t Output[FFT_SIZE];

//static float32_t complex_data[SAMPLES];
static uint16_t complex_data_point = 0;

static uint8_t collect_complex_data(uint16_t *PDM_data, uint16_t length)
{
    for (int i = 0; i < length; i ++)
    {
        // Real part, make offset by ADC / 2
        Input[complex_data_point] = calc_ones_arr[(uint8_t)(PDM_data[i] >> 8)] + calc_ones_arr[(uint8_t)PDM_data[i]];
        // + calc_ones_arr[(uint8_t)(data[i + 1] >> 8)] + calc_ones_arr[(uint8_t)data[i + 1]];
        // Imaginary part
        Input[complex_data_point + 1] = 0;
        complex_data_point += 2;
        if (complex_data_point >= SAMPLES)
        {
            complex_data_point = 0;
            return 1;
        }
    }

    return 0;
}

void FFT(uint16_t *PDM_data, uint16_t length, int32_t *freq_magnitudes, uint16_t freq_count)
{
    if (!collect_complex_data(PDM_data, length))
        return;

    arm_cfft_instance_f32 S; //ARM CFFT module
    
        // Initialization function for the floating-point, intFlag = 0, doBitReverse = 1
    arm_cfft_radix4_init_f32(&S, FFT_SIZE, 0, 1);

    //Processing function for the floating-point Radix-4    
    arm_cfft_radix4_f32(&S, Input);

    // Process the data through the Complex Magniture Module for calculating the magnitude at each bin
    arm_cmplx_mag_f32(Input, Output, FFT_SIZE);

    // Output[0] = Signals DC value
    uint16_t samples_per_freq = FFT_SIZE / freq_count / 2;

    for(uint16_t i = 0; i < freq_count; i++)
    {        
        for (uint16_t j = 1; j < samples_per_freq; j++)
        {
            freq_magnitudes[i] += (int32_t)Output[i * samples_per_freq + j];
        }
    }
}