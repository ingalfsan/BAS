#include "tmp117.h"

#include <math.h>
#include <stddef.h>

#define TMP117_REG_TEMPERATURE 0x00
#define TMP117_REG_DEVICE_ID 0x0f
#define TMP117_DEVICE_ID 0x0117
#define TMP117_CELSIUS_PER_LSB 0.0078125f
#define TMP117_MIN_CELSIUS (-55.0f)
#define TMP117_MAX_CELSIUS 150.0f
#define TMP117_TIMEOUT_MS 100

static esp_err_t read_register(
    const tmp117_t *sensor,
    uint8_t register_address,
    uint16_t *value)
{
    if (sensor == NULL || sensor->device == NULL || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t bytes[2] = {0};
    const esp_err_t result = i2c_master_transmit_receive(
        sensor->device,
        &register_address,
        sizeof(register_address),
        bytes,
        sizeof(bytes),
        TMP117_TIMEOUT_MS);
    if (result != ESP_OK) {
        return result;
    }

    *value = ((uint16_t)bytes[0] << 8) | bytes[1];
    return ESP_OK;
}

esp_err_t tmp117_open(
    i2c_master_bus_handle_t bus,
    uint8_t address,
    tmp117_t *sensor)
{
    if (bus == NULL || sensor == NULL || address < 0x48 || address > 0x4b) {
        return ESP_ERR_INVALID_ARG;
    }
    sensor->device = NULL;

    const i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 400000,
    };
    esp_err_t result = i2c_master_bus_add_device(bus, &config, &sensor->device);
    if (result != ESP_OK) {
        return result;
    }

    uint16_t device_id = 0;
    result = read_register(sensor, TMP117_REG_DEVICE_ID, &device_id);
    if (result == ESP_OK && device_id != TMP117_DEVICE_ID) {
        result = ESP_ERR_INVALID_RESPONSE;
    }
    if (result != ESP_OK) {
        tmp117_close(sensor);
    }
    return result;
}

esp_err_t tmp117_read_celsius(const tmp117_t *sensor, float *celsius)
{
    if (sensor == NULL || sensor->device == NULL || celsius == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t raw_value = 0;
    const esp_err_t result = read_register(sensor, TMP117_REG_TEMPERATURE, &raw_value);
    if (result != ESP_OK) {
        return result;
    }

    const int16_t signed_value = (int16_t)raw_value;
    const float measurement = (float)signed_value * TMP117_CELSIUS_PER_LSB;
    if (!isfinite(measurement) ||
        measurement < TMP117_MIN_CELSIUS ||
        measurement > TMP117_MAX_CELSIUS) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    *celsius = measurement;
    return ESP_OK;
}

void tmp117_close(tmp117_t *sensor)
{
    if (sensor != NULL && sensor->device != NULL) {
        i2c_master_bus_rm_device(sensor->device);
        sensor->device = NULL;
    }
}
