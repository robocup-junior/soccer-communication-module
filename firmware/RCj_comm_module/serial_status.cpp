#include "serial_status.h"

#include <Arduino.h>
#include <string.h>

#include "driver/usb_serial_jtag.h"

static bool usb_serial_jtag_ready = false;

void serial_status_init()
{
    if (usb_serial_jtag_is_driver_installed()) {
        usb_serial_jtag_ready = true;
        return;
    }

    usb_serial_jtag_driver_config_t usb_config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    usb_serial_jtag_ready = usb_serial_jtag_driver_install(&usb_config) == ESP_OK;
}

void serial_status_println(const char *message)
{
    Serial.println(message);

    if (!usb_serial_jtag_ready) {
        return;
    }

    usb_serial_jtag_write_bytes(message, strlen(message), 0);
    usb_serial_jtag_write_bytes("\r\n", 2, 0);
}
