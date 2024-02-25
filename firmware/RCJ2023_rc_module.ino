#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define LED_RED   0
#define LED_GREEN 10
#define LED_BLUE  1
#define LED_IR    2
#define BUTTON    3
#define I2C_SDA   6
#define I2C_SCL   7
#define DIG_OUT_1 5
#define DIG_OUT_2 4


BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
BLECharacteristic * pRxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string BLErxValue;
uint8_t txValue = 0;

bool robot_play = false;
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
        }
      }
    }
};


void setup() {
  Serial.begin(9600);

  // Init GPIOs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_IR, OUTPUT);
  pinMode(DIG_OUT_1, OUTPUT);
  pinMode(DIG_OUT_2, OUTPUT);
  pinMode(BUTTON, INPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_IR, LOW);
  digitalWrite(DIG_OUT_1, HIGH);
  digitalWrite(DIG_OUT_2, HIGH);

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
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, HIGH);
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    state_changed = true;
  }

  if (state_changed) {
    state_changed = false;
    if (robot_play) {
      Serial.println("G");
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(DIG_OUT_1, HIGH);
      digitalWrite(DIG_OUT_2, HIGH);
    } else {
      Serial.println("S");
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(DIG_OUT_1, LOW);
      digitalWrite(DIG_OUT_2, LOW);
    }
  }


}
