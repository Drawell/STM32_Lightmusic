#include "signal_processor.h"

#define MAX_SIGNAL_VOLUME(length) (16 * (length))
static uint32_t maximum_volume = 570; //1;
static uint32_t minimum_volume = 530; //MAX_SIGNAL_VOLUME(64);
static uint8_t iteration = 0;

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

uint32_t signal_volume(uint16_t *data, uint16_t length)
{
    if (data == 0)
        return 0;

    uint32_t result = 0;
    for (uint16_t i = 0; i < length; i++)
        result += calc_ones_arr[(uint8_t)(data[i] >> 8)] + calc_ones_arr[(uint8_t)(data[i])];

    return result;
}

uint8_t signal_volume_in_percent(uint16_t *data, uint16_t length)
{
    uint32_t volume = signal_volume(data, length);
    if (volume < minimum_volume)
        return 0;

    uint8_t percent = (volume - minimum_volume) * 100 / (maximum_volume - minimum_volume);
    return percent;
}
