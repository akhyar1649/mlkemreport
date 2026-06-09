#ifndef METRICS_H
#define METRICS_H

#include <stdint.h>

void metrics_log_header_s1(void);
void metrics_log_header_s2(void);

void metrics_log_s1(uint32_t iter,
                    const char *variant,
                    uint32_t keygen_cycles,
                    uint32_t decaps_cycles,
                    int64_t keygen_us,
                    int64_t decaps_us,
                    int64_t handshake_us,
                    int64_t net_latency_us,
                    int64_t jitter_us,
                    uint64_t energy_uj,
                    int64_t reconnect_us,
                    uint64_t reconnect_energy_uj,
                    int8_t rssi_dbm,
                    int packet_loss,
                    const char *ss_dec_hex,
                    uint32_t pub_bytes,
                    uint32_t ct_bytes,
                    uint32_t overhead_bytes,
                    uint32_t heap_free,
                    uint32_t heap_min,
                    uint32_t stack_hwm_bytes,
                    float cpu_util_percent);

void metrics_log_s2(uint32_t iter,
                    const char *variant,
                    const char *op,
                    uint32_t cycles,
                    int64_t time_us,
                    uint64_t energy_uj,
                    uint32_t heap_free,
                    uint32_t heap_min,
                    uint32_t stack_hwm_bytes);

#endif
