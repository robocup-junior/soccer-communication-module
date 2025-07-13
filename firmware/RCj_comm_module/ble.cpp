#include <cstddef>
#include <Arduino.h>
#include "HardwareSerial.h"
#include <sys/_stdint.h>
#include "WString.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "definitions.h"
#include "functions.h"
#include "state_machine.h"
#include "ble_processing.h"
#include "ble.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

static BLEServer *pServer;
static BLECharacteristic *pRxCharacteristic;
static BLECharacteristic *pTxCharacteristic;
static bool device_connected = false;
static String receive_data;
static QueueHandle_t ble_msg_queue;


// Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override {
        //Serial.println("Client connected");
        device_connected = true;
        //stm_set_state(STM_PLAY);
    }

    void onDisconnect(BLEServer* pServer) override {
        //Serial.println("Client disconnected");
        device_connected = false;
        stm_set_state(STM_DISCONNECTED);
        // Start advertising
        pServer->getAdvertising()->start();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        // Get received data
        receive_data = pCharacteristic->getValue();

        if (pCharacteristic == pRxCharacteristic) {
            if (receive_data.length() > 0 && receive_data.length() < BLE_DATA_MAX_LENGTH + 1) {
                ble_msg_t msg;
                msg.msg_id = receive_data[0];
                msg.data_length = receive_data.length() - 1;  // -1 for MSG ID
                for (uint8_t i = 0; i < receive_data.length() - 1; i++) {
                    msg.data[i] = receive_data[i+1];
                }
                xQueueSend(ble_msg_queue, (void *) &msg, 0);
            }
        }
    }
};

int8_t ble_start_server() {

    ble_msg_processing_init();

    // Assign message queue
    ble_msg_queue = ble_msg_proccesing_get_queue();

    // Create the BLE Device
    BLEDevice::init(BLE_NAME);

    // Create the BLE Server
    pServer = BLEDevice::createServer();

    // Server callback
    pServer->setCallbacks(new MyServerCallbacks());
  
    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create write characteristic
    pRxCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID_RX,
                                            BLECharacteristic::PROPERTY_WRITE
                                            );  
    pRxCharacteristic->setCallbacks(new MyCallbacks()); 

    // Create notify (read) characteristic
    pTxCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID_TX,
                                            BLECharacteristic::PROPERTY_INDICATE
                                            );
    pTxCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();

    return ESP_OK;
}

int8_t ble_disconnect() {
    if (device_connected) {
        ble_msg_t send_msg;

        send_msg.msg_id = BLE_MSG_DISCONNECT;
        ble_send_msg((uint8_t*) &send_msg, 1);

        pServer->disconnect(pServer->getConnId());
    }
    return ESP_OK;
}

int8_t ble_send_msg(uint8_t *data, size_t length) {
    pTxCharacteristic->setValue(data, length);
    pTxCharacteristic->indicate();
    //Serial.println("Data send");

    return ESP_OK;
}


