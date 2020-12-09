#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

BLEService eMOBService("1809");
BLEStringCharacteristic accelerometerChar("107b", BLERead | BLENotify, 20);
// BLEStringCharacteristic gyroscopeChar("107b", BLERead | BLENotify, 50);
// BLEStringCharacteristic magnetometerChar("107b", BLERead | BLENotify, 50);
bool isConnected = false;
float Ax = 0.0, Ay = 0.0, Az = 0.0;
float Gx, Gy, Gz;
float Mx, My, Mz;
unsigned long last_t=micros();
unsigned long actual_t=micros();

void bleConnectedHandler(BLEDevice central)
{
  Serial.print(F("Connected to: "));
  Serial.println(central.address());
}

void bleDisconnectedHandler(BLEDevice central)
{
  Serial.print(F("Disconnected to: "));
  Serial.println(central.address());
}

void charUnsubscribedHandler(BLEDevice central, BLECharacteristic characteristic)
{
  Serial.println(F("Unsubscribed "));
  isConnected = false;
  digitalWrite(LED_BUILTIN, LOW);
}

void charSubscribedHandler(BLEDevice central, BLECharacteristic characteristic)
{
  isConnected = true;
  Serial.println(F("Unsubscribed "));
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  if (!BLE.begin())
  {
    Serial.println("Starting BLE failed!");
    while (1)
      ;
  }
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
  BLE.setDeviceName("eMOB_BLE");
  BLE.setLocalName("eMOB_BLE");
  BLE.setAdvertisedService(eMOBService);

  eMOBService.addCharacteristic(accelerometerChar);
  // eMOBService.addCharacteristic(gyroscopeChar);
  // eMOBService.addCharacteristic(magnetometerChar);

  BLE.addService(eMOBService);
  BLE.setEventHandler(BLEConnected, bleConnectedHandler);
  BLE.setEventHandler(BLEDisconnected, bleDisconnectedHandler);

  accelerometerChar.setEventHandler(BLESubscribed, charSubscribedHandler);
  accelerometerChar.setEventHandler(BLEUnsubscribed, charUnsubscribedHandler);

  BLE.advertise();
}

void loop()
{
  BLE.poll(); //TODO check here time optimization
  if (isConnected)
  {
    if (IMU.accelerationAvailable())
    {
      IMU.readAcceleration(Ax, Ay, Az);
      //unsigned long actual_t = millis();
      String accelerometerData = String(Ax) + " " + String(Ay) + " " + String(Az) + " " + String(actual_t - last_t);
      actual_t = micros();
      accelerometerChar.writeValue(accelerometerData);
      last_t = actual_t;
    }
  }
}