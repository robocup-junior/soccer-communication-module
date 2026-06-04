#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define FW_VERSION_MAJOR    0
#define FW_VERSION_MINOR    97
#define FW_VERSION          (FW_VERSION_MAJOR*0xFF + FW_VERSION_MINOR)

#define DEFAULT_INDICATOR   "--"

// UART
#define UART_SPEED      115200

// BLE
#define BLE_NAME            "RCJs-m_" + BLE_MAC_to_string()
#define BLE_DATA_MAX_LENGTH 10
#define BLE_QUEUE_MAX_SIZE  16
#define BLE_CONN_INTERVAL_MIN 12     // 15 ms, units are 1.25 ms
#define BLE_CONN_INTERVAL_MAX 24     // 30 ms, units are 1.25 ms
#define BLE_CONN_LATENCY      0
#define BLE_CONN_TIMEOUT      300    // 3 s, units are 10 ms

// Disconnect button hold time
#define DISCONNECT_HOLD_TIME    5000

/*** GPIOs (ESP32-C5) ***/
// The legacy ESP32-C6 (2024 board) pin map lives on the `legacy/esp32-c6` branch.
// I2C
#define I2C_SDA_GPIO    2
#define I2C_SCL_GPIO    3

// Button
#define BUTTON_GPIO     10
#define BUTTON2_GPIO     7

// Status pins
#define OUTPUT1_GPIO    9
#define OUTPUT2_GPIO    8

// Passive buzzer
#define BUZZER_GPIO             26
#define BUZZER_FREQUENCY_HZ     2700
#define BUZZER_DURATION_MS      80

#endif // DEFINITIONS_H
