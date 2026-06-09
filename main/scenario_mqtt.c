#include "scenario_mqtt.h"

#include "esp_cpu.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "metrics.h"
#include "mlkem_config.h"
#include "mlkem_util.h"
#include "mqtt_client.h"
#include "wifi_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include <stdlib.h>
#include <string.h>

#define MQTT_RESP_BIT    BIT0
#define MQTT_CONN_BIT    BIT1

static const char *TAG = "scenario_mqtt";

static EventGroupHandle_t s_mqtt_event_group;
static esp_mqtt_client_handle_t s_client;

static uint8_t s_rx_buf[MLKEM_CIPHERTEXT_BYTES + 4];
static size_t s_rx_total;
static size_t s_rx_expected;
static int64_t s_resp_time_us;
static bool s_receiving_ct;

static bool topic_match(const char *topic, int topic_len, const char *expected) {
    size_t len = strlen(expected);
    if ((size_t)topic_len != len) {
        return false;
    }
    return memcmp(topic, expected, len) == 0;
}

static void bytes_to_hex(char *out, size_t out_len, const uint8_t *in, size_t in_len) {
    static const char kHex[] = "0123456789abcdef";

    if (out_len < (in_len * 2 + 1)) {
        if (out_len > 0) {
            out[0] = '\0';
        }
        return;
    }

    for (size_t i = 0; i < in_len; i++) {
        uint8_t v = in[i];
        out[2 * i] = kHex[v >> 4];
        out[2 * i + 1] = kHex[v & 0x0F];
    }
    out[2 * in_len] = '\0';
}

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    if (event_id == MQTT_EVENT_CONNECTED) {
        xEventGroupSetBits(s_mqtt_event_group, MQTT_CONN_BIT);
        esp_mqtt_client_subscribe(s_client, CONFIG_MQTT_TOPIC_CIPHERTEXT, CONFIG_MQTT_QOS);
        return;
    }

    if (event_id == MQTT_EVENT_DATA) {
        if (event->current_data_offset == 0) {
            s_receiving_ct = topic_match(event->topic, event->topic_len,
                                         CONFIG_MQTT_TOPIC_CIPHERTEXT);
            if (s_receiving_ct) {
                s_rx_total = 0;
                s_rx_expected = event->total_data_len;
            }
        }
        if (!s_receiving_ct) {
            return;
        }

        if (s_rx_expected > sizeof(s_rx_buf)) {
            return;
        }

        if (event->data_len > 0) {
            memcpy(s_rx_buf + event->current_data_offset, event->data, event->data_len);
            s_rx_total += event->data_len;
        }

        if (s_rx_total >= s_rx_expected) {
            s_resp_time_us = esp_timer_get_time();
            xEventGroupSetBits(s_mqtt_event_group, MQTT_RESP_BIT);
            s_receiving_ct = false;
        }
    }
}

static uint32_t mqtt_overhead_bytes(const char *topic) {
    uint32_t topic_len = (uint32_t)strlen(topic);
    uint32_t fixed_header = 2;
    uint32_t topic_field = 2 + topic_len;
    uint32_t packet_id = 2;
    uint32_t properties = 1;
    return fixed_header + topic_field + packet_id + properties;
}

