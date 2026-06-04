#include "buzzer.h"

#include <Arduino.h>

#include "definitions.h"

static bool buzzer_ready = false;
static bool buzzer_active = false;
static uint32_t buzzer_stop_time = 0;

void buzzer_init()
{
    pinMode(BUZZER_GPIO, OUTPUT);
    digitalWrite(BUZZER_GPIO, LOW);

    buzzer_ready = ledcAttach(BUZZER_GPIO, BUZZER_FREQUENCY_HZ, 10);

    if (buzzer_ready) {
        ledcWriteTone(BUZZER_GPIO, 0);
    }
}

void buzzer_notify_state_change()
{
    if (!buzzer_ready) {
        return;
    }

    ledcWriteTone(BUZZER_GPIO, BUZZER_FREQUENCY_HZ);
    buzzer_stop_time = millis() + BUZZER_DURATION_MS;
    buzzer_active = true;
}

void buzzer_update()
{
    if (!buzzer_active || (int32_t)(millis() - buzzer_stop_time) < 0) {
        return;
    }

    ledcWriteTone(BUZZER_GPIO, 0);
    buzzer_active = false;
}
