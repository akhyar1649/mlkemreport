#ifndef ENERGY_SAMPLER_H
#define ENERGY_SAMPLER_H

#include "ina219.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t energy_uj;
    uint64_t samples;
    int64_t duration_us;
} energy_result_t;

typedef struct {
    ina219_dev_t dev;
    int64_t start_us;
    int64_t last_sample_us;
    uint64_t energy_uj;
    uint64_t samples;
    uint32_t sample_period_ms;
    float last_power_w;
    bool running;
    TaskHandle_t task;
    portMUX_TYPE lock;
} energy_sampler_t;

esp_err_t energy_sampler_init(energy_sampler_t *sampler);

void energy_sampler_start(energy_sampler_t *sampler);

void energy_sampler_stop(energy_sampler_t *sampler, energy_result_t *out);

#ifdef __cplusplus
}
#endif

#endif
