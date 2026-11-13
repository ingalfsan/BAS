#include <Arduino.h>
#include <Wire.h>

#include "ThermalCore.h"
#include "Tmp117.h"

namespace {

constexpr int kSdaPin = 8;
constexpr int kSclPin = 9;
constexpr uint8_t kObjectAddress = 0x48;
constexpr uint8_t kAmbientAddress = 0x49;
constexpr uint32_t kSamplePeriodMs = 1000;
constexpr bool kSimulationMode = false;

enum ChannelStatus {
  CHANNEL_OK,
  CHANNEL_UNAVAILABLE,
  CHANNEL_READ_ERROR,
  CHANNEL_OUT_OF_RANGE,
};

struct ChannelReading {
  float celsius;
  ChannelStatus status;
};

Tmp117 objectSensor;
Tmp117 ambientSensor;
bool objectSensorAvailable = false;
bool ambientSensorAvailable = false;
ThermalCore thermal;
uint32_t lastSampleMs = 0;
uint64_t uptimeMs = 0;
uint64_t sequenceNumber = 0;

const char *statusName(ChannelStatus status) {
  switch (status) {
  case CHANNEL_OK:
    return "ok";
  case CHANNEL_UNAVAILABLE:
    return "sensor_unavailable";
  case CHANNEL_OUT_OF_RANGE:
    return "out_of_range";
  case CHANNEL_READ_ERROR:
  default:
    return "read_error";
  }
}

ChannelReading readChannel(Tmp117 &sensor, bool available) {
  ChannelReading reading = {0.0f, CHANNEL_UNAVAILABLE};
  if (!available) {
    return reading;
  }

  if (sensor.readCelsius(reading.celsius)) {
    reading.status = CHANNEL_OK;
  } else if (sensor.lastErrorWasOutOfRange()) {
    reading.status = CHANNEL_OUT_OF_RANGE;
  } else {
    reading.status = CHANNEL_READ_ERROR;
  }
  return reading;
}

void printChannel(const char *id, const ChannelReading &reading) {
  Serial.printf("{\"id\":\"%s\",\"celsius\":", id);
  if (reading.status == CHANNEL_OK) {
    Serial.printf("%.4f", static_cast<double>(reading.celsius));
  } else {
    Serial.print("null");
  }
  Serial.printf(",\"valid\":%s,\"status\":\"%s\"}",
                reading.status == CHANNEL_OK ? "true" : "false",
                statusName(reading.status));
}

void printTelemetry(const char *mode,
                    const ChannelReading &object,
                    const ChannelReading &ambient,
                    const ThermalMetrics &metrics) {
  Serial.printf("{\"schema_version\":\"1.0\",\"seq\":%llu,"
                "\"uptime_ms\":%llu,\"mode\":\"%s\",\"object\":",
                static_cast<unsigned long long>(sequenceNumber),
                static_cast<unsigned long long>(uptimeMs), mode);
  printChannel("object", object);
  Serial.print(",\"ambient\":");
  printChannel("ambient", ambient);
  Serial.print(",\"delta_celsius\":");
  if (metrics.deltaValid) {
    Serial.printf("%.4f", static_cast<double>(metrics.deltaCelsius));
  } else {
    Serial.print("null");
  }
  Serial.print(",\"delta_slope_c_per_s\":");
  if (metrics.trendValid) {
    Serial.printf("%.6f", static_cast<double>(metrics.deltaSlopeCPerS));
  } else {
    Serial.print("null");
  }
  Serial.printf(",\"trend_valid\":%s}\n",
                metrics.trendValid ? "true" : "false");
}

} // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("BAS thermal bench prototype 0.1.0; Arduino-ESP32");
  Serial.printf("Sample period: %lu ms\n",
                static_cast<unsigned long>(kSamplePeriodMs));

  thermal.begin(kSamplePeriodMs);

  if (!kSimulationMode) {
    if (kObjectAddress == kAmbientAddress) {
      Serial.println("ERROR: object and ambient TMP117 addresses must differ");
      while (true) {
        delay(1000);
      }
    }

    if (!Wire.begin(kSdaPin, kSclPin, 400000)) {
      Serial.println("ERROR: I2C bus initialization failed");
    } else {
      objectSensorAvailable = objectSensor.begin(Wire, kObjectAddress);
      ambientSensorAvailable = ambientSensor.begin(Wire, kAmbientAddress);
    }
    if (!objectSensorAvailable) {
      Serial.printf("WARN: object TMP117 not detected at 0x%02x\n",
                    kObjectAddress);
    }
    if (!ambientSensorAvailable) {
      Serial.printf("WARN: ambient TMP117 not detected at 0x%02x\n",
                    kAmbientAddress);
    }
  } else {
    Serial.println("SIMULATION enabled; values are synthetic, not sensor data");
  }

  lastSampleMs = millis();
}

void loop() {
  const uint32_t now = millis();
  const uint32_t elapsedMs = now - lastSampleMs;
  if (elapsedMs < kSamplePeriodMs) {
    delay(1);
    return;
  }
  lastSampleMs = now;
  uptimeMs += elapsedMs;

  ChannelReading object = {0.0f, CHANNEL_UNAVAILABLE};
  ChannelReading ambient = {0.0f, CHANNEL_UNAVAILABLE};
  const char *mode = "real";

  if (kSimulationMode) {
    const uint64_t ramp = sequenceNumber < 100 ? sequenceNumber : 100;
    object = {22.0f + static_cast<float>(ramp) * 0.03f, CHANNEL_OK};
    ambient = {22.0f, CHANNEL_OK};
    mode = "simulated";
  } else {
    object = readChannel(objectSensor, objectSensorAvailable);
    ambient = readChannel(ambientSensor, ambientSensorAvailable);
  }

  const ThermalMetrics metrics = thermal.update(
      uptimeMs,
      object.status == CHANNEL_OK,
      object.celsius,
      ambient.status == CHANNEL_OK,
      ambient.celsius);
  printTelemetry(mode, object, ambient, metrics);
  sequenceNumber++;
}
