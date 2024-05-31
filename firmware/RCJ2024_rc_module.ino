
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SSD1306.h>
#include <qrcodeoled.h>
#include "esp_mac.h"

#define BUTTON    9
#define I2C_SDA   6
#define I2C_SCL   7
#define DIG_OUT_1 20
#define DIG_OUT_2 19

#define BLINK_INTERVAL 500


BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
BLECharacteristic * pRxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String BLErxValue;
uint8_t txValue = 0;
SSD1306  display(0x3c, I2C_SDA, I2C_SCL);
QRcodeOled qrcode (&display);

bool robot_play = false;
bool ready_blink = false;
bool state_changed = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      BLErxValue = pCharacteristic->getValue();

      if (BLErxValue.length() > 0) {
        //        Serial.println("*********");
        //        Serial.print("Received Value: ");
        //        for (int i = 0; i < BLErxValue.length(); i++)
        //          Serial.print(BLErxValue[i]);
        //
        //        Serial.println();
        //        Serial.println("*********");

        switch (BLErxValue[0]) {
          case 'G':
            if (!robot_play) {
              state_changed = true;
            }
            robot_play = true;
            break;
          case 'S':
            if (robot_play) {
              state_changed = true;
            }
            robot_play = false;
            break;
          case 'R':
            if (!robot_play) {
              ready_blink = true;
            }
            break;
        }
      }
    }
};

uint32_t blink_start_time = 0;
bool blink_state = true;
void led_blink() {
  if (millis() - blink_start_time > BLINK_INTERVAL) {
    if (blink_state) {
      display.invertDisplay();
      blink_state = false;
    } else {
      display.normalDisplay();
      blink_state = true;
    }
    blink_start_time = millis();
  }
}

void connectScreen() {
  // Variable to store the MAC address
  uint8_t baseMac[6];
  char BLEmac[17];
  
  display.clear();

  esp_read_mac(baseMac, ESP_MAC_BT);
  sprintf(BLEmac, "%X:%X:%X:%X:%X:%X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

  qrcode.init(64, 64);
  qrcode.create(BLEmac);

  sprintf(BLEmac, "%X:%X:%X:\n%X:%X:%X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  display.setFont(ArialMT_Plain_10);
  display.drawString(96, 2, "Wait for\nconnection");
  display.drawString(96, 39, BLEmac);
  display.display();

  display.setFont(ArialMT_Plain_24);
}


void setup() {
  Serial.begin(9600);

  // Init GPIOs
  pinMode(DIG_OUT_1, OUTPUT);
  pinMode(DIG_OUT_2, OUTPUT);
  pinMode(BUTTON, INPUT);

  digitalWrite(DIG_OUT_1, HIGH);
  digitalWrite(DIG_OUT_2, HIGH);

  // Init display
  display.init();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  
  connectScreen();

  Serial.println("S");

  // Create the BLE Device
  BLEDevice::init("RCJ-rc");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  pTxCharacteristic->addDescriptor(new BLE2902());

  pRxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        BLECharacteristic::PROPERTY_WRITE
                      );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  //Serial.println("Waiting a client connection to notify...");
}

void loop() {

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    //Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;

    connectScreen();
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    state_changed = true;
  }

  if (state_changed) {
    state_changed = false;
    ready_blink = false;
    display.normalDisplay();
    if (robot_play) {
      Serial.println("G");
      display.clear();
      display.drawString(64, 18, "PLAY");
      display.display();
      digitalWrite(DIG_OUT_1, HIGH);
      digitalWrite(DIG_OUT_2, HIGH);
    } else {
      Serial.println("S");
      display.clear();
      display.drawString(64, 18, "STOP");
      display.display();
      digitalWrite(DIG_OUT_1, LOW);
      digitalWrite(DIG_OUT_2, LOW);

    }
  }

  if (ready_blink) {
    led_blink();
  }


}
