#include "ThermalCore.h"

#include <math.h>
#include <string.h>

ThermalCore::ThermalCore() : count_(0), maxGapMs_(2000) {
  memset(deltas_, 0, sizeof(deltas_));
  memset(timestampsMs_, 0, sizeof(timestampsMs_));
}

void ThermalCore::begin(uint64_t samplePeriodMs) {
  memset(deltas_, 0, sizeof(deltas_));
  memset(timestampsMs_, 0, sizeof(timestampsMs_));
  count_ = 0;
  maxGapMs_ = samplePeriodMs == 0 ? 2000 : samplePeriodMs * 2;
}

void ThermalCore::clearTrend() {
  count_ = 0;
}

void ThermalCore::appendDelta(uint64_t timestampMs, float delta) {
  if (count_ > 0) {
    const size_t last = count_ - 1;
    if (timestampMs <= timestampsMs_[last] ||
        timestampMs - timestampsMs_[last] > maxGapMs_) {
      clearTrend();
    }
  }

  if (count_ == THERMAL_TREND_WINDOW_SIZE) {
    memmove(&deltas_[0], &deltas_[1],
            (THERMAL_TREND_WINDOW_SIZE - 1) * sizeof(deltas_[0]));
    memmove(&timestampsMs_[0], &timestampsMs_[1],
            (THERMAL_TREND_WINDOW_SIZE - 1) * sizeof(timestampsMs_[0]));
    count_--;
  }

  deltas_[count_] = delta;
  timestampsMs_[count_] = timestampMs;
  count_++;
}

bool ThermalCore::calculateSlope(float &slope) const {
  if (count_ < THERMAL_TREND_WINDOW_SIZE) {
    return false;
  }

  const double originMs = static_cast<double>(timestampsMs_[0]);
  double sumX = 0.0;
  double sumY = 0.0;
  double sumXX = 0.0;
  double sumXY = 0.0;

  for (size_t i = 0; i < count_; i++) {
    const double x = (static_cast<double>(timestampsMs_[i]) - originMs) / 1000.0;
    const double y = deltas_[i];
    sumX += x;
    sumY += y;
    sumXX += x * x;
    sumXY += x * y;
  }

  const double n = static_cast<double>(count_);
  const double denominator = n * sumXX - sumX * sumX;
  if (denominator <= 0.0) {
    return false;
  }

  const double result = (n * sumXY - sumX * sumY) / denominator;
  if (!isfinite(result)) {
    return false;
  }

  slope = static_cast<float>(result);
  return true;
}

ThermalMetrics ThermalCore::update(uint64_t timestampMs,
                                   bool objectValid,
                                   float objectCelsius,
                                   bool ambientValid,
                                   float ambientCelsius) {
  ThermalMetrics metrics = {};
  if (!objectValid || !ambientValid ||
      !isfinite(objectCelsius) || !isfinite(ambientCelsius)) {
    clearTrend();
    return metrics;
  }

  const float delta = objectCelsius - ambientCelsius;
  if (!isfinite(delta)) {
    clearTrend();
    return metrics;
  }

  metrics.deltaCelsius = delta;
  metrics.deltaValid = true;
  appendDelta(timestampMs, delta);
  metrics.trendValid = calculateSlope(metrics.deltaSlopeCPerS);
  return metrics;
}
