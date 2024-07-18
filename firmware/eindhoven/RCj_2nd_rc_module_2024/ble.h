#ifndef BLE_H
#define BLE_H

int8_t ble_start_server();

int8_t ble_disconnect();

int8_t ble_send_msg(uint8_t *data, size_t length);

#endif // BLE_H