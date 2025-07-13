#include "HardwareSerial.h"
#include "esp32-hal.h"
#include "esp32-hal-gpio.h"
#include <sys/_stdint.h>
#include "WString.h"
#include "esp_mac.h"

#include "definitions.h"
#include "ble.h"
#include "ble_processing.h"
#include "functions.h"

#define DEBOUNCE_DELAY 50 //ms
#define DOUBLE_PRESS_MAX_DELAY 1000 //ms

static String module_indicator = DEFAULT_INDICATOR;
static uint8_t my_score = 0;
static uint8_t opponent_score = 0;

// Long press check
static bool button_pressed = false;
static uint32_t button_press_start_time = 0;

// Double press check
static bool double_button_state = false;
static bool double_last_button_state = false;
static bool double_waiting_second_press = false;
static uint32_t double_last_debounce_time = 0;
static uint32_t double_first_press_time = 0;



String BLE_MAC_to_string() {
    uint8_t base_mac[6];
    char BLE_mac[18];
    esp_read_mac(base_mac, ESP_MAC_BT);
    sprintf(BLE_mac, "%02X:%02X:%02X:%02X:%02X:%02X", base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);
    return String(BLE_mac);
}



int8_t module_set_indicator(String name) {
    module_indicator = name;
    return ESP_OK;
}

String module_get_indicator() {
    return module_indicator;
}

void module_set_my_score(uint8_t score) {
    my_score = score;
}

uint8_t module_get_my_score() {
    return my_score;
}

void module_set_opponent_score(uint8_t score) {
    opponent_score = score;
}

uint8_t module_get_opponent_score() {
    return opponent_score;
}

void module_init_gpios() {
    // Init GPIOs
    pinMode(OUTPUT1_GPIO, OUTPUT);
    pinMode(OUTPUT2_GPIO, OUTPUT);
    pinMode(BUTTON_GPIO, INPUT);
    pinMode(BUTTON2_GPIO, INPUT);
}

void check_disconnect_button() {
    if (button_pressed == true && millis() - button_press_start_time > DISCONNECT_HOLD_TIME) {
        button_pressed = false;
        ble_disconnect();
    } else if (digitalRead(BUTTON_GPIO) == LOW && button_pressed == false) {
        button_press_start_time = millis();
        button_pressed = true;
    } else if (digitalRead(BUTTON_GPIO) == HIGH && button_pressed == true) {
        button_pressed = false;
    }
}

void check_penalty_button() {

    bool reading = !digitalRead(BUTTON_GPIO) || !digitalRead(BUTTON2_GPIO);

    // debounce safety
    if (reading != double_last_button_state) {
        double_last_debounce_time = millis();
        double_last_button_state = reading;
    }


    if ((millis() - double_last_debounce_time) > DEBOUNCE_DELAY) {  // correct press
        if (reading != double_button_state) {
            double_button_state = reading;

            if (reading == 1) {
                if (double_waiting_second_press && (millis() - double_first_press_time <= DOUBLE_PRESS_MAX_DELAY)) {
                    // Double press detect
                    double_waiting_second_press = false; // reset
                    //Serial.println("Double Press Detected");
                    ble_msg_procesing_ask_for_penalty();
                } else {
                    // First press
                    double_first_press_time = millis();
                    double_waiting_second_press = true;
                }
            }
        }
    }

    // Double presss time out
    if (double_waiting_second_press && (millis() - double_first_press_time > DOUBLE_PRESS_MAX_DELAY)) {
        // Single press
        //Serial.println("Single Press Detected");
        double_waiting_second_press = false;
    }
}