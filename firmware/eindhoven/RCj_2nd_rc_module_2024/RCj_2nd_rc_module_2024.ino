#include <Arduino.h>
#include "definitions.h"
#include "ble.h"
#include "functions.h"
#include "state_machine.h"
#include "ble_processing.h"
#include "display.h"

void setup() {
    Serial.begin(UART_SPEED);
    
    // Init display
    display_init();

    module_init_gpios();

    stm_init();   

    ble_start_server();
}

void loop() {

    ble_msg_processing();

    stm_update();

    check_disconnect_button();

}
