#include "Tmp117.h"

#include <math.h>

namespace {

constexpr uint8_t kTemperatureRegister = 0x00;
constexpr uint8_t kDeviceIdRegister = 0x0f;
constexpr uint16_t kDeviceId = 0x0117;
constexpr float kCelsiusPerLsb = 0.0078125f;
constexpr float kMinCelsius = -55.0f;
constexpr float kMaxCelsius = 150.0f;

} // namespace

Tmp117::Tmp117() : wire_(nullptr), address_(0), outOfRange_(false) {}

bool Tmp117::begin(TwoWire &wire, uint8_t address) {
  if (address < 0x48 || address > 0x4b) {
    return false;
  }
  wire_ = &wire;
  address_ = address;
  uint16_t deviceId = 0;
  if (!readRegister(kDeviceIdRegister, deviceId) || deviceId != kDeviceId) {
    wire_ = nullptr;
    return false;
  }
  return true;
}

bool Tmp117::readRegister(uint8_t registerAddress, uint16_t &value) {
  if (wire_ == nullptr) {
    return false;
  }

  wire_->beginTransmission(address_);
  wire_->write(registerAddress);
  if (wire_->endTransmission(false) != 0) {
    return false;
  }
  if (wire_->requestFrom(address_, static_cast<size_t>(2), true) !=
      static_cast<size_t>(2)) {
    while (wire_->available()) {
      wire_->read();
    }
    return false;
  }

  const int highByte = wire_->read();
  const int lowByte = wire_->read();
  if (highByte < 0 || lowByte < 0) {
    return false;
  }
  value = (static_cast<uint16_t>(highByte) << 8) |
          static_cast<uint16_t>(lowByte);
  return true;
}

bool Tmp117::readCelsius(float &celsius) {
  outOfRange_ = false;
  uint16_t rawValue = 0;
  if (!readRegister(kTemperatureRegister, rawValue)) {
    return false;
  }

  const int16_t signedValue = static_cast<int16_t>(rawValue);
  const float measurement = static_cast<float>(signedValue) * kCelsiusPerLsb;
  if (!isfinite(measurement) ||
      measurement < kMinCelsius || measurement > kMaxCelsius) {
    outOfRange_ = true;
    return false;
  }

  celsius = measurement;
  return true;
}

bool Tmp117::lastErrorWasOutOfRange() const {
  return outOfRange_;
}
