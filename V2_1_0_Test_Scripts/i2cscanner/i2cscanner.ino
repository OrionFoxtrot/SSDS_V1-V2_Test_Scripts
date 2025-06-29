// SPDX-FileCopyrightText: 2023 Carter Nelson for Adafruit Industries
//
// SPDX-License-Identifier: MIT
// --------------------------------------
// i2c_scanner
//
// Modified from https://playground.arduino.cc/Main/I2cScanner/
// --------------------------------------

#include <Wire.h>
#include <SoftwareSerial.h>

//#define Print_rxPin PC1
//#define Print_txPin PC0

#define Print_rxPin PB6
#define Print_txPin PB7

SoftwareSerial Print_tx_rx =  SoftwareSerial(Print_rxPin, Print_txPin);


// Set I2C bus to use: Wire, Wire1, etc.
#define WIRE Wire

void setup() {
  WIRE.begin();

  Print_tx_rx.begin(9600);
  while (!Print_tx_rx)
     delay(10);
  Print_tx_rx.println("\nI2C Scanner");
}


void loop() {
  byte error, address;
  int nDevices;

  Print_tx_rx.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    if (error == 0)
    {
      Print_tx_rx.print("I2C device found at address 0x");
      if (address<16)
        Print_tx_rx.print("0");
      Print_tx_rx.print(address,HEX);
      Print_tx_rx.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Print_tx_rx.print("Unknown error at address 0x");
      if (address<16)
        Print_tx_rx.print("0");
      Print_tx_rx.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Print_tx_rx.println("No I2C devices found\n");
  else
    Print_tx_rx.println("done\n");

  delay(5000);           // wait 5 seconds for next scan
}
