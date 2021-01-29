#include <Arduino.h>
#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>

BLEService environmentService("181A");
BLEStringCharacteristic IMU_characteristic("e95dca4b-251d-470a-a062-fa1922dfa9a8", BLERead | BLENotify, 90);
BLEDescriptor IMULabelDescriptor("2901", "3 Axis IMU data ints: AccX|AccY|AccZ GyrX|GyrY|GyrZ MagX|MagY|MagZ");

bool isSubscribed = false;

float Ax = 0.0, Ay = 0.0, Az = 0.0;
float Gx = 0.0, Gy = 0.0, Gz = 0.0;
float Mx = 0.0, My = 0.0, Mz = 0.0;

char buffer[70];

void charUnsubscribedHandler(BLEDevice central, BLECharacteristic characteristic);
void charSubscribedHandler(BLEDevice central, BLECharacteristic characteristic);
void connectedHandler(BLEDevice central);
void disconnectedHandler(BLEDevice central);

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  while (!BLE.begin())
    ;
  while (!IMU.begin())
    ;

  BLE.setLocalName("eMOB_BLE");
  BLE.setConnectionInterval(0x0006, 0x0006); // minimum and maximum connection interval set at 7.5ms
  BLE.setAdvertisedService(environmentService);

  environmentService.addCharacteristic(IMU_characteristic);
  IMU_characteristic.addDescriptor(IMULabelDescriptor);

  BLE.addService(environmentService);
  BLE.setEventHandler(BLEConnected, connectedHandler);
  BLE.setEventHandler(BLEDisconnected, disconnectedHandler);

  IMU_characteristic.setEventHandler(BLESubscribed, charSubscribedHandler);
  IMU_characteristic.setEventHandler(BLEUnsubscribed, charUnsubscribedHandler);
  IMU_characteristic.writeValue("0");
  
  BLE.advertise();

  while (!IMU.magneticFieldAvailable())
    ; // wait

  //initialize magnetic field data
  IMU.readMagneticField(Mx, My, Mz);
  IMU.setContinuousMode();
}

void loop()
{
  BLE.poll();
  if (isSubscribed)
  {
    if (IMU.magneticFieldAvailable())
    {
      IMU.readMagneticField(Mx, My, Mz);
    }
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable())
    {
      IMU.readAcceleration(Ax, Ay, Az);
      IMU.readGyroscope(Gx, Gy, Gz);
    }

    sprintf(buffer, "%lu %.0f|%.0f|%.0f %.0f|%.0f|%.0f %.0f|%.0f|%.0f ", millis(), (Ax * 1000), (Ay * 1000), (Az * 1000), (Gx * 10), (Gy * 10), (Gz * 10), (Mx * 10), (My * 10), (Mz * 10)); // transform char* to string
    IMU_characteristic.writeValue(buffer);
    delay(5); //TODO
  }
}

void charUnsubscribedHandler(BLEDevice central, BLECharacteristic characteristic)
{
  Serial.println(F("Unsubscribed "));
  isSubscribed = false;
  digitalWrite(LED_BUILTIN, LOW);
}

void charSubscribedHandler(BLEDevice central, BLECharacteristic characteristic)
{
  isSubscribed = true;
  Serial.println(F("Subscribed "));
  digitalWrite(LED_BUILTIN, HIGH);
}

void connectedHandler(BLEDevice central){
  Serial.print(F("Connected to "));
  Serial.println(central.address());
}

void disconnectedHandler(BLEDevice central){
  Serial.println(F("Disconnected to "));
  Serial.println(central.address());
  BLE.advertise();
}