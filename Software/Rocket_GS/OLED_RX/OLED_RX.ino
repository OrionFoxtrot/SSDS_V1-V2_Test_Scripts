#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>

#include <U8x8lib.h>


// ============================================================
// RFM69
// ============================================================

#define RFM_CS    10
#define RFM_DIO0  2
#define RFM_RST   4
#define blinkypin 9

RF69 radio = new Module(RFM_CS, RFM_DIO0, RFM_RST);


// ============================================================
// OLED - 128x32 SSD1306 I2C
// ============================================================

// No framebuffer required
U8X8_SSD1306_128X32_UNIVISION_HW_I2C display(U8X8_PIN_NONE);


// ============================================================
// RFM69 RESET
// ============================================================

void manualRFM69Reset() {
  pinMode(RFM_RST, OUTPUT);

  digitalWrite(RFM_RST, LOW);
  delay(10);

  digitalWrite(RFM_RST, HIGH);
  delay(10);

  digitalWrite(RFM_RST, LOW);
  delay(100);
}


// ============================================================
// DISPLAY
// ============================================================

void displayWaiting() {
  display.clear();

  display.setCursor(0, 0);
  display.print("RFM69 RX");

  display.setCursor(0, 1);
  display.print("915 MHz");

  display.setCursor(0, 2);
  display.print("Waiting...");
}


void displayPacket(String message, float rssi) {

  display.clear();

  display.setCursor(0, 0);
  display.print("RX:");

  display.setCursor(0, 1);

  // 128-pixel-wide U8x8 display is approximately
  // 16 characters per line
  display.print(message.substring(0, 16));

  display.setCursor(0, 2);
  display.print("RSSI:");

  display.setCursor(0, 3);
  display.print(rssi);
  display.print(" dBm");
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(500);


  // ----------------------------------------------------------
  // OLED
  // ----------------------------------------------------------

  display.begin();
  display.setFont(u8x8_font_chroma48medium8_r);

  displayWaiting();


  // ----------------------------------------------------------
  // RFM69
  // ----------------------------------------------------------

  pinMode(blinkypin, OUTPUT);
  digitalWrite(blinkypin, LOW);

  pinMode(RFM_CS, OUTPUT);
  digitalWrite(RFM_CS, HIGH);

  pinMode(RFM_RST, OUTPUT);
  digitalWrite(RFM_RST, LOW);

  delay(500);

  digitalWrite(blinkypin, HIGH);
  delay(500);

  manualRFM69Reset();


  Serial.println(F("[RF69] Initializing ..."));

  int state = radio.begin(915.0);

  while (state != RADIOLIB_ERR_NONE) {

    Serial.print(F("failed, code "));
    Serial.println(state);

    display.clear();
    display.setCursor(0, 0);
    display.print("RF69 INIT FAIL");

    manualRFM69Reset();

    digitalWrite(blinkypin, LOW);
    delay(100);
    digitalWrite(blinkypin, HIGH);

    delay(100);

    state = radio.begin(915.0);
  }


  Serial.println(F("[RF69] Success!"));

  displayWaiting();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  String receivedString;

  Serial.println(F("[RF69] Waiting..."));

  int state = radio.receive(receivedString);


  if (state == RADIOLIB_ERR_NONE) {

    float rssi = radio.getRSSI();

    Serial.print(F("Data: "));
    Serial.println(receivedString);

    Serial.print(F("RSSI: "));
    Serial.print(rssi);
    Serial.println(F(" dBm"));

    displayPacket(receivedString, rssi);
  }


  else if (state == RADIOLIB_ERR_RX_TIMEOUT) {

    Serial.println(F("RX timeout"));

  }


  else if (state == RADIOLIB_ERR_CRC_MISMATCH) {

    Serial.println(F("CRC error"));

    display.clear();
    display.setCursor(0, 0);
    display.print("CRC ERROR");
  }


  else {

    Serial.print(F("RX error: "));
    Serial.println(state);

    display.clear();
    display.setCursor(0, 0);
    display.print("RX ERROR");
  }
}