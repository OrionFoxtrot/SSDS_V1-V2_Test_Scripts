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
// OLED - SSD1306 128x32 I2C
// ============================================================

U8X8_SSD1306_128X32_UNIVISION_HW_I2C display(U8X8_PIN_NONE);


// ============================================================
// RFM69 RESET
// ============================================================

void manualRFM69Reset() {

  pinMode(RFM_RST, OUTPUT);

  digitalWrite(RFM_RST, LOW);
  delay(10);

  digitalWrite(RFM_RST, HIGH);   // active-high reset
  delay(10);

  digitalWrite(RFM_RST, LOW);    // release reset
  delay(100);
}


// ============================================================
// DISPLAY FUNCTIONS
// ============================================================

void displayReady() {

  display.clear();

  display.setCursor(0, 0);
  display.print("RFM69 TX");

  display.setCursor(0, 1);
  display.print("915 MHz");

  display.setCursor(0, 2);
  display.print("Ready");
}


void displayTransmitting(String message) {

  display.clear();

  display.setCursor(0, 0);
  display.print("TX:");

  display.setCursor(0, 1);

  // U8x8 gives about 16 chars per line
  display.print(message.substring(0, 16));

  display.setCursor(0, 3);
  display.print("Sending...");
}


void displaySuccess(String message) {

  display.clear();

  display.setCursor(0, 0);
  display.print("TX:");

  display.setCursor(0, 1);
  display.print(message.substring(0, 16));

  display.setCursor(0, 3);
  display.print("SUCCESS");
}


void displayError(int state) {

  display.clear();

  display.setCursor(0, 0);
  display.print("TX ERROR");

  display.setCursor(0, 1);
  display.print("Code:");

  display.setCursor(6, 1);
  display.print(state);
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

  display.clear();

  display.setCursor(0, 0);
  display.print("Starting...");


  // ----------------------------------------------------------
  // RFM69 pins / translator
  // ----------------------------------------------------------

  pinMode(blinkypin, OUTPUT);

  // Keep translator disabled initially
  digitalWrite(blinkypin, LOW);


  pinMode(RFM_CS, OUTPUT);
  digitalWrite(RFM_CS, HIGH);


  pinMode(RFM_RST, OUTPUT);
  digitalWrite(RFM_RST, LOW);


  delay(500);


  // Enable translator after pins are sane
  digitalWrite(blinkypin, HIGH);

  delay(500);


  // Reset RFM69
  manualRFM69Reset();


  // ----------------------------------------------------------
  // Initialize RFM69
  // ----------------------------------------------------------

  Serial.println(F("[RF69] Initializing ..."));

  int state = radio.begin(915.0);


  while (state != RADIOLIB_ERR_NONE) {

    Serial.print(F("failed, code "));
    Serial.println(state);

    Serial.println(F("Trying to initialize again"));
    Serial.println(
      F("Hint: if connected to CityLabs Programmer, press power reset")
    );


    display.clear();

    display.setCursor(0, 0);
    display.print("RF69 INIT FAIL");

    display.setCursor(0, 1);
    display.print("Code:");

    display.setCursor(6, 1);
    display.print(state);


    // Try resetting radio again
    manualRFM69Reset();

    digitalWrite(blinkypin, LOW);
    delay(100);

    digitalWrite(blinkypin, HIGH);
    delay(100);


    state = radio.begin(915.0);
  }


  Serial.println(F("[RF69] Initialization successful!"));


  // ----------------------------------------------------------
  // Set high-power RFM69HCW mode
  // ----------------------------------------------------------

  Serial.println(F("[RF69] Setting output power ..."));

  state = radio.setOutputPower(5, true);


  if (state == RADIOLIB_ERR_NONE) {

    Serial.println(F("[RF69] Output power configured"));

  }

  else {

    Serial.print(F("[RF69] Output power error: "));
    Serial.println(state);

    displayError(state);

    while (true) {
      delay(10);
    }
  }


  displayReady();

  delay(1000);
}


// ============================================================
// Packet counter
// ============================================================

unsigned long count = 0;


// ============================================================
// LOOP
// ============================================================

void loop() {

  // Build packet
  String str = "Hello World! #" + String(count++);


  // ----------------------------------------------------------
  // Show packet before transmitting
  // ----------------------------------------------------------

  Serial.print(F("[RF69] Transmitting: "));
  Serial.println(str);

  displayTransmitting(str);


  // ----------------------------------------------------------
  // Transmit
  // ----------------------------------------------------------

  int state = radio.transmit(str);


  // ----------------------------------------------------------
  // Result
  // ----------------------------------------------------------

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println(F("[RF69] Transmission successful!"));

    displaySuccess(str);
  }


  else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {

    Serial.println(F("[RF69] Packet too long!"));

    display.clear();

    display.setCursor(0, 0);
    display.print("PACKET TOO LONG");
  }


  else {

    Serial.print(F("[RF69] Transmission failed, code "));
    Serial.println(state);

    displayError(state);
  }


  // Send once per second
  delay(1000);
}