#include "energy_sampler.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "driver/uart.h"
#include "mlkem_config.h"
#include "scenario_local.h"
#include "scenario_mqtt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>

static const char *TAG = "app";

#define MLKEM_TASK_STACK_BYTES (32 * 1024)
#define MLKEM_TASK_STACK_WORDS (MLKEM_TASK_STACK_BYTES / sizeof(StackType_t))

static energy_sampler_t s_sampler;

static void mlkem_task(void *arg) {
    energy_sampler_t *sampler = (energy_sampler_t *)arg;

#if CONFIG_SCENARIO_1
    scenario_mqtt_run(sampler);
#else
    scenario_local_run(sampler);
#endif

    vTaskDelete(NULL);
}

static void configure_cpu_policy(void) {
#if CONFIG_SCENARIO_2 && CONFIG_PM_ENABLE
    esp_pm_config_t pm_cfg = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 240,
        .light_sleep_enable = false,
    };
    esp_err_t err = esp_pm_configure(&pm_cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CPU frequency locked to 240 MHz (Scenario 2)");
    } else {
        ESP_LOGW(TAG, "CPU frequency lock failed (%d)", err);
    }
#endif
}

void app_main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    uart_set_baudrate(UART_NUM_0, CONFIG_MLKEM_UART_BAUDRATE);

    ESP_LOGI(TAG, "ML-KEM app start: %s", MLKEM_VARIANT_NAME);
    ESP_LOGI(TAG, "Iterations: %d", CONFIG_MLKEM_ITERATIONS);

    configure_cpu_policy();

    s_sampler = (energy_sampler_t){0};
    if (energy_sampler_init(&s_sampler) != ESP_OK) {
        ESP_LOGW(TAG, "Energy sampler init failed");
    }

    BaseType_t ok = xTaskCreatePinnedToCore(
        mlkem_task,
        "mlkem_task",
        MLKEM_TASK_STACK_WORDS,
        &s_sampler,
        tskIDLE_PRIORITY + 2,
        NULL,
        1);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "Failed to start ML-KEM task");
    }
}
