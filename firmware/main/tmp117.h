#ifndef BAS_TMP117_H
#define BAS_TMP117_H

#include "driver/i2c_master.h"
#include "esp_err.h"

#include <stdint.h>

typedef struct {
    i2c_master_dev_handle_t device;
} tmp117_t;

esp_err_t tmp117_open(
    i2c_master_bus_handle_t bus,
    uint8_t address,
    tmp117_t *sensor);

esp_err_t tmp117_read_celsius(const tmp117_t *sensor, float *celsius);

void tmp117_close(tmp117_t *sensor);

#endif
