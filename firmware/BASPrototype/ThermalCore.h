#ifndef BAS_ARDUINO_THERMAL_CORE_H
#define BAS_ARDUINO_THERMAL_CORE_H

#include <stddef.h>
#include <stdint.h>

constexpr size_t THERMAL_TREND_WINDOW_SIZE = 10;

struct ThermalMetrics {
  float deltaCelsius;
  float deltaSlopeCPerS;
  bool deltaValid;
  bool trendValid;
};

class ThermalCore {
public:
  ThermalCore();

  void begin(uint64_t samplePeriodMs);
  ThermalMetrics update(uint64_t timestampMs,
                        bool objectValid,
                        float objectCelsius,
                        bool ambientValid,
                        float ambientCelsius);

private:
  float deltas_[THERMAL_TREND_WINDOW_SIZE];
  uint64_t timestampsMs_[THERMAL_TREND_WINDOW_SIZE];
  size_t count_;
  uint64_t maxGapMs_;

  void clearTrend();
  void appendDelta(uint64_t timestampMs, float delta);
  bool calculateSlope(float &slope) const;
};

#endif
