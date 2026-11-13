#include "thermal.h"

#include <math.h>
#include <string.h>

void thermal_state_init(thermal_state_t *state, uint64_t sample_period_ms)
{
    if (state == NULL) {
        return;
    }

    memset(state, 0, sizeof(*state));
    if (sample_period_ms == 0) {
        sample_period_ms = 1000;
    }
    state->max_gap_ms = sample_period_ms * 2;
}

static void clear_trend(thermal_state_t *state)
{
    state->count = 0;
}

static void append_delta(thermal_state_t *state, uint64_t timestamp_ms, float delta)
{
    if (state->count > 0) {
        const size_t last = state->count - 1;
        if (timestamp_ms <= state->timestamps_ms[last] ||
            timestamp_ms - state->timestamps_ms[last] > state->max_gap_ms) {
            clear_trend(state);
        }
    }

    if (state->count == THERMAL_TREND_WINDOW_SIZE) {
        memmove(&state->deltas[0], &state->deltas[1],
                (THERMAL_TREND_WINDOW_SIZE - 1) * sizeof(state->deltas[0]));
        memmove(&state->timestamps_ms[0], &state->timestamps_ms[1],
                (THERMAL_TREND_WINDOW_SIZE - 1) * sizeof(state->timestamps_ms[0]));
        state->count--;
    }

    state->deltas[state->count] = delta;
    state->timestamps_ms[state->count] = timestamp_ms;
    state->count++;
}

static bool calculate_slope(const thermal_state_t *state, float *slope)
{
    if (state->count < THERMAL_TREND_WINDOW_SIZE || slope == NULL) {
        return false;
    }

    const double origin_ms = (double)state->timestamps_ms[0];
    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_xx = 0.0;
    double sum_xy = 0.0;

    for (size_t i = 0; i < state->count; i++) {
        const double x = ((double)state->timestamps_ms[i] - origin_ms) / 1000.0;
        const double y = state->deltas[i];
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }

    const double n = (double)state->count;
    const double denominator = n * sum_xx - sum_x * sum_x;
    if (denominator <= 0.0) {
        return false;
    }

    const double result = (n * sum_xy - sum_x * sum_y) / denominator;
    if (!isfinite(result)) {
        return false;
    }

    *slope = (float)result;
    return true;
}

void thermal_state_update(
    thermal_state_t *state,
    uint64_t timestamp_ms,
    bool object_valid,
    float object_celsius,
    bool ambient_valid,
    float ambient_celsius,
    thermal_metrics_t *metrics)
{
    if (metrics == NULL) {
        return;
    }
    memset(metrics, 0, sizeof(*metrics));

    if (state == NULL) {
        return;
    }

    if (!object_valid || !ambient_valid ||
        !isfinite(object_celsius) || !isfinite(ambient_celsius)) {
        clear_trend(state);
        return;
    }

    const float delta = object_celsius - ambient_celsius;
    if (!isfinite(delta)) {
        clear_trend(state);
        return;
    }

    metrics->delta_celsius = delta;
    metrics->delta_valid = true;
    append_delta(state, timestamp_ms, delta);
    metrics->trend_valid = calculate_slope(state, &metrics->delta_slope_c_per_s);
}
