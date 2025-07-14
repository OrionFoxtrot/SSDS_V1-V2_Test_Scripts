#include <Adafruit_INA260.h>

Adafruit_INA260 INA1 = Adafruit_INA260(); // Left
Adafruit_INA260 INA2 = Adafruit_INA260(); // Middle
Adafruit_INA260 INA3 = Adafruit_INA260(); // Right

void setup() {
  Serial.begin(115200);
  // Wait until serial port is opened
  while (!Serial) { delay(10); }

  Serial.println("Adafruit INA260 Test");

  if (!INA1.begin(0x40)) {
    Serial.println("Couldn't find INA1 chip");
    while (1);
  }
  if (!INA2.begin(0x41)) {
    Serial.println("Couldn't find INA2 chip");
    while (1);
  }
  if (!INA3.begin(0x44)) {
    Serial.println("Couldn't find INA3 chip");
    while (1);
  }
  Serial.println("Found All INA chips");
}

void loop() {
  Serial.print(String(INA1.readBusVoltage())+','+String(INA1.readCurrent()));
  Serial.print(','+String(INA2.readBusVoltage())+','+String(INA2.readCurrent()));
  Serial.println(','+String(INA3.readBusVoltage())+','+String(INA3.readCurrent()));
  
  
  delay(250);
}
