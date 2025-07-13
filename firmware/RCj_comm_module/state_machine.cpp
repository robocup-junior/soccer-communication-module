#include <ctime>
#include "esp32-hal.h"
#include <sys/_stdint.h>
#include "WString.h"
#include <Arduino.h>
#include "HardwareSerial.h"
#include "freertos/projdefs.h"
#include <stdint.h>
#include "esp_err.h"

#include "definitions.h"
#include "display.h"
#include "state_machine.h"

static stm_states current_state = STM_INIT;
static bool state_changed = false;
static bool robot_play = false;
static uint32_t timer_stop = 0;

static uint16_t get_remaining_time() {
    uint32_t current_time = millis();
    if (current_time < timer_stop) {
        return (timer_stop - current_time) / 1000;
    }
    return 0;
}

static void update_output_satet() {
    if (robot_play) {
        digitalWrite(OUTPUT1_GPIO, HIGH);
        digitalWrite(OUTPUT2_GPIO, HIGH);
        Serial.println("PLAY");
    } else {
        digitalWrite(OUTPUT1_GPIO, LOW);
        digitalWrite(OUTPUT2_GPIO, LOW);
        Serial.println("STOP");
    }
}


// States
static void state_init() {
    display_screen_init();
    delay(2000);
    stm_set_state(STM_DISCONNECTED);
}

static void state_wait_connecting() {
    display_screen_wait_for_connection();
}

static void state_play() {
    robot_play = true;
    display_screen_play();
}

static void state_stop() {
    robot_play = false;
    display_screen_stop();
}

static void state_damage() {
    robot_play = false;
    display_screen_damage(get_remaining_time());

}

static void state_half_break() {
    robot_play = false;
    display_screen_half_break(get_remaining_time());
}

static void state_game_over() {
    robot_play = false;
    display_screen_game_over();
}



// Public functions
int8_t stm_set_timer(uint32_t miliseconds) {
    timer_stop = millis() + miliseconds;

    return ESP_OK;
}

int8_t stm_init() {
    stm_set_timer(660000); //DOTO why ist here this? WTF

    return ESP_OK;
}

void stm_set_state(stm_states state) {
    current_state = state;
    state_changed = true;
}

stm_states stm_get_state() {
    return current_state;
}

int8_t stm_update() {
    bool change_noted = false;

    if (state_changed) change_noted = true;

    switch (current_state) {
        case STM_INIT:
            state_init();
            break;
        case STM_DISCONNECTED:
            if (state_changed) {
                state_wait_connecting();
            }
            break;
        case STM_PLAY:
            state_play();
            break;
        case STM_STOP:
            state_stop();
            break;
        case STM_DAMAGE:
            state_damage();
            break; 
        case STM_HALF_TIME:
            state_half_break();
            break;
        case STM_GAME_OVER:
            state_game_over();
            break;             
        default:
            // statements
            break;
    }

    if (state_changed) {
        update_output_satet();
    }

    if (change_noted) state_changed = false;

    return ESP_OK;
}
