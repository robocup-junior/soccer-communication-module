#include <Arduino.h>

#include "ble.h"
#include "definitions.h"
#include "ble_processing.h"
#include "display.h"
#include "functions.h"
#include "serial_status.h"
#include "state_machine.h"

static void module_setup()
{
    Serial.begin(UART_SPEED);
    serial_status_init();

    display_init();
    module_init_gpios();
    stm_init();
    ble_start_server();
}

static void module_loop()
{
    ble_msg_processing();
    stm_update();
    check_disconnect_button();
    check_penalty_button();
}

extern "C" void app_main(void)
{
    initArduino();
    module_setup();

    while (true) {
        module_loop();
        delay(1);
    }
}