static void mqtt_wait_connected(void) {
    xEventGroupWaitBits(s_mqtt_event_group, MQTT_CONN_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

#if CONFIG_MQTT_RECONNECT_EACH_ITER
static void mqtt_reconnect_for_iteration(void) {
    esp_mqtt_client_disconnect(s_client);
    xEventGroupClearBits(s_mqtt_event_group, MQTT_CONN_BIT);
    esp_mqtt_client_reconnect(s_client);
    mqtt_wait_connected();
}
#endif

void scenario_mqtt_run(energy_sampler_t *sampler) {
    ESP_LOGI(TAG, "Scenario 1: MQTT handshake (%s)", MLKEM_VARIANT_NAME);

    ESP_ERROR_CHECK(wifi_manager_init_and_connect());

    s_mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_BROKER_URI,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
        .session.keepalive = 60,
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);

    mqtt_wait_connected();

    metrics_log_header_s1();

    uint8_t *pk = heap_caps_malloc(MLKEM_PUBLICKEY_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *sk = heap_caps_malloc(MLKEM_SECRETKEY_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *ct = heap_caps_malloc(MLKEM_CIPHERTEXT_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *ss_dec = heap_caps_malloc(MLKEM_SHAREDSECRET_BYTES, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint8_t *payload_pub = heap_caps_malloc(MLKEM_PUBLICKEY_BYTES + 4, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (!pk || !sk || !ct || !ss_dec || !payload_pub) {
        ESP_LOGE(TAG, "Heap allocation failed for ML-KEM buffers");
        goto cleanup;
    }

    int64_t prev_latency_us = 0;

    for (uint32_t iter = 0; iter < CONFIG_MLKEM_ITERATIONS; iter++) {
        energy_result_t energy = {0};
        energy_result_t reconnect_energy = {0};
        int64_t reconnect_us = 0;
        int packet_loss = 0;
        char ss_dec_hex[MLKEM_SHAREDSECRET_BYTES * 2 + 1];

        ss_dec_hex[0] = '\0';

#if CONFIG_MQTT_RECONNECT_EACH_ITER
        energy_sampler_start(sampler);
        int64_t t_reconn_start = esp_timer_get_time();
        mqtt_reconnect_for_iteration();
        reconnect_us = esp_timer_get_time() - t_reconn_start;
        energy_sampler_stop(sampler, &reconnect_energy);
        vTaskDelay(pdMS_TO_TICKS(50));
#endif

        energy_sampler_start(sampler);
        int64_t t_handshake_start = esp_timer_get_time();

        int64_t t0 = esp_timer_get_time();
        uint32_t c0 = esp_cpu_get_cycle_count();
        int ret = mlkem_keypair(pk, sk);
        uint32_t c1 = esp_cpu_get_cycle_count();
        int64_t t1 = esp_timer_get_time();
        int64_t keygen_us = t1 - t0;
        uint32_t keygen_cycles = mlkem_cycle_diff(c0, c1);
        if (ret != 0) {
            ESP_LOGE(TAG, "KeyGen failed (iter %u)", (unsigned)iter);
        }

        payload_pub[0] = (uint8_t)(iter & 0xFF);
        payload_pub[1] = (uint8_t)((iter >> 8) & 0xFF);
        payload_pub[2] = (uint8_t)((iter >> 16) & 0xFF);
        payload_pub[3] = (uint8_t)((iter >> 24) & 0xFF);
        memcpy(payload_pub + 4, pk, MLKEM_PUBLICKEY_BYTES);

        int64_t t_pub = esp_timer_get_time();
        int msg_id = esp_mqtt_client_publish(s_client,
                                             CONFIG_MQTT_TOPIC_PUBKEY,
                                             (const char *)payload_pub,
                                             MLKEM_PUBLICKEY_BYTES + 4,
                                             CONFIG_MQTT_QOS,
                                             0);
        if (msg_id < 0) {
            ESP_LOGE(TAG, "Publish failed (iter %u)", (unsigned)iter);
        }

        EventBits_t bits = xEventGroupWaitBits(s_mqtt_event_group,
                                               MQTT_RESP_BIT,
                                               pdTRUE,
                                               pdFALSE,
                                               pdMS_TO_TICKS(CONFIG_MQTT_HANDSHAKE_TIMEOUT_MS));

        int64_t t_resp = 0;
        int64_t net_latency_us = -1;
        int64_t jitter_us = -1;
        uint32_t decaps_cycles = 0;
        int64_t decaps_us = -1;

        if (!(bits & MQTT_RESP_BIT)) {
            packet_loss = 1;
        } else {
            t_resp = s_resp_time_us;
            net_latency_us = t_resp - t_pub;
            if (prev_latency_us > 0) {
                int64_t diff = net_latency_us - prev_latency_us;
                jitter_us = diff < 0 ? -diff : diff;
            } else {
                jitter_us = 0;
            }
            prev_latency_us = net_latency_us;

            if (s_rx_expected >= 4 + MLKEM_CIPHERTEXT_BYTES) {
                uint32_t rx_iter = (uint32_t)s_rx_buf[0] |
                                   ((uint32_t)s_rx_buf[1] << 8) |
                                   ((uint32_t)s_rx_buf[2] << 16) |
                                   ((uint32_t)s_rx_buf[3] << 24);
                if (rx_iter != iter) {
                    packet_loss = 1;
                } else {
                    memcpy(ct, s_rx_buf + 4, MLKEM_CIPHERTEXT_BYTES);
                }
            } else {
                packet_loss = 1;
            }

            if (!packet_loss) {
                t0 = esp_timer_get_time();
                c0 = esp_cpu_get_cycle_count();
                ret = mlkem_decaps(ss_dec, ct, sk);
                c1 = esp_cpu_get_cycle_count();
                t1 = esp_timer_get_time();
                decaps_us = t1 - t0;
                decaps_cycles = mlkem_cycle_diff(c0, c1);
                if (ret != 0) {
                    ESP_LOGE(TAG, "Decaps failed (iter %u)", (unsigned)iter);
                } else {
                    bytes_to_hex(ss_dec_hex, sizeof(ss_dec_hex), ss_dec, MLKEM_SHAREDSECRET_BYTES);
                }
            }
        }

        int64_t t_handshake_end = esp_timer_get_time();
        energy_sampler_stop(sampler, &energy);

        int64_t handshake_us = t_handshake_end - t_handshake_start;
        float cpu_util = 0.0f;
        if (handshake_us > 0 && decaps_us > 0) {
            cpu_util = ((float)(keygen_us + decaps_us) / (float)handshake_us) * 100.0f;
        }

        int8_t rssi_dbm = 0;
        wifi_ap_record_t ap_info = {0};
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            rssi_dbm = ap_info.rssi;
        }

        uint32_t pub_bytes = MLKEM_PUBLICKEY_BYTES + 4;
        uint32_t ct_bytes = MLKEM_CIPHERTEXT_BYTES + 4;
        uint32_t overhead_bytes = mqtt_overhead_bytes(CONFIG_MQTT_TOPIC_PUBKEY) +
                                  mqtt_overhead_bytes(CONFIG_MQTT_TOPIC_CIPHERTEXT);

        uint32_t heap_free = esp_get_free_heap_size();
        uint32_t heap_min = esp_get_minimum_free_heap_size();
        uint32_t stack_hwm_bytes = (uint32_t)uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t);

        metrics_log_s1(iter,
                       MLKEM_VARIANT_NAME,
                       keygen_cycles,
                       decaps_cycles,
                       keygen_us,
                       decaps_us,
                       handshake_us,
                       net_latency_us,
                       jitter_us,
                       energy.energy_uj,
                       reconnect_us,
                       reconnect_energy.energy_uj,
                       rssi_dbm,
                       packet_loss,
                       ss_dec_hex,
                       pub_bytes,
                       ct_bytes,
                       overhead_bytes,
                       heap_free,
                       heap_min,
                       stack_hwm_bytes,
                       cpu_util);

        mlkem_secure_zero(pk, MLKEM_PUBLICKEY_BYTES);
        mlkem_secure_zero(sk, MLKEM_SECRETKEY_BYTES);
        mlkem_secure_zero(ct, MLKEM_CIPHERTEXT_BYTES);
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
    if (ss_dec) {
        mlkem_secure_zero(ss_dec, MLKEM_SHAREDSECRET_BYTES);
        free(ss_dec);
    }
    if (payload_pub) {
        mlkem_secure_zero(payload_pub, MLKEM_PUBLICKEY_BYTES + 4);
        free(payload_pub);
    }
}
