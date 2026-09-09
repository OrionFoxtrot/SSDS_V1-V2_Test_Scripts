#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <U8x8lib.h>


// ============================================================
// RFM69
// ============================================================

#define RFM_CS 10
#define RFM_DIO0 2
#define RFM_RST 4

#define TRANSLATOR_EN 9

RF69 radio = new Module(
  RFM_CS,
  RFM_DIO0,
  RFM_RST);


// ============================================================
// SWITCH INPUTS
// ============================================================

// External pull-downs on PCB:
//
// OFF -> LOW
// ON  -> HIGH

#define FILL_PIN 7
#define IGNITION_PIN 8

#define SWITCH_ACTIVE_STATE HIGH


// ============================================================
// CONTROLLER / LINK CONFIGURATION
// ============================================================

const uint8_t CONTROLLER_ID = 1;

uint16_t sequenceNumber = 0;


// wait for ACK after each command
const unsigned long ACK_TIMEOUT_MS = 200;

// consider link dead if no valid ACK for this long
const unsigned long LINK_TIMEOUT_MS = 1000;

// delay after each TX/ACK cycle
const unsigned long TX_INTERVAL_MS = 100;


// ============================================================
// LINK STATE
// ============================================================

bool haveReceivedAck = false;

unsigned long lastAckTime = 0;

float lastAckRSSI = 0;


// ============================================================
// PACKETS
// ============================================================

String txPacket;
String ackPacket;
String expectedAck;


// ============================================================
// OLED
// ============================================================

// 128x32 SSD1306
// I2C:
// SDA = A4
// SCL = A5

U8X8_SSD1306_128X32_UNIVISION_HW_I2C display(
  U8X8_PIN_NONE);


// ============================================================
// RF69 MANUAL RESET
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
// LINK STATUS
// ============================================================

bool isLinked() {

  if (!haveReceivedAck) {
    return false;
  }

  if ((millis() - lastAckTime) > LINK_TIMEOUT_MS) {
    return false;
  }

  return true;
}


// ============================================================
// OLED
// ============================================================

void updateDisplay(bool Fill, bool Ignition) {

  display.clear();


  // ----------------------------------------------------------
  // Line 1
  // ----------------------------------------------------------

  display.setCursor(0, 0);

  display.print("ID:");
  display.print(CONTROLLER_ID);


  // ----------------------------------------------------------
  // Line 2
  // ----------------------------------------------------------

  display.setCursor(0, 1);

  display.print("Fill: ");

  if (Fill) {
    display.print("ON");
  } else {
    display.print("OFF");
  }


  // ----------------------------------------------------------
  // Line 3
  // ----------------------------------------------------------

  display.setCursor(0, 2);

  display.print("Ign:  ");

  if (Ignition) {
    display.print("ON");
  } else {
    display.print("OFF");
  }


  // ----------------------------------------------------------
  // Line 4
  // ----------------------------------------------------------

  display.setCursor(0, 3);

  if (isLinked()) {

    display.print("LINK ");

    display.print((int)lastAckRSSI);

    display.print("dBm");

  } else {

    display.print("NO LINK");
  }
}


// ============================================================
// BUILD TX PACKET
// ============================================================

void buildPacket(bool Fill, bool Ignition) {

  txPacket = "";

  txPacket += F("ID=");
  txPacket += CONTROLLER_ID;

  txPacket += F(",SEQ=");
  txPacket += sequenceNumber;

  txPacket += F(",Fill=");
  txPacket += (Fill ? 1 : 0);

  txPacket += F(",Ignition=");
  txPacket += (Ignition ? 1 : 0);
}


// ============================================================
// BUILD EXPECTED ACK
// ============================================================

void buildExpectedAck() {

  expectedAck = "";

  expectedAck += F("ACK,ID=");
  expectedAck += CONTROLLER_ID;

  expectedAck += F(",SEQ=");
  expectedAck += sequenceNumber;
}


// ============================================================
// SETUP
// ============================================================
#include <avr/io.h>

uint8_t resetFlags __attribute__((section(".noinit")));

void captureResetFlags(void)
  __attribute__((naked))
  __attribute__((section(".init3")));

