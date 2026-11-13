#include "thermal.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static thermal_metrics_t update(
    thermal_state_t *state,
    uint64_t time_ms,
    bool object_valid,
    float object_celsius,
    bool ambient_valid,
    float ambient_celsius)
{
    thermal_metrics_t metrics;
    thermal_state_update(state, time_ms, object_valid, object_celsius,
                         ambient_valid, ambient_celsius, &metrics);
    return metrics;
}

static void test_delta_requires_both_valid_channels(void)
{
    thermal_state_t state;
    thermal_state_init(&state, 1000);

    thermal_metrics_t metrics = update(&state, 1000, true, 25.0f, true, 22.0f);
    assert(metrics.delta_valid);
    assert(fabsf(metrics.delta_celsius - 3.0f) < 0.0001f);
    assert(!metrics.trend_valid);

    metrics = update(&state, 2000, true, 25.0f, false, 22.0f);
    assert(!metrics.delta_valid);
    assert(!metrics.trend_valid);
    assert(state.count == 0);
}

static void test_ten_samples_produce_linear_trend(void)
{
    thermal_state_t state;
    thermal_state_init(&state, 1000);

    thermal_metrics_t metrics = {0};
    for (uint64_t i = 0; i < THERMAL_TREND_WINDOW_SIZE; i++) {
        const float object = 20.0f + (float)i * 0.5f;
        metrics = update(&state, 1000 + i * 1000, true, object, true, 20.0f);
    }

    assert(metrics.trend_valid);
    assert(fabsf(metrics.delta_slope_c_per_s - 0.5f) < 0.0001f);
}

static void test_gap_resets_trend_window(void)
{
    thermal_state_t state;
    thermal_state_init(&state, 1000);

    for (uint64_t i = 0; i < THERMAL_TREND_WINDOW_SIZE; i++) {
        update(&state, 1000 + i * 1000, true, 24.0f, true, 20.0f);
    }
    assert(state.count == THERMAL_TREND_WINDOW_SIZE);

    thermal_metrics_t metrics = update(&state, 12000, true, 24.0f, true, 20.0f);
    assert(metrics.delta_valid);
    assert(!metrics.trend_valid);
    assert(state.count == 1);
}

static void test_non_finite_reading_is_rejected(void)
{
    thermal_state_t state;
    thermal_state_init(&state, 1000);

    const thermal_metrics_t metrics =
        update(&state, 1000, true, NAN, true, 20.0f);
    assert(!metrics.delta_valid);
    assert(state.count == 0);
}

int main(void)
{
    test_delta_requires_both_valid_channels();
    test_ten_samples_produce_linear_trend();
    test_gap_resets_trend_window();
    test_non_finite_reading_is_rejected();
    puts("thermal tests: PASS");
    return 0;
}
