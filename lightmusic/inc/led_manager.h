#ifndef LED_MANAGER
#define LED_MANAGER
#include <stm32f4xx.h>

#define PWM_MAX_PULSE 8399
#define PWM_MIN_PULSE 1
#define PERCENT_TO_PULSE(p) ((PWM_MAX_PULSE - PWM_MIN_PULSE) * (p) / 100)

typedef enum
{
    BLUE = 1,
    GREEN = 2,
    ORANGE = 4,
    RED = 8
} LED_color_t;

/*!
    \brief Init LEDs
    Init GPIOD Pins 12, 13, 14, 15 with PWM mode with timer TIM4
 */
void init_LED_manager(void);

/*!
    \brief Set brightness for chosen LEDs
    \param led LED as bitmap
    \param percent Percentage brightness
*/
void set_brightness(LED_color_t led, uint8_t percent);

/*!
    \brief Set brightness for chosen LEDs on maximum
    \param led LED as bitmap
*/
void turn_on_led(LED_color_t led);

/*!
    \brief Set brightness for chosen LEDs on mininum
    \param led LED as bitmap
*/
void turn_off_led(LED_color_t led);

#endif //LED_MANAGER