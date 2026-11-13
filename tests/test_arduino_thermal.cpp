#include "ThermalCore.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void testDeltaNeedsBothChannels() {
  ThermalCore core;
  core.begin(1000);

  ThermalMetrics metrics = core.update(1000, true, 25.0f, true, 22.0f);
  assert(metrics.deltaValid);
  assert(fabsf(metrics.deltaCelsius - 3.0f) < 0.0001f);
  assert(!metrics.trendValid);

  metrics = core.update(2000, true, 25.0f, false, 22.0f);
  assert(!metrics.deltaValid);
  assert(!metrics.trendValid);
}

static void testTenSamplesProduceLinearTrend() {
  ThermalCore core;
  core.begin(1000);
  ThermalMetrics metrics = {};

  for (uint64_t i = 0; i < THERMAL_TREND_WINDOW_SIZE; i++) {
    metrics = core.update(1000 + i * 1000, true,
                          20.0f + static_cast<float>(i) * 0.5f,
                          true, 20.0f);
  }

  assert(metrics.trendValid);
  assert(fabsf(metrics.deltaSlopeCPerS - 0.5f) < 0.0001f);
}

static void testGapClearsTrend() {
  ThermalCore core;
  core.begin(1000);
  for (uint64_t i = 0; i < THERMAL_TREND_WINDOW_SIZE; i++) {
    core.update(1000 + i * 1000, true, 24.0f, true, 20.0f);
  }

  const ThermalMetrics metrics = core.update(12000, true, 24.0f, true, 20.0f);
  assert(metrics.deltaValid);
  assert(!metrics.trendValid);
}

int main() {
  testDeltaNeedsBothChannels();
  testTenSamplesProduceLinearTrend();
  testGapClearsTrend();
  puts("Arduino thermal core tests: PASS");
  return 0;
}
