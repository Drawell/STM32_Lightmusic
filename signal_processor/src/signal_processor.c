#include "signal_processor.h"

static uint8_t listening_for_repeated_freq();
static void insertion_sort(uint16_t arr[], uint8_t start, uint8_t end);
static uint32_t signal_volume(uint16_t *PDM_data, uint16_t length);
static uint8_t signal_volume_in_percent(uint16_t *PDM_data, uint16_t length);
static uint8_t collect_complex_data(uint16_t *PDM_data, uint16_t length);
static uint8_t FFT(uint16_t *PDM_data, uint16_t length);

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

/* Global variables */
static float32_t Input[SAMPLES];
static float32_t Output[FFT_SIZE];
static uint16_t Freq[FREQ_COUNT];
static uint16_t Memory[FREQ_COUNT];
static uint16_t complex_data_point = 0;

void switch_mod(void)
{
    complex_data_point = 0;
    memset(Memory, 0, sizeof(Memory) * sizeof(Memory[0]));
}

uint8_t simple_sound_loudness(uint16_t *PDM_data, uint16_t length, uint8_t *brightness_per_led, uint8_t led_count)
{
    uint8_t volume = signal_volume_in_percent(PDM_data, length);
    uint16_t max_volume_percent_per_led = 100 / led_count;
    uint8_t led_volume;
    for (uint8_t i = 0; i < led_count; i++)
    {
        if (volume > max_volume_percent_per_led)
        {
            led_volume = 100;
            volume -= max_volume_percent_per_led;
        }
        else
        {
            led_volume = volume * led_count;
            volume = 0;
        }
        brightness_per_led[i] = led_volume;
    }
    return 1;
}

uint8_t burn_all_divide_by_led_count(uint16_t *PDM_data, uint16_t length, uint8_t *brightness_per_led, uint8_t led_count)
{
    if (PDM_data == 0 || !FFT(PDM_data, length))
        return 0;

    uint8_t freq_per_led = FREQ_COUNT / led_count;
    uint32_t noise_threshold = 0;
    uint8_t i;
    uint8_t j;
    for (i = 0; i < FREQ_COUNT; i++)
        noise_threshold += Freq[i];

    noise_threshold /= FREQ_COUNT / 2;

    uint8_t current_led_idx = 0;

    uint32_t max_brightness = 0;
    uint32_t min_brightness = 0;
    uint32_t brightness = 0;
    for (i = 0; i < led_count; i++)
    {
        brightness = 0;
        for (j = 0; j < freq_per_led; j++)
        {
            if (Freq[i * freq_per_led + j] > noise_threshold)
                brightness += Freq[i * freq_per_led + j];
        }
        if (brightness > max_brightness)
            max_brightness = brightness;

        if (min_brightness == 0 || min_brightness > brightness)
            min_brightness = brightness;
    }

    if (min_brightness == max_brightness)
        return 0;

    for (i = 0; i < led_count; i++)
    {
        brightness = 0;
        for (j = 0; j < freq_per_led; j++)
        {
            if (Freq[i * freq_per_led + j] > noise_threshold)
                brightness += Freq[i * freq_per_led + j];
        }

        uint8_t brightness_percent = (brightness - min_brightness) * 100 / (max_brightness - min_brightness);
        brightness_percent = (brightness_percent * brightness_percent) / 100;
        brightness_per_led[i] = brightness_percent;
    }

    return 1;
}

uint8_t burn_after_adapt_median_treshold(uint16_t *PDM_data, uint16_t length, uint8_t *brightness_per_led, uint8_t led_count)
{
    // uses Memory as counter
    if (PDM_data == 0 || !FFT(PDM_data, length))
        return 0;

    uint8_t freq_per_led = FREQ_COUNT / led_count;
    uint32_t noise_threshold;
    uint8_t led_idx, freq_idx, j;
    for (led_idx = 0; led_idx < led_count; led_idx++)
    {
        noise_threshold = 0;
        insertion_sort(Freq, freq_per_led * led_idx, freq_per_led * (led_idx + 1));
        noise_threshold = Freq[freq_per_led * led_idx + freq_per_led / 2];

        for (j = 0; j < freq_per_led; j++)
        {
            freq_idx = freq_per_led * led_idx + j;
            brightness_per_led[led_idx] = 0;
            if (Freq[freq_idx] > 3 * noise_threshold)
            {
                Memory[freq_idx] = 20;
            }
            else
            {
                Memory[freq_idx] = Memory[freq_idx] < 1 ? 0 : Memory[freq_idx] - 1;
            }

            if (Memory[freq_idx])
            {
                brightness_per_led[led_idx] = 100;
            }
        }
    }

    return 1;
}

uint8_t burn_after_artificial_treshold(uint16_t *PDM_data, uint16_t length, uint8_t *brightness_per_led, uint8_t led_count)
{
    // uses Memory as counter
    if (PDM_data == 0 || !FFT(PDM_data, length))
        return 0;

    uint8_t freq_per_led = FREQ_COUNT / led_count;
    uint32_t noise_threshold;
    uint8_t led_idx, freq_idx, j;
    for (led_idx = 0; led_idx < led_count; led_idx++)
    {
        for (j = 0; j < freq_per_led; j++)
        {
            freq_idx = freq_per_led * led_idx + j;
            brightness_per_led[led_idx] = 0;
            if (Freq[freq_idx] > ARTIFICIAL_TRESHOLD)
            {
                Memory[freq_idx] = 20;
            }
            else
            {
                Memory[freq_idx] = Memory[freq_idx] < 1 ? 0 : Memory[freq_idx] - 1;
            }

            if (Memory[freq_idx])
            {
                brightness_per_led[led_idx] = 100;
            }
        }
    }

    return 1;
}

