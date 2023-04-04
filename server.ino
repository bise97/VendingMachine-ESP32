#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Stepper.h>

const int sPR = 2048;  // numero di passi per un giro completo del motore
const int speedo=5; //velocià dei motori in rpm

Stepper stepper1(sPR, 5, 4, 2, 15);
Stepper stepper2(sPR, 13, 12, 14, 27);
Stepper stepper3(sPR, 23, 22, 21, 19);


// generatore di uuid: https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        if(value=="A1") {
          stepper1.step(sPR);
        }
        else if(value=="B2") {
          stepper2.step(sPR);
        }
        else if(value=="C3") {
          stepper3.step(sPR);
        }
        else Serial.println("ERRORE");
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  BLEDevice::init("Server");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Characteristic OK");
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
//  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
//  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  stepper1.setSpeed(speedo);
  stepper2.setSpeed(speedo);
  stepper3.setSpeed(speedo);
}

void loop() {
  delay(1000);
}
