/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
#include <SoftwareSerial.h>

//#define rxPin PC1
//#define txPin PC0

#define rxPin PB6
#define txPin PB7

SoftwareSerial soft_tx_rx =  SoftwareSerial(rxPin, txPin);


// the setup function runs once when you press reset or power the board
void setup() {
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);

    soft_tx_rx.begin(9600);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PA9, OUTPUT);
  
}
int counter = 0;
// the loop function runs over and over again forever
void loop() {
  counter ++;
  soft_tx_rx.print("hello world counter k:");
  soft_tx_rx.println(counter);

  digitalWrite(PA9, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(2000);                      // wait for a second
  digitalWrite(PA9, LOW);   // turn the LED off by making the voltage LOW
  delay(2000);                      // wait for a second
}
