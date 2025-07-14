#include <SPI.h>

// Correct SPI pin mapping for your Wio-E5 custom board
#define FLASH_CS_PIN PB9
#define MOSI_PIN     PA10
#define MISO_PIN     PB14
#define SCK_PIN      PB13

#include <SoftwareSerial.h>
#define rxPin PC1
#define txPin PC0

SoftwareSerial soft_tx_rx =  SoftwareSerial(rxPin, txPin);


void flashSelect()   { digitalWrite(FLASH_CS_PIN, LOW); }
void flashDeselect() { digitalWrite(FLASH_CS_PIN, HIGH); }

void writeEnable() {
  flashSelect();
  SPI.transfer(0x06); // Write Enable
  flashDeselect();

  delayMicroseconds(1);
}

uint8_t readStatusRegister() {
  flashSelect();
  SPI.transfer(0x05); // RDSR
  uint8_t status = SPI.transfer(0);
  flashDeselect();
  return status;
}

void printStatusRegister() {
  uint8_t status = readStatusRegister();
  soft_tx_rx.print("Status Register: 0b");
  soft_tx_rx.println(status, BIN);
}

void waitForWriteEnd() {
  flashSelect();
  SPI.transfer(0x05); // RDSR
  unsigned long start = millis();
  uint8_t status;
  do {
    status = SPI.transfer(0x00);
    if (millis() - start > 2000) {
      soft_tx_rx.println("Timeout waiting for write to finish!");
      break;
    }
  } while (status & 0x01);
  flashDeselect();
}

void readJEDEC_ID(uint8_t *id) {
  flashSelect();
  SPI.transfer(0x9F);
  id[0] = SPI.transfer(0);
  id[1] = SPI.transfer(0);
  id[2] = SPI.transfer(0);
  flashDeselect();
}

void sectorErase(uint32_t addr) {
  writeEnable();
  flashSelect();
  SPI.transfer(0x20);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  flashDeselect();
  waitForWriteEnd();
}

void pageProgram(uint32_t addr, uint8_t *data, uint16_t len) {
  writeEnable();
  flashSelect();
  SPI.transfer(0x02);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  for (int i = 0; i < len; i++) {
    SPI.transfer(data[i]);
  }
  flashDeselect();
  waitForWriteEnd();
}

void readData(uint32_t addr, uint8_t *buf, uint16_t len) {
  flashSelect();
  SPI.transfer(0x03);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  for (int i = 0; i < len; i++) {
    buf[i] = SPI.transfer(0);
  }
  flashDeselect();
}

void flashTest() {
  uint8_t id[3] = {0};
  readJEDEC_ID(id);
  soft_tx_rx.print("JEDEC ID: ");
  soft_tx_rx.print(id[0], HEX); soft_tx_rx.print(" ");
  soft_tx_rx.print(id[1], HEX); soft_tx_rx.print(" ");
  soft_tx_rx.println(id[2], HEX);

  if (id[0] == 0xFF && id[1] == 0xFF && id[2] == 0xFF) {
    soft_tx_rx.println("Flash not responding (JEDEC = 0xFF). Skipping write/read.");
    soft_tx_rx.println("----------------------------");
    return;
  }

  const uint32_t addr = 0x000000;
  uint8_t writeBuf[256];
  uint8_t readBuf[256];
  for (int i = 0; i < 256; i++) writeBuf[i] = i;

  soft_tx_rx.println("Erasing sector...");
  sectorErase(addr);

  printStatusRegister();

  soft_tx_rx.println("Writing page...");
  pageProgram(addr, writeBuf, 256);

  soft_tx_rx.println("Reading back...");
  readData(addr, readBuf, 256);

  soft_tx_rx.println("Read Data:");
  for (int i = 0; i < 256; i++) {
    soft_tx_rx.print(readBuf[i], HEX);
    soft_tx_rx.print(" ");
    if (i % 16 == 15) soft_tx_rx.println();
  }

  bool match = true;
  for (int i = 0; i < 256; i++) {
    if (readBuf[i] != writeBuf[i]) {
      match = false;
      soft_tx_rx.print("Mismatch at ");
      soft_tx_rx.print(i); soft_tx_rx.print(": ");
      soft_tx_rx.print(readBuf[i]); soft_tx_rx.print(" != ");
      soft_tx_rx.println(writeBuf[i]);
    }
  }

  if (match) {
    soft_tx_rx.println("FLASH TEST: SUCCESS");
  } else {
    soft_tx_rx.println("FLASH TEST: FAIL");
  }

  soft_tx_rx.println("----------------------------");
}

void setup() {
  soft_tx_rx.begin(9600);
  soft_tx_rx.println("I HAVE RISEN");
  delay(1000);

  pinMode(FLASH_CS_PIN, OUTPUT);
  flashDeselect();

  SPI.setMOSI(MOSI_PIN);
  SPI.setMISO(MISO_PIN);
  SPI.setSCLK(SCK_PIN);
  SPI.begin();
}

void loop() {
  flashTest();
  delay(3000); // Wait 3 seconds and repeat
}