
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#define Print_rxPin PB6
#define Print_txPin PB7

#define GPS_rxPin PC1
#define GPS_txPin PC0

TinyGPSPlus gps;

SoftwareSerial Print_tx_rx =  SoftwareSerial(Print_rxPin, Print_txPin);
SoftwareSerial GPS_tx_rx =  SoftwareSerial(GPS_rxPin, GPS_txPin);


// the setup function runs once when you press reset or power the board
void setup() {
    pinMode(Print_rxPin, INPUT);
    pinMode(Print_txPin, OUTPUT);

    Print_tx_rx.begin(9600);
    GPS_tx_rx.begin(9600);


  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PA9, OUTPUT);
  Print_tx_rx.println("I have Risen");
  digitalWrite(PA9, HIGH);
  
}

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (GPS_tx_rx.available() > 0)
    if (gps.encode(GPS_tx_rx.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}



void displayInfo()
{
  Print_tx_rx.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Print_tx_rx.print(gps.location.lat(), 6);
    Print_tx_rx.print(F(","));
    Print_tx_rx.print(gps.location.lng(), 6);
  }
  else
  {
    Print_tx_rx.print(F("INVALID"));
  }

  Print_tx_rx.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Print_tx_rx.print(gps.date.month());
    Print_tx_rx.print(F("/"));
    Print_tx_rx.print(gps.date.day());
    Print_tx_rx.print(F("/"));
    Print_tx_rx.print(gps.date.year());
  }
  else
  {
    Print_tx_rx.print(F("INVALID"));
  }

  Print_tx_rx.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Print_tx_rx.print(F("0"));
    Print_tx_rx.print(gps.time.hour());
    Print_tx_rx.print(F(":"));
    if (gps.time.minute() < 10) Print_tx_rx.print(F("0"));
    Print_tx_rx.print(gps.time.minute());
    Print_tx_rx.print(F(":"));
    if (gps.time.second() < 10) Print_tx_rx.print(F("0"));
    Print_tx_rx.print(gps.time.second());
    Print_tx_rx.print(F("."));
    if (gps.time.centisecond() < 10) Print_tx_rx.print(F("0"));
    Print_tx_rx.print(gps.time.centisecond());
  }
  else
  {
    Print_tx_rx.print(F("INVALID"));
  }

  Print_tx_rx.println();
}
