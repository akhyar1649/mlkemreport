#include "scenario_local.h"

#include "esp_cpu.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "metrics.h"
#include "microbench.h"
#include "mlkem_config.h"
#include "mlkem_util.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdlib.h>
#include <string.h>

static const char *TAG = "scenario_local";

static void log_op(uint32_t iter,
                   const char *op,
                   uint32_t cycles,
                   int64_t time_us,
                   uint64_t energy_uj) {
    uint32_t heap_free = esp_get_free_heap_size();
    uint32_t heap_min = esp_get_minimum_free_heap_size();
    uint32_t stack_hwm_bytes = (uint32_t)uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t);
    metrics_log_s2(iter, MLKEM_VARIANT_NAME, op, cycles, time_us, energy_uj, heap_free, heap_min, stack_hwm_bytes);
}

#if CONFIG_MLKEM_SELFTEST
    static bool mlkem_selftest(uint8_t *pk,
                            uint8_t *sk,
                            uint8_t *ct,
                            uint8_t *ss_enc,
                            uint8_t *ss_dec) {
        int ret = mlkem_keypair(pk, sk);
        if (ret != 0) {
            return false;
        }
        ret = mlkem_encaps(ct, ss_enc, pk);
        if (ret != 0) {
            return false;
        }
        ret = mlkem_decaps(ss_dec, ct, sk);
        if (ret != 0) {
            return false;
        }
        return memcmp(ss_enc, ss_dec, MLKEM_SHAREDSECRET_BYTES) == 0;
    }
#endif

void scenario_local_run(energy_sampler_t *sampler) {
    ESP_LOGI(TAG, "Scenario 2: local compute (%s)", MLKEM_VARIANT_NAME);
    metrics_log_header_s2();

    uint8_t *pk = heap_caps_malloc(MLKEM_PUBLICKEY_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *sk = heap_caps_malloc(MLKEM_SECRETKEY_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *ct = heap_caps_malloc(MLKEM_CIPHERTEXT_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *ss_enc = heap_caps_malloc(MLKEM_SHAREDSECRET_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *ss_dec = heap_caps_malloc(MLKEM_SHAREDSECRET_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (!pk || !sk || !ct || !ss_enc || !ss_dec) {
        ESP_LOGE(TAG, "Heap allocation failed for ML-KEM buffers");
        goto cleanup;
    }

#if CONFIG_MLKEM_SELFTEST
    if (!mlkem_selftest(pk, sk, ct, ss_enc, ss_dec)) {
        ESP_LOGE(TAG, "Self-test failed: shared secret mismatch");
    } else {
        ESP_LOGI(TAG, "Self-test passed");
    }
#endif

#if CONFIG_MLKEM_MICROBENCH
    mlkem_microbench_run(sampler);
#endif

    for (uint32_t iter = 0; iter < CONFIG_MLKEM_ITERATIONS; iter++) {
        energy_result_t energy = {0};

        energy_sampler_start(sampler);
        int64_t t0 = esp_timer_get_time();
        uint32_t c0 = esp_cpu_get_cycle_count();
        int ret = mlkem_keypair(pk, sk);
        uint32_t c1 = esp_cpu_get_cycle_count();
        int64_t t1 = esp_timer_get_time();
        energy_sampler_stop(sampler, &energy);
        if (ret != 0) {
            ESP_LOGE(TAG, "KeyGen failed (iter %u)", (unsigned)iter);
        }
        log_op(iter, "keygen", mlkem_cycle_diff(c0, c1), (t1 - t0), energy.energy_uj);

        energy_sampler_start(sampler);
        t0 = esp_timer_get_time();
        c0 = esp_cpu_get_cycle_count();
        ret = mlkem_encaps(ct, ss_enc, pk);
        c1 = esp_cpu_get_cycle_count();
        t1 = esp_timer_get_time();
        energy_sampler_stop(sampler, &energy);
        if (ret != 0) {
            ESP_LOGE(TAG, "Encaps failed (iter %u)", (unsigned)iter);
        }
        log_op(iter, "encaps", mlkem_cycle_diff(c0, c1), (t1 - t0), energy.energy_uj);

        energy_sampler_start(sampler);
        t0 = esp_timer_get_time();
        c0 = esp_cpu_get_cycle_count();
        ret = mlkem_decaps(ss_dec, ct, sk);
        c1 = esp_cpu_get_cycle_count();
        t1 = esp_timer_get_time();
        energy_sampler_stop(sampler, &energy);
        if (ret != 0) {
            ESP_LOGE(TAG, "Decaps failed (iter %u)", (unsigned)iter);
        }
        log_op(iter, "decaps", mlkem_cycle_diff(c0, c1), (t1 - t0), energy.energy_uj);

        mlkem_secure_zero(pk, MLKEM_PUBLICKEY_BYTES);
        mlkem_secure_zero(sk, MLKEM_SECRETKEY_BYTES);
        mlkem_secure_zero(ct, MLKEM_CIPHERTEXT_BYTES);
        mlkem_secure_zero(ss_enc, MLKEM_SHAREDSECRET_BYTES);
        mlkem_secure_zero(ss_dec, MLKEM_SHAREDSECRET_BYTES);
    }

cleanup:
    if (pk) {
        mlkem_secure_zero(pk, MLKEM_PUBLICKEY_BYTES);
        free(pk);
    }
    if (sk) {
        mlkem_secure_zero(sk, MLKEM_SECRETKEY_BYTES);
        free(sk);
    }
    if (ct) {
        mlkem_secure_zero(ct, MLKEM_CIPHERTEXT_BYTES);
        free(ct);
    }
    if (ss_enc) {
        mlkem_secure_zero(ss_enc, MLKEM_SHAREDSECRET_BYTES);
        free(ss_enc);
    }
    if (ss_dec) {
        mlkem_secure_zero(ss_dec, MLKEM_SHAREDSECRET_BYTES);
        free(ss_dec);
    }
}
