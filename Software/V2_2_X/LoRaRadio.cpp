#include "LoRaRadio.h"

LoRaRadio::LoRaRadio(const uint32_t *rfswitch_pins, const Module::RfSwitchMode_t *rfswitch_table)
    : _rfswitch_pins(rfswitch_pins), _rfswitch_table(rfswitch_table) {}

bool LoRaRadio::begin(float freq, float power)
{
  // initialize STM32WL with default settings, except frequency
  radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);
  int state = radio.begin(freq);
  radio.setOutputPower(power);

  if (state == RADIOLIB_ERR_NONE)
  {
    Print_tx_rx.println(F("success!"));
  }
  else
  {
    Print_tx_rx.print(F("failed, code "));
    Print_tx_rx.println(state);
    while (true)
    {
      delay(10);
    }
  }
  return true;
}

bool LoRaRadio::setTCX0(float voltage)
{
  // set appropriate TCXO voltage for Nucleo WL55JC1
  state = radio.setTCXO(voltage);
  if (state == RADIOLIB_ERR_NONE)
  {
    Print_tx_rx.println(F("success!"));
  }
  else
  {
    Print_tx_rx.print(F("failed, code "));
    Print_tx_rx.println(state);
    while (true)
    {
      delay(10);
    }
  }
}

int LoRaRadio::transmit(const String &payload)
{
  int state = radio.transmit(payload);
  return state;
}

void LoRaRadio::interpretState(int state, Stream &debug = Serial)
{
  switch (state)
  {
  case RADIOLIB_ERR_NONE:
    debug.println(F("[LoRaRadio] Packet transmitted successfully!"));
    debug.print(F("[LoRaRadio] Data rate: "));
    debug.print(radio.getDataRate());
    debug.println(F(" bps"));
    break;

  case RADIOLIB_ERR_PACKET_TOO_LONG:
    debug.println(F("[LoRaRadio] Packet too long!"));
    break;

  case RADIOLIB_ERR_TX_TIMEOUT:
    debug.println(F("[LoRaRadio] Transmission timeout!"));
    break;

  default:
    debug.print(F("[LoRaRadio] Transmission failed, code: "));
    debug.println(state);
    break;
  }
}