void captureResetFlags(void) {
  resetFlags = MCUSR;
  MCUSR = 0;
}
void setup() {

  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println(F("===== RESET ====="));

  Serial.print(F("MCUSR = 0x"));
  Serial.println(resetFlags, HEX);

  if (resetFlags & _BV(PORF)) {
    Serial.println(F("Reset cause: POWER ON"));
  }

  if (resetFlags & _BV(EXTRF)) {
    Serial.println(F("Reset cause: EXTERNAL RESET"));
  }

  if (resetFlags & _BV(BORF)) {
    Serial.println(F("Reset cause: BROWN OUT"));
  }

  if (resetFlags & _BV(WDRF)) {
    Serial.println(F("Reset cause: WATCHDOG"));
  }

  delay(1000);

  Serial.println();
  Serial.println(F("===== CONTROLLER START ====="));


  // ----------------------------------------------------------
  // Switches
  // ----------------------------------------------------------

  pinMode(FILL_PIN, INPUT);
  pinMode(IGNITION_PIN, INPUT);


  // ----------------------------------------------------------
  // Reserve String memory once
  // ----------------------------------------------------------

  txPacket.reserve(50);
  ackPacket.reserve(30);
  expectedAck.reserve(30);


  // ----------------------------------------------------------
  // RF hardware startup
  // ----------------------------------------------------------

  pinMode(TRANSLATOR_EN, OUTPUT);

  digitalWrite(
    TRANSLATOR_EN,
    LOW);


  pinMode(RFM_CS, OUTPUT);

  digitalWrite(
    RFM_CS,
    HIGH);


  pinMode(RFM_RST, OUTPUT);

  digitalWrite(
    RFM_RST,
    LOW);


  delay(500);


  // Enable level translator
  digitalWrite(
    TRANSLATOR_EN,
    HIGH);


  delay(500);


  manualRFM69Reset();


  // ==========================================================
  // RADIOLIB 7.7.x INITIALIZATION
  // ==========================================================

  Serial.print(F("[RF69] Initializing ... "));


  ConfigFSK_t config;

  config.frequency = 915.0;


  int state = radio.begin(config);


  if (state == RADIOLIB_ERR_NONE) {

    Serial.println(F("success!"));

  } else {

    Serial.print(F("failed, code "));
    Serial.println(state);

    while (true) {
      delay(1000);
    }
  }


  // ----------------------------------------------------------
  // RFM69HCW high-power PA
  // ----------------------------------------------------------

  Serial.print(
    F("[RF69] Setting HCW output power ... "));


  state = radio.setOutputPower(
    20,    // +2 dBm
    true  // RF69H/HC/HCW
  );


  if (state == RADIOLIB_ERR_NONE) {

    Serial.println(F("success!"));

  } else {

    Serial.print(F("failed, code "));
    Serial.println(state);

    while (true) {
      delay(1000);
    }
  }


  // ==========================================================
  // OLED
  // ==========================================================

  display.begin();

  display.setFont(
    u8x8_font_chroma48medium8_r);

  display.clear();


  display.setCursor(0, 0);
  display.print("Controller");

  display.setCursor(0, 1);
  display.print("Starting...");


  Serial.println(F("[SYSTEM] Ready"));


  // ----------------------------------------------------------
  // Initial display state
  // ----------------------------------------------------------

  bool Fill =
    (digitalRead(FILL_PIN) == SWITCH_ACTIVE_STATE);


  bool Ignition =
    (digitalRead(IGNITION_PIN) == SWITCH_ACTIVE_STATE);


  updateDisplay(
    Fill,
    Ignition);


  delay(500);
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // Read switch states
  // ----------------------------------------------------------

  bool Fill =
    (digitalRead(FILL_PIN) == SWITCH_ACTIVE_STATE);


  bool Ignition =
    (digitalRead(IGNITION_PIN) == SWITCH_ACTIVE_STATE);


  // ----------------------------------------------------------
  // Build command
  // ----------------------------------------------------------

  buildPacket(
    Fill,
    Ignition);


  buildExpectedAck();


  Serial.print(F("[TX] "));
  Serial.println(txPacket);


  // ==========================================================
  // TRANSMIT COMMAND
  // ==========================================================

  int state =
    radio.transmit(txPacket);


  if (state != RADIOLIB_ERR_NONE) {

    Serial.print(F("[TX] Failed: "));
    Serial.println(state);


    updateDisplay(
      Fill,
      Ignition);


    sequenceNumber++;

    delay(TX_INTERVAL_MS);

    return;
  }


  // ==========================================================
  // WAIT FOR ACK
  // ==========================================================

  ackPacket = "";


  state = radio.receive(
    ackPacket,
    0,
    ACK_TIMEOUT_MS);


  // ==========================================================
  // ACK RECEIVED
  // ==========================================================

  if (state == RADIOLIB_ERR_NONE) {

    Serial.print(F("[RX ACK] "));
    Serial.println(ackPacket);


    // --------------------------------------------------------
    // Validate ID + sequence
    // --------------------------------------------------------

    if (ackPacket == expectedAck) {

      haveReceivedAck = true;

      lastAckTime = millis();

      lastAckRSSI =
        radio.getRSSI();


      Serial.print(
        F("[LINK] Valid ACK, RSSI = "));

      Serial.print(lastAckRSSI);

      Serial.println(F(" dBm"));

    } else {

      Serial.print(
        F("[LINK] Unexpected ACK. Expected: "));

      Serial.println(expectedAck);
    }
  }


  // ==========================================================
  // ACK TIMEOUT
  // ==========================================================

  else if (
    state == RADIOLIB_ERR_RX_TIMEOUT) {

    Serial.println(
      F("[LINK] ACK timeout"));
  }


  // ==========================================================
  // CRC ERROR
  // ==========================================================

  else if (
    state == RADIOLIB_ERR_CRC_MISMATCH) {

    Serial.println(
      F("[LINK] ACK CRC error"));
  }


  // ==========================================================
  // OTHER RX ERROR
  // ==========================================================

  else {

    Serial.print(F("[LINK] RX error: "));
    Serial.println(state);
  }


  // ----------------------------------------------------------
  // Refresh OLED
  // ----------------------------------------------------------

  updateDisplay(
    Fill,
    Ignition);


  // ----------------------------------------------------------
  // Next packet
  // ----------------------------------------------------------

  sequenceNumber++;


  delay(TX_INTERVAL_MS);
}