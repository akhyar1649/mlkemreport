#include "microbench.h"

#include "esp_cpu.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "metrics.h"
#include "mlkem_config.h"
#include "mlkem_util.h"
#include "ntt.h"
#include "params.h"
#include "poly.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>

static const char *TAG = "microbench";

#if CONFIG_MLKEM_512
#define MB_POLY_NTT PQCLEAN_MLKEM512_CLEAN_poly_ntt
#define MB_POLY_INVNTT PQCLEAN_MLKEM512_CLEAN_poly_invntt_tomont
#define MB_POLY_BASEMUL PQCLEAN_MLKEM512_CLEAN_poly_basemul_montgomery
#elif CONFIG_MLKEM_768
#define MB_POLY_NTT PQCLEAN_MLKEM768_CLEAN_poly_ntt
#define MB_POLY_INVNTT PQCLEAN_MLKEM768_CLEAN_poly_invntt_tomont
#define MB_POLY_BASEMUL PQCLEAN_MLKEM768_CLEAN_poly_basemul_montgomery
#elif CONFIG_MLKEM_1024
#define MB_POLY_NTT PQCLEAN_MLKEM1024_CLEAN_poly_ntt
#define MB_POLY_INVNTT PQCLEAN_MLKEM1024_CLEAN_poly_invntt_tomont
#define MB_POLY_BASEMUL PQCLEAN_MLKEM1024_CLEAN_poly_basemul_montgomery
#else
#error "Select an ML-KEM variant in menuconfig"
#endif

#if CONFIG_MLKEM_MICROBENCH
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

    static void fill_poly(poly *p) {
        for (size_t i = 0; i < KYBER_N; i++) {
            int16_t v = (int16_t)((i * 17u + 3u) % KYBER_Q);
            if (v > (KYBER_Q / 2)) {
                v = (int16_t)(v - KYBER_Q);
            }
            p->coeffs[i] = v;
        }
    }
#endif

void mlkem_microbench_run(energy_sampler_t *sampler) {
    #if !CONFIG_MLKEM_MICROBENCH
        ESP_LOGW(TAG, "Microbench disabled");
        return;
    #else
        ESP_LOGI(TAG, "Microbench enabled (%u iterations)", (unsigned)CONFIG_MLKEM_MICROBENCH_ITERS);

        poly base_a = {0};
        poly base_b = {0};
        poly a_ntt = {0};
        poly b_ntt = {0};
        poly work = {0};
        poly out = {0};

        fill_poly(&base_a);
        fill_poly(&base_b);

        a_ntt = base_a;
        b_ntt = base_b;
        MB_POLY_NTT(&a_ntt);
        MB_POLY_NTT(&b_ntt);

        for (uint32_t iter = 0; iter < CONFIG_MLKEM_MICROBENCH_ITERS; iter++) {
            energy_result_t energy = {0};

            memcpy(&work, &base_a, sizeof(work));
            energy_sampler_start(sampler);
            int64_t t0 = esp_timer_get_time();
            uint32_t c0 = esp_cpu_get_cycle_count();
            MB_POLY_NTT(&work);
            uint32_t c1 = esp_cpu_get_cycle_count();
            int64_t t1 = esp_timer_get_time();
            energy_sampler_stop(sampler, &energy);
            log_op(iter, "mb_ntt", mlkem_cycle_diff(c0, c1), (t1 - t0), energy.energy_uj);

            memcpy(&work, &a_ntt, sizeof(work));
            energy_sampler_start(sampler);
            t0 = esp_timer_get_time();
            c0 = esp_cpu_get_cycle_count();
            MB_POLY_INVNTT(&work);
            c1 = esp_cpu_get_cycle_count();
            t1 = esp_timer_get_time();
            energy_sampler_stop(sampler, &energy);
            log_op(iter, "mb_invntt", mlkem_cycle_diff(c0, c1), (t1 - t0), energy.energy_uj);

            energy_sampler_start(sampler);
            t0 = esp_timer_get_time();
            c0 = esp_cpu_get_cycle_count();
            MB_POLY_BASEMUL(&out, &a_ntt, &b_ntt);
            c1 = esp_cpu_get_cycle_count();
            t1 = esp_timer_get_time();
            energy_sampler_stop(sampler, &energy);
            log_op(iter, "mb_basemul", mlkem_cycle_diff(c0, c1), (t1 - t0), energy.energy_uj);
        }
    #endif
}
