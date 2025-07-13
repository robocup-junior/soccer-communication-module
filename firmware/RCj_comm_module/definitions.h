#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define FW_VERSION_MAJOR    0
#define FW_VERSION_MINOR    91
#define FW_VERSION          (FW_VERSION_MAJOR*0xFF + FW_VERSION_MINOR)

#define DEFAULT_INDICATOR   "--"

// UART
#define UART_SPEED      115200

// BLE
#define BLE_NAME            "RCJ-soccer_module"
#define BLE_DATA_MAX_LENGTH 10
#define BLE_QUEUE_MAX_SIZE  5

// Disconnect button hold time
#define DISCONNECT_HOLD_TIME    5000

/*** GPIOs ***/
// I2C
#define I2C_SDA_GPIO    6
#define I2C_SCL_GPIO    7

// Button
#define BUTTON_GPIO     18
#define BUTTON2_GPIO     9 

// Status pins
#define OUTPUT1_GPIO    20
#define OUTPUT2_GPIO    19

#endif // DEFINITIONS_H
