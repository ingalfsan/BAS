#ifndef BAS_THERMAL_H
#define BAS_THERMAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define THERMAL_TREND_WINDOW_SIZE 10

typedef struct {
    float delta_celsius;
    float delta_slope_c_per_s;
    bool delta_valid;
    bool trend_valid;
} thermal_metrics_t;

typedef struct {
    float deltas[THERMAL_TREND_WINDOW_SIZE];
    uint64_t timestamps_ms[THERMAL_TREND_WINDOW_SIZE];
    size_t count;
    uint64_t max_gap_ms;
} thermal_state_t;

void thermal_state_init(thermal_state_t *state, uint64_t sample_period_ms);

void thermal_state_update(
    thermal_state_t *state,
    uint64_t timestamp_ms,
    bool object_valid,
    float object_celsius,
    bool ambient_valid,
    float ambient_celsius,
    thermal_metrics_t *metrics);

#endif
