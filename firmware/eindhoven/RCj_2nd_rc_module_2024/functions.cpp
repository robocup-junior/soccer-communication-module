#include "HardwareSerial.h"
#include "esp32-hal.h"
#include "esp32-hal-gpio.h"
#include <sys/_stdint.h>
#include "WString.h"
#include "esp_mac.h"

#include "definitions.h"
#include "ble.h"
#include "functions.h"

static String module_indicator = DEFAULT_INDICATOR;
static uint8_t my_score = 0;
static uint8_t opponent_score = 0;
static bool button_pressed = false;
static uint32_t button_press_start_time = 0;


String BLE_MAC_to_string() {
    uint8_t base_mac[6];
    char BLE_mac[18];
    esp_read_mac(base_mac, ESP_MAC_BT);
    sprintf(BLE_mac, "%X:%X:%X:%X:%X:%X", base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);
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