#ifndef LED_MANAGER
#define LED_MANAGER
#include <stm32f4xx.h>

typedef enum
{
    RED,
    GREEN,
    BLUE,
    ORANGE
} LED_color_t;

/*!
    \brief Init LEDs
    Init GPIOD Pins 12, 13, 14, 15 with PWM mode with timer TIM4
 */
void init_LED_manager(void);


/*!
    \brief Set brightness for chosen LED
    \param led LED
    \param percent Percentage brightness
*/
void set_brightness(LED_color_t led, uint8_t percent);



#endif //LED_MANAGER