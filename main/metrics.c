#include "metrics.h"

#include <stdio.h>

void metrics_log_header_s1(void) {
    printf("scenario,variant,iter,keygen_cycles,decaps_cycles,keygen_us,decaps_us,handshake_us,net_latency_us,jitter_us,energy_uj,reconnect_us,reconnect_energy_uj,rssi_dbm,packet_loss,ss_dec_hex,pub_bytes,ct_bytes,overhead_bytes,heap_free,heap_min,stack_hwm_bytes,cpu_util_percent\n");
}

void metrics_log_header_s2(void) {
    printf("scenario,variant,iter,op,cycles,time_us,energy_uj,heap_free,heap_min,stack_hwm_bytes\n");
}

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
                    float cpu_util_percent) {
    printf("s1,%s,%u,%u,%u,%lld,%lld,%lld,%lld,%lld,%llu,%lld,%llu,%d,%d,%s,%u,%u,%u,%u,%u,%u,%.2f\n",
           variant,
           (unsigned)iter,
           (unsigned)keygen_cycles,
           (unsigned)decaps_cycles,
           (long long)keygen_us,
           (long long)decaps_us,
           (long long)handshake_us,
           (long long)net_latency_us,
           (long long)jitter_us,
           (unsigned long long)energy_uj,
           (long long)reconnect_us,
           (unsigned long long)reconnect_energy_uj,
           (int)rssi_dbm,
           packet_loss,
        ss_dec_hex ? ss_dec_hex : "",
           (unsigned)pub_bytes,
           (unsigned)ct_bytes,
           (unsigned)overhead_bytes,
           (unsigned)heap_free,
           (unsigned)heap_min,
           (unsigned)stack_hwm_bytes,
           (double)cpu_util_percent);
}

void metrics_log_s2(uint32_t iter,
                    const char *variant,
                    const char *op,
                    uint32_t cycles,
                    int64_t time_us,
                    uint64_t energy_uj,
                    uint32_t heap_free,
                    uint32_t heap_min,
                    uint32_t stack_hwm_bytes) {
    printf("s2,%s,%u,%s,%u,%lld,%llu,%u,%u,%u\n",
           variant,
           (unsigned)iter,
           op,
           (unsigned)cycles,
           (long long)time_us,
           (unsigned long long)energy_uj,
           (unsigned)heap_free,
           (unsigned)heap_min,
           (unsigned)stack_hwm_bytes);
}
