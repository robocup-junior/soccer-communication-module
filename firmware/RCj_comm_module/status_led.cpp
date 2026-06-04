#include "status_led.h"

#include <Arduino.h>

#include "definitions.h"

void status_led_init()
{
    ledcAttach(RGB_LED_RED_GPIO, RGB_LED_PWM_HZ, RGB_LED_PWM_RESOLUTION);
    ledcAttach(RGB_LED_GREEN_GPIO, RGB_LED_PWM_HZ, RGB_LED_PWM_RESOLUTION);
    ledcAttach(RGB_LED_BLUE_GPIO, RGB_LED_PWM_HZ, RGB_LED_PWM_RESOLUTION);

    ledcWrite(RGB_LED_RED_GPIO, 0);
    ledcWrite(RGB_LED_GREEN_GPIO, 0);
    ledcWrite(RGB_LED_BLUE_GPIO, 0);
}

void status_led_set_play(bool play)
{
    ledcWrite(RGB_LED_RED_GPIO, play ? 0 : RGB_LED_PWM_DUTY);
    ledcWrite(RGB_LED_GREEN_GPIO, play ? RGB_LED_PWM_DUTY : 0);
    ledcWrite(RGB_LED_BLUE_GPIO, 0);
}
