#include <sys/_stdint.h>
#ifndef BLE_PROCCESING_H
#define BLE_PROCCESING_H

// Message structure
typedef struct {
    uint8_t msg_id;
    uint8_t data[BLE_DATA_MAX_LENGTH];
    uint8_t data_length;

} ble_msg_t;

// BLE Massage IDs
enum ble_msg_id {
    BLE_MSG_PING,
    BLE_MSG_FW_VERSION,
    BLE_MSG_SET_NAME,
    BLE_MSG_SET_SCORE,
    BLE_MSG_PLAY,
    BLE_MSG_STOP,
    BLE_MSG_DAMAGE,
    BLE_MSG_HALF_BREAK,
    BLE_MSG_GAME_OVER,
    BLE_MSG_DISCONNECT,
    BLE_MSG_ASK_FOR_PENALTY,
    BLE_MSG_MAX_ID // Must be last
}; 

int8_t ble_msg_processing_init();

int8_t ble_msg_processing();

QueueHandle_t ble_msg_proccesing_get_queue();

void ble_msg_procesing_ask_for_penalty();

#endif // BLE_PROCCESING_H