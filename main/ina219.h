#ifndef INA219_H
#define INA219_H

#include "driver/i2c.h"
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    i2c_port_t port;
    uint8_t addr;
    float shunt_ohms;
    float current_lsb;
    float power_lsb;
} ina219_dev_t;

esp_err_t ina219_init(ina219_dev_t *dev);

esp_err_t ina219_read_bus_voltage_mv(ina219_dev_t *dev, float *bus_mv);

esp_err_t ina219_read_current_ma(ina219_dev_t *dev, float *current_ma);

#ifdef __cplusplus
}
#endif

#endif
