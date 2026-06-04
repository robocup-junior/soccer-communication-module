set(APP_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(OLED_DIR "${APP_DIR}/libraries/ESP8266_and_ESP32_OLED_driver_for_SSD1306_displays/src")
set(QRCODE_DISPLAY_DIR "${APP_DIR}/libraries/QRcodeDisplay/src")
set(QRCODE_OLED_DIR "${APP_DIR}/libraries/QRcodeOled-2.0.0/src")

idf_component_register(
    SRCS
        "app_main.cpp"
        "${APP_DIR}/ble.cpp"
        "${APP_DIR}/ble_processing.cpp"
        "${APP_DIR}/display.cpp"
        "${APP_DIR}/functions.cpp"
        "${APP_DIR}/serial_status.cpp"
        "${APP_DIR}/state_machine.cpp"
        "${OLED_DIR}/OLEDDisplay.cpp"
        "${OLED_DIR}/OLEDDisplayUi.cpp"
        "${QRCODE_DISPLAY_DIR}/frame-v1.c"
        "${QRCODE_DISPLAY_DIR}/frame-v2.c"
        "${QRCODE_DISPLAY_DIR}/frame-v3.c"
        "${QRCODE_DISPLAY_DIR}/frame-v4.c"
        "${QRCODE_DISPLAY_DIR}/frame-v5.c"
        "${QRCODE_DISPLAY_DIR}/frame-v6.c"
        "${QRCODE_DISPLAY_DIR}/frame-v7.c"
        "${QRCODE_DISPLAY_DIR}/frame-v8.c"
        "${QRCODE_DISPLAY_DIR}/frame-v9.c"
        "${QRCODE_DISPLAY_DIR}/frame-v10.c"
        "${QRCODE_DISPLAY_DIR}/qrcodedisplay.cpp"
        "${QRCODE_DISPLAY_DIR}/qrencode.c"
        "${QRCODE_OLED_DIR}/qrcodeoled.cpp"
    INCLUDE_DIRS
        "."
        "${APP_DIR}"
        "${OLED_DIR}"
        "${QRCODE_DISPLAY_DIR}"
        "${QRCODE_OLED_DIR}"
    REQUIRES
        espressif__arduino-esp32
        esp_driver_usb_serial_jtag
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=overloaded-virtual)
