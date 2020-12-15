#include <Arduino.h>
#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>

BLEService eMOBService("7730e37f-21a3-407e-96e0-d39c83bb989c");
BLEStringCharacteristic IMU_characteristic("72ce899b-0049-4e10-9ae6-f440822db322", BLERead | BLENotify, 60);

bool isSubscribed = false;

float Ax = 0.0, Ay = 0.0, Az = 0.0;
float Gx = 0, Gy = 0, Gz = 0;
float Mx = 0, My = 0, Mz = 0;
unsigned long start=0, end=0, count = 0;
char buffer[58];

void charUnsubscribedHandler(BLEDevice central, BLECharacteristic characteristic);
void charSubscribedHandler(BLEDevice central, BLECharacteristic characteristic);

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

  eMOBService.addCharacteristic(IMU_characteristic);
  BLE.addService(eMOBService);

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

    sprintf(buffer, "%.0f|%.0f|%.0f %.0f|%.0f|%.0f %.0f|%.0f|%.0f ", (Ax*1000), (Ay*1000), (Az*1000), (Gx*10), (Gy*10), (Gz*10), (Mx*10), (My*10), (Mz*10)); // transform char* to string
    IMU_characteristic.writeValue(buffer);
    count++;
  }
  else if(!isSubscribed && end>start && count !=0){
    Serial.print("Freq: ");
    Serial.println((float)(count * 1000.0) / (end - start));
    count = 0;
    Serial.printf("%s  \n ", buffer);
    //BLE.disconnect();
    // delay(10000);
  }
}

void charUnsubscribedHandler(BLEDevice central, BLECharacteristic characteristic)
{
  Serial.println(F("Unsubscribed "));
  isSubscribed = false;
  digitalWrite(LED_BUILTIN, LOW);
  end = millis();
}

void charSubscribedHandler(BLEDevice central, BLECharacteristic characteristic)
{
  isSubscribed = true;
  Serial.println(F("Subscribed "));
  digitalWrite(LED_BUILTIN, HIGH);
  start = millis();
}