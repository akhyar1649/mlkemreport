#include "energy_sampler.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "energy";

static void energy_sampler_task(void *arg) {
    energy_sampler_t *s = (energy_sampler_t *)arg;

    TickType_t delay_ticks = pdMS_TO_TICKS(s->sample_period_ms);
    if (delay_ticks < 1) {
        delay_ticks = 1;
    }
    TickType_t idle_delay_ticks = pdMS_TO_TICKS(1);
    if (idle_delay_ticks < 1) {
        idle_delay_ticks = 1;
    }

    while (1) {
        if (!s->running) {
            vTaskDelay(idle_delay_ticks);
            continue;
        }

        float bus_mv = 0.0f;
        float cur_ma = 0.0f;
        int64_t now_us = esp_timer_get_time();
        if (ina219_read_bus_voltage_mv(&s->dev, &bus_mv) == ESP_OK &&
            ina219_read_current_ma(&s->dev, &cur_ma) == ESP_OK) {
            float power_w = (bus_mv / 1000.0f) * (cur_ma / 1000.0f);
            int64_t delta_us = now_us - s->last_sample_us;
            if (delta_us > 0) {
                portENTER_CRITICAL(&s->lock);
                s->energy_uj += (uint64_t)(power_w * (float)delta_us);
                s->samples++;
                s->last_power_w = power_w;
                s->last_sample_us = now_us;
                portEXIT_CRITICAL(&s->lock);
            }
        }

        vTaskDelay(delay_ticks);
    }
}

esp_err_t energy_sampler_init(energy_sampler_t *sampler) {
    if (!sampler) {
        return ESP_ERR_INVALID_ARG;
    }

    sampler->sample_period_ms = CONFIG_INA219_SAMPLE_PERIOD_MS;
    sampler->running = false;
    sampler->energy_uj = 0;
    sampler->samples = 0;
    sampler->start_us = 0;
    sampler->last_sample_us = 0;
    sampler->last_power_w = 0.0f;
    sampler->lock = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED;

    esp_err_t err = ina219_init(&sampler->dev);
    if (err != ESP_OK) {
        return err;
    }

    BaseType_t ok = xTaskCreatePinnedToCore(
        energy_sampler_task,
        "energy_sampler",
        4096,
        sampler,
        tskIDLE_PRIORITY + 1,
        &sampler->task,
        0);
    if (ok != pdPASS) {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Energy sampler ready (%ums)", (unsigned)sampler->sample_period_ms);
    return ESP_OK;
}

void energy_sampler_start(energy_sampler_t *sampler) {
    if (!sampler) {
        return;
    }

    portENTER_CRITICAL(&sampler->lock);
    sampler->energy_uj = 0;
    sampler->samples = 0;
    sampler->start_us = esp_timer_get_time();
    sampler->last_sample_us = sampler->start_us;
    sampler->running = true;
    portEXIT_CRITICAL(&sampler->lock);

    float bus_mv = 0.0f;
    float cur_ma = 0.0f;
    if (ina219_read_bus_voltage_mv(&sampler->dev, &bus_mv) == ESP_OK &&
        ina219_read_current_ma(&sampler->dev, &cur_ma) == ESP_OK) {
        float power_w = (bus_mv / 1000.0f) * (cur_ma / 1000.0f);
        portENTER_CRITICAL(&sampler->lock);
        sampler->last_power_w = power_w;
        portEXIT_CRITICAL(&sampler->lock);
    }
}

void energy_sampler_stop(energy_sampler_t *sampler, energy_result_t *out) {
    if (!sampler || !out) {
        return;
    }

    int64_t stop_us = esp_timer_get_time();

    portENTER_CRITICAL(&sampler->lock);
    sampler->running = false;
    if (sampler->samples == 0 && sampler->last_power_w > 0.0f) {
        out->energy_uj = (uint64_t)(sampler->last_power_w * (float)(stop_us - sampler->start_us));
        out->samples = 1;
    } else {
        out->energy_uj = sampler->energy_uj;
        out->samples = sampler->samples;
    }
    out->duration_us = stop_us - sampler->start_us;
    portEXIT_CRITICAL(&sampler->lock);
}
