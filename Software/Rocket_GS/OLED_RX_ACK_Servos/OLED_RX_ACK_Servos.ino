#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <U8x8lib.h>
#include <Servo.h>

#include <avr/io.h>
#include <avr/wdt.h>


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
// SERVOS
// ============================================================

#define FILL_SERVO_PIN 6
#define IGNITION_SERVO_PIN 5

Servo fillServo;
Servo ignitionServo;


// ============================================================
// SERVO POSITIONS
//
// REPLACE THESE WITH YOUR CALIBRATED VALUES
// ============================================================

const int FILL_OFF_ANGLE = 0;
const int FILL_ON_ANGLE = 180;

const int IGNITION_OFF_ANGLE = 0;
const int IGNITION_ON_ANGLE = 180;


// ============================================================
// CONTROLLER CONFIG
// ============================================================

const uint8_t CONTROLLER_ID = 1;

const unsigned long RX_TIMEOUT_MS = 200;
const unsigned long LINK_TIMEOUT_MS = 1000;


// ============================================================
// LINK STATE
// ============================================================

bool haveValidPacket = false;

unsigned long lastPacketTime = 0;

float lastCommandRSSI = 0;


// ============================================================
// COMMAND STATE
// ============================================================

bool Fill = false;
bool Ignition = false;

uint16_t lastSequence = 0;
bool haveSequence = false;


// ============================================================
// PACKETS
// ============================================================

String rxPacket;
String ackPacket;


// ============================================================
// OLED
// ============================================================

U8X8_SSD1306_128X32_UNIVISION_HW_I2C display(
  U8X8_PIN_NONE);


// ============================================================
// RESET FLAGS
// ============================================================

uint8_t resetFlags __attribute__((section(".noinit")));

void captureResetFlags(void)
  __attribute__((naked))
  __attribute__((section(".init3")));

void captureResetFlags(void) {

  resetFlags = MCUSR;

  MCUSR = 0;

  // Prevent a watchdog reset from causing a reset loop.
  wdt_disable();
}


// ============================================================
// MANUAL RF69 RESET
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

  if (!haveValidPacket) {
    return false;
  }

  if ((millis() - lastPacketTime) > LINK_TIMEOUT_MS) {
    return false;
  }

  return true;
}


// ============================================================
// OLED
// ============================================================

void updateDisplay() {

  display.clear();


  display.setCursor(0, 0);

  display.print("RX ID:");
  display.print(CONTROLLER_ID);


  display.setCursor(0, 1);

  display.print("Fill: ");

  if (Fill) {
    display.print("ON");
  } else {
    display.print("OFF");
  }


  display.setCursor(0, 2);

  display.print("Ign:  ");

  if (Ignition) {
    display.print("ON");
  } else {
    display.print("OFF");
  }


  display.setCursor(0, 3);

  if (isLinked()) {

    display.print("LINK ");
    display.print((int)lastCommandRSSI);
    display.print("dBm");

  } else {

    display.print("NO LINK");
  }
}


// ============================================================
// FIELD PARSER
//
// Expected:
//
// ID=1,SEQ=42,Fill=1,Ignition=0
//
// ============================================================

bool getField(
  String &packet,
  const char *fieldName,
  long &value) {

  int start = packet.indexOf(fieldName);

  if (start < 0) {
    return false;
  }


  start += strlen(fieldName);


  int end = packet.indexOf(',', start);

  if (end < 0) {
    end = packet.length();
  }


  String valueString =
    packet.substring(start, end);


  if (valueString.length() == 0) {
    return false;
  }


  value = valueString.toInt();

  return true;
}


// ============================================================
// ACK BUILDER
// ============================================================

void buildAck(uint16_t sequence) {

  ackPacket = "";

  ackPacket += F("ACK,ID=");
  ackPacket += CONTROLLER_ID;

  ackPacket += F(",SEQ=");
  ackPacket += sequence;
}


// ============================================================
// APPLY COMMAND
// ============================================================