uint8_t smooth_changing_adapt_treshold(uint16_t *PDM_data, uint16_t length, uint8_t *brightness_per_led, uint8_t led_count)
{
    // uses Memory for remembering early brightness power
    if (PDM_data == 0 || !FFT(PDM_data, length))
        return 0;

    uint8_t freq_per_led = FREQ_COUNT / led_count;
    uint32_t noise_threshold;
    uint8_t led_idx, freq_idx, j;
    for (led_idx = 0; led_idx < led_count; led_idx++)
    {
        noise_threshold = 0;
        insertion_sort(Freq, freq_per_led * led_idx, freq_per_led * (led_idx + 1));
        noise_threshold = Freq[freq_per_led * led_idx + freq_per_led / 2];

        for (j = 0; j < freq_per_led; j++)
        {
            freq_idx = freq_per_led * led_idx + j;
            brightness_per_led[led_idx] = 0;
            if (Freq[freq_idx] > (7 * noise_threshold) / 2)
                Memory[freq_idx] = 100;
            else if (Freq[freq_idx] > (13 * noise_threshold) / 4)
                Memory[freq_idx] = Memory[freq_idx] > 60 ? Memory[freq_idx] : 60;
            else
                Memory[freq_idx] = Memory[freq_idx] < FADING_RATE ? 0 : Memory[freq_idx] - FADING_RATE;

            if (brightness_per_led[led_idx] < Memory[freq_idx])
                brightness_per_led[led_idx] = Memory[freq_idx];
        }
    }

    return 1;
}

uint8_t smooth_changing_high_treshold(uint16_t *PDM_data, uint16_t length, uint8_t *brightness_per_led, uint8_t led_count)
{
    if (PDM_data == 0 || !FFT(PDM_data, length))
        return 0;

    uint8_t freq_per_led = FREQ_COUNT / led_count;
    uint32_t noise_threshold;
    uint8_t led_idx, freq_idx, j;
    for (led_idx = 0; led_idx < led_count; led_idx++)
    {
        noise_threshold = 0;
        insertion_sort(Freq, freq_per_led * led_idx, freq_per_led * (led_idx + 1));
        noise_threshold = Freq[freq_per_led * (led_idx + 1) - 2];

        for (j = 0; j < freq_per_led; j++)
        {
            freq_idx = freq_per_led * led_idx + j;
            brightness_per_led[led_idx] = 0;
            if (Freq[freq_idx] > 2 * noise_threshold)
                Memory[freq_idx] = 100;
            else if (Freq[freq_idx] > (3 * noise_threshold) / 2)
                Memory[freq_idx] = Memory[freq_idx] > 20 ? Memory[freq_idx] : 20;
            else
                Memory[freq_idx] = Memory[freq_idx] < FADING_RATE ? 0 : Memory[freq_idx] - FADING_RATE;

            if (brightness_per_led[led_idx] < Memory[freq_idx])
                brightness_per_led[led_idx] = Memory[freq_idx];
        }
    }

    return 1;
}

// static fuctions

static void insertion_sort(uint16_t arr[], uint8_t start, uint8_t end)
{
    uint8_t i, j;
    uint16_t key;

    for (i = start + 1; i < end; i++)
    {
        key = arr[i];
        j = i - 1;
        while (j >= start && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

static uint32_t signal_volume(uint16_t *PDM_data, uint16_t length)
{
    if (PDM_data == 0)
        return 0;

    uint32_t result = 0;
    for (uint16_t i = 0; i < length; i++)
        result += calc_ones_arr[(uint8_t)(PDM_data[i] >> 8)] + calc_ones_arr[(uint8_t)(PDM_data[i])];

    return result;
}

static uint8_t signal_volume_in_percent(uint16_t *PDM_data, uint16_t length)
{
    uint32_t volume = signal_volume(PDM_data, length);
    if (volume < MIN_SIGNAL_VOLUME(length))
        return 0;

    uint8_t percent = (volume - MIN_SIGNAL_VOLUME(length)) * 100 / (MAX_SIGNAL_VOLUME(length) - MIN_SIGNAL_VOLUME(length));
    return percent;
}

static uint8_t collect_complex_data(uint16_t *PDM_data, uint16_t length)
{
    for (int i = 0; i < length; i += 2)
    {
        Input[complex_data_point] = calc_ones_arr[(uint8_t)(PDM_data[i] >> 8)] + calc_ones_arr[(uint8_t)PDM_data[i]] + calc_ones_arr[(uint8_t)(PDM_data[i + 1] >> 8)] + calc_ones_arr[(uint8_t)PDM_data[i + 1]];
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

static uint8_t FFT(uint16_t *PDM_data, uint16_t length)
{
    if (!collect_complex_data(PDM_data, length))
        return 0;

    arm_cfft_instance_f32 S; //ARM CFFT module

    // Initialization function for the floating-point, intFlag = 0, doBitReverse = 1
    if (arm_cfft_radix4_init_f32(&S, FFT_SIZE, 0, 1) != ARM_MATH_SUCCESS)
        return 0;

    //Processing function for the floating-point Radix-4
    arm_cfft_radix4_f32(&S, Input);

    // Process the data through the Complex Magnitude Module for calculating the magnitude at each bin
    arm_cmplx_mag_f32(Input, Output, FFT_SIZE);

    // Output[0] = Signals DC value
    for (uint8_t i = 0; i < FREQ_COUNT; i++)
        Freq[i] = (uint16_t)Output[i + 1];

    return 1;
}