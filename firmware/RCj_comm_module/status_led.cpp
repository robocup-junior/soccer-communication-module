#include "status_led.h"

#include <Arduino.h>

#include "definitions.h"

void status_led_init()
{
    pinMode(RGB_LED_RED_GPIO, OUTPUT);
    pinMode(RGB_LED_GREEN_GPIO, OUTPUT);
    pinMode(RGB_LED_BLUE_GPIO, OUTPUT);

    digitalWrite(RGB_LED_RED_GPIO, LOW);
    digitalWrite(RGB_LED_GREEN_GPIO, LOW);
    digitalWrite(RGB_LED_BLUE_GPIO, LOW);
}

void status_led_set_play(bool play)
{
    digitalWrite(RGB_LED_RED_GPIO, play ? LOW : HIGH);
    digitalWrite(RGB_LED_GREEN_GPIO, play ? HIGH : LOW);
    digitalWrite(RGB_LED_BLUE_GPIO, LOW);
}