void applyCommand(
  bool newFill,
  bool newIgnition) {

  // ----------------------------------------------------------
  // Only move a servo when that state actually changes.
  // ----------------------------------------------------------

  if (newFill != Fill) {

    Fill = newFill;

    if (Fill) {

      fillServo.write(FILL_ON_ANGLE);

      Serial.print(F("[SERVO] Fill -> ON, angle "));
      Serial.println(FILL_ON_ANGLE);

    } else {

      fillServo.write(FILL_OFF_ANGLE);

      Serial.print(F("[SERVO] Fill -> OFF, angle "));
      Serial.println(FILL_OFF_ANGLE);
    }
  }


  if (newIgnition != Ignition) {

    Ignition = newIgnition;

    if (Ignition) {

      ignitionServo.write(IGNITION_ON_ANGLE);

      Serial.print(F("[SERVO] Ignition -> ON, angle "));
      Serial.println(IGNITION_ON_ANGLE);

    } else {

      ignitionServo.write(IGNITION_OFF_ANGLE);

      Serial.print(F("[SERVO] Ignition -> OFF, angle "));
      Serial.println(IGNITION_OFF_ANGLE);
    }
  }


  Serial.print(F("[CMD] Fill="));
  Serial.print(Fill);

  Serial.print(F(" Ignition="));
  Serial.println(Ignition);
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);
  delay(500);


  // ==========================================================
  // RESET CAUSE
  // ==========================================================

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


  Serial.println();
  Serial.println(F("===== RF69 GROUND STATION ====="));


  // ==========================================================
  // STRING MEMORY
  // ==========================================================

  rxPacket.reserve(50);
  ackPacket.reserve(30);


  // ==========================================================
  // SERVOS
  // ==========================================================

  Serial.println(F("[SERVO] Initializing..."));


  fillServo.attach(FILL_SERVO_PIN);

  ignitionServo.attach(IGNITION_SERVO_PIN);


  // Explicit startup states.
  //
  // Change these only if these angles are physically verified
  // for your system.

  Fill = false;
  Ignition = false;

  fillServo.write(FILL_OFF_ANGLE);

  ignitionServo.write(IGNITION_OFF_ANGLE);


  delay(500);


  Serial.println(F("[SERVO] Ready"));


  // ==========================================================
  // RF HARDWARE STARTUP
  // ==========================================================

  pinMode(
    TRANSLATOR_EN,
    OUTPUT);

  digitalWrite(
    TRANSLATOR_EN,
    LOW);


  pinMode(
    RFM_CS,
    OUTPUT);

  digitalWrite(
    RFM_CS,
    HIGH);


  pinMode(
    RFM_RST,
    OUTPUT);

  digitalWrite(
    RFM_RST,
    LOW);


  delay(500);


  digitalWrite(
    TRANSLATOR_EN,
    HIGH);


  delay(500);


  manualRFM69Reset();


  // ==========================================================
  // RADIOLIB 7.7.x INIT
  // ==========================================================

  Serial.print(F("[RF69] Initializing ... "));


  ConfigFSK_t config;

  config.frequency = 915.0;


  int state =
    radio.begin(config);


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
  // RFM69HCW ACK POWER
  // ==========================================================

  Serial.print(
    F("[RF69] Setting HCW output power ... "));


  state = radio.setOutputPower(
    10,
    true);


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
  display.print("Ground Station");

  display.setCursor(0, 1);
  display.print("Starting...");


  Serial.println(F("[SYSTEM] Ready"));


  updateDisplay();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  rxPacket = "";


  // ==========================================================
  // WAIT FOR COMMAND
  // ==========================================================

  int state = radio.receive(
    rxPacket,
    0,
    RX_TIMEOUT_MS);


  // ==========================================================
  // PACKET RECEIVED
  // ==========================================================

  if (state == RADIOLIB_ERR_NONE) {

    Serial.print(F("[RX] "));
    Serial.println(rxPacket);


    lastCommandRSSI =
      radio.getRSSI();


    // --------------------------------------------------------
    // Parse packet
    // --------------------------------------------------------

    long idValue;
    long seqValue;
    long fillValue;
    long ignitionValue;


    bool valid =

      getField(
        rxPacket,
        "ID=",
        idValue)

      &&

      getField(
        rxPacket,
        "SEQ=",
        seqValue)

      &&

      getField(
        rxPacket,
        "Fill=",
        fillValue)

      &&

      getField(
        rxPacket,
        "Ignition=",
        ignitionValue);


    if (!valid) {

      Serial.println(
        F("[RX] Invalid packet format"));

      updateDisplay();

      return;
    }


    // --------------------------------------------------------
    // Controller ID
    // --------------------------------------------------------

    if (idValue != CONTROLLER_ID) {

      Serial.print(
        F("[RX] Wrong controller ID: "));

      Serial.println(idValue);

      updateDisplay();

      return;
    }


    // --------------------------------------------------------
    // Validate boolean values
    // --------------------------------------------------------

    if (
      !((fillValue == 0) || (fillValue == 1))
      || !((ignitionValue == 0) || (ignitionValue == 1))) {

      Serial.println(
        F("[RX] Invalid command values"));

      updateDisplay();

      return;
    }


    uint16_t sequence =
      (uint16_t)seqValue;


    // --------------------------------------------------------
    // Link state
    // --------------------------------------------------------

    haveValidPacket = true;

    lastPacketTime =
      millis();


    // --------------------------------------------------------
    // Sequence handling
    // --------------------------------------------------------

    bool newPacket =
      (!haveSequence) || (sequence != lastSequence);


    if (newPacket) {

      applyCommand(
        fillValue == 1,
        ignitionValue == 1);


      lastSequence =
        sequence;

      haveSequence =
        true;

    } else {

      Serial.println(
        F("[RX] Duplicate SEQ - command already processed"));
    }


    // ========================================================
    // SEND ACK
    // ========================================================

    buildAck(sequence);


    Serial.print(F("[TX ACK] "));
    Serial.print(ackPacket);
    Serial.print(F(" ... "));


    state =
      radio.transmit(ackPacket);


    if (state == RADIOLIB_ERR_NONE) {

      Serial.println(F("success!"));

    } else {

      Serial.print(
        F("failed, code "));

      Serial.println(state);
    }


    updateDisplay();
  }


  // ==========================================================
  // RX TIMEOUT
  // ==========================================================

  // ==========================================================
  // RX TIMEOUT
  // ==========================================================

  else if (state == RADIOLIB_ERR_RX_TIMEOUT) {

    // If no valid command has been received for longer than
    // LINK_TIMEOUT_MS, force both commanded outputs OFF.
    if (!isLinked()) {

      // Only issue the failsafe command once if something
      // is currently ON.
      if (Fill || Ignition) {

        Serial.println(
          F("[FAILSAFE] Link lost -> Fill OFF, Ignition OFF"));

        applyCommand(
          false,  // Fill OFF
          false   // Ignition OFF
        );
      }
    }

    updateDisplay();
  }
  // ==========================================================
  // CRC ERROR
  // ==========================================================

  else if (
    state == RADIOLIB_ERR_CRC_MISMATCH) {

    Serial.println(
      F("[RX] CRC mismatch"));

    updateDisplay();
  }


  // ==========================================================
  // OTHER ERROR
  // ==========================================================

  else {

    Serial.print(
      F("[RX] Error: "));

    Serial.println(state);

    updateDisplay();
  }
}