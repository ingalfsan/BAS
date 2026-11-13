#include "thermal.h"
#include "tmp117.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

static const char *TAG = "bas";

typedef enum {
    CHANNEL_OK,
    CHANNEL_UNAVAILABLE,
    CHANNEL_READ_ERROR,
    CHANNEL_OUT_OF_RANGE,
} channel_status_t;

typedef struct {
    float celsius;
    channel_status_t status;
} channel_reading_t;

static const char *channel_status_name(channel_status_t status)
{
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

#if !CONFIG_BAS_SIMULATION
static channel_reading_t read_channel(const tmp117_t *sensor)
{
    channel_reading_t reading = {
        .celsius = 0.0f,
        .status = CHANNEL_UNAVAILABLE,
    };

    if (sensor == NULL || sensor->device == NULL) {
        return reading;
    }

    const esp_err_t result = tmp117_read_celsius(sensor, &reading.celsius);
    if (result == ESP_OK) {
        reading.status = CHANNEL_OK;
    } else if (result == ESP_ERR_INVALID_RESPONSE) {
        reading.status = CHANNEL_OUT_OF_RANGE;
    } else {
        reading.status = CHANNEL_READ_ERROR;
    }
    return reading;
}
#endif

static void print_channel(const char *id, const channel_reading_t *reading)
{
    printf("{\"id\":\"%s\",\"celsius\":", id);
    if (reading->status == CHANNEL_OK) {
        printf("%.4f", (double)reading->celsius);
    } else {
        printf("null");
    }
    printf(",\"valid\":%s,\"status\":\"%s\"}",
           reading->status == CHANNEL_OK ? "true" : "false",
           channel_status_name(reading->status));
}

static void print_telemetry(
    uint64_t sequence,
    uint64_t uptime_ms,
    const char *mode,
    const channel_reading_t *object,
    const channel_reading_t *ambient,
    const thermal_metrics_t *metrics)
{
    printf("{\"schema_version\":\"1.0\",\"seq\":%" PRIu64
           ",\"uptime_ms\":%" PRIu64 ",\"mode\":\"%s\",\"object\":",
           sequence, uptime_ms, mode);
    print_channel("object", object);
    printf(",\"ambient\":");
    print_channel("ambient", ambient);
    printf(",\"delta_celsius\":");
    if (metrics->delta_valid) {
        printf("%.4f", (double)metrics->delta_celsius);
    } else {
        printf("null");
    }
    printf(",\"delta_slope_c_per_s\":");
    if (metrics->trend_valid) {
        printf("%.6f", (double)metrics->delta_slope_c_per_s);
    } else {
        printf("null");
    }
    printf(",\"trend_valid\":%s}\n", metrics->trend_valid ? "true" : "false");
}

void app_main(void)
{
    if (CONFIG_BAS_I2C_SDA_GPIO == CONFIG_BAS_I2C_SCL_GPIO) {
        ESP_LOGE(TAG, "I2C SDA and SCL must use different GPIOs");
        return;
    }

    ESP_LOGI(TAG, "BAS thermal bench prototype 0.1.0; target ESP32-S3");
    ESP_LOGI(TAG, "Sample period: %d ms", CONFIG_BAS_SAMPLE_PERIOD_MS);

    thermal_state_t thermal_state;
    thermal_state_init(&thermal_state, CONFIG_BAS_SAMPLE_PERIOD_MS);

#if CONFIG_BAS_SIMULATION
    ESP_LOGW(TAG, "SIMULATION enabled; telemetry is synthetic, not sensor data");
#else
    tmp117_t object_sensor = {0};
    tmp117_t ambient_sensor = {0};
    i2c_master_bus_handle_t bus = NULL;

    if (CONFIG_BAS_OBJECT_SENSOR_ADDRESS == CONFIG_BAS_AMBIENT_SENSOR_ADDRESS) {
        ESP_LOGE(TAG, "Object and ambient TMP117 addresses must differ");
        return;
    }

    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = CONFIG_BAS_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_BAS_I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus));

    if (tmp117_open(bus, CONFIG_BAS_OBJECT_SENSOR_ADDRESS, &object_sensor) != ESP_OK) {
        ESP_LOGW(TAG, "Object sensor not detected at 0x%02x",
                 CONFIG_BAS_OBJECT_SENSOR_ADDRESS);
    }
    if (tmp117_open(bus, CONFIG_BAS_AMBIENT_SENSOR_ADDRESS, &ambient_sensor) != ESP_OK) {
        ESP_LOGW(TAG, "Ambient sensor not detected at 0x%02x",
                 CONFIG_BAS_AMBIENT_SENSOR_ADDRESS);
    }
#endif

    uint64_t sequence = 0;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t sample_period = pdMS_TO_TICKS(CONFIG_BAS_SAMPLE_PERIOD_MS);

    while (true) {
        channel_reading_t object = {0};
        channel_reading_t ambient = {0};
        const char *mode = "real";

#if CONFIG_BAS_SIMULATION
        const float simulated_object = 22.0f + (float)(sequence < 100 ? sequence : 100) * 0.03f;
        object = (channel_reading_t){.celsius = simulated_object, .status = CHANNEL_OK};
        ambient = (channel_reading_t){.celsius = 22.0f, .status = CHANNEL_OK};
        mode = "simulated";
#else
        object = read_channel(&object_sensor);
        ambient = read_channel(&ambient_sensor);
#endif

        const uint64_t uptime_ms = (uint64_t)(esp_timer_get_time() / 1000);
        thermal_metrics_t metrics;
        thermal_state_update(
            &thermal_state,
            uptime_ms,
            object.status == CHANNEL_OK,
            object.celsius,
            ambient.status == CHANNEL_OK,
            ambient.celsius,
            &metrics);
        print_telemetry(sequence, uptime_ms, mode, &object, &ambient, &metrics);
        sequence++;

        vTaskDelayUntil(&last_wake, sample_period);
    }
}
