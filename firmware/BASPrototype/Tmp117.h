#ifndef BAS_ARDUINO_TMP117_H
#define BAS_ARDUINO_TMP117_H

#include <Arduino.h>
#include <Wire.h>

class Tmp117 {
public:
  Tmp117();

  bool begin(TwoWire &wire, uint8_t address);
  bool readCelsius(float &celsius);
  bool lastErrorWasOutOfRange() const;

private:
  TwoWire *wire_;
  uint8_t address_;
  bool outOfRange_;

  bool readRegister(uint8_t registerAddress, uint16_t &value);
};

#endif
