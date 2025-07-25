/*
  Using the BNO080 IMU
  By: Nathan Seidle
  SparkFun Electronics
  Date: December 21st, 2017
  SparkFun code, firmware, and software is released under the MIT License.
	Please see LICENSE.md for further details.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14586

  This example shows how to print the raw packets. This is handy for debugging.

  It takes about 1ms at 400kHz I2C to read a record from the sensor, but we are polling the sensor continually
  between updates from the sensor. Use the interrupt pin on the BNO080 breakout to avoid polling.

  Hardware Connections:
  Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the sensor onto the shield
  Serial.print it out at 115200 baud to serial monitor.
*/

#include <Wire.h>

//#define Print_rxPin PC1
//#define Print_txPin PC0
#define Print_rxPin PB6
#define Print_txPin PB7
#include <SoftwareSerial.h>

SoftwareSerial Print_tx_rx =  SoftwareSerial(Print_rxPin, Print_txPin);

#include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
BNO080 myIMU;

void setup()
{
  Print_tx_rx.begin(9600);
  Print_tx_rx.println();
  Print_tx_rx.println("BNO080 Read Example");

  Wire.begin();

  myIMU.begin();

  Wire.setClock(9600); //Increase I2C data rate to 400kHz

  myIMU.enableDebugging(Print_tx_rx); //Output debug messages to the Serial port. Serial1, SerialUSB, etc is also allowed.

  myIMU.enableMagnetometer(1000);
  myIMU.enableAccelerometer(1000);
}

void loop()
{
  //Look for reports from the IMU
  if (myIMU.receivePacket() == true)
  {
    myIMU.printPacket();
  }
}
