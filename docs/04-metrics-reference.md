# Metrics Reference — Lineage and Interpretation

Dokumen ini menjelaskan: apa itu metrics, fungsi metrics, dihitung dari mana, function mana yang menghasilkan, dan konfigurasi apa yang memengaruhi hasil.

## 1) Apa itu Metrics di Project Ini?

Metrics adalah angka kuantitatif untuk menilai:

- performa komputasi kriptografi,
- biaya energi,
- kualitas jaringan (Scenario 1),
- tekanan resource runtime (heap/stack/CPU util).

Output utama metrics dicetak sebagai CSV lewat UART.

## 2) Sumber Utama Metrics di Kode

- `main/scenario_mqtt.c`: menghitung metrics Scenario 1.
- `main/scenario_local.c`: menghitung metrics Scenario 2 (end-to-end KEM lokal).
- `main/microbench.c`: metrics tambahan kernel-level untuk Scenario 2.
- `main/energy_sampler.c`: perhitungan energi (`energy_uj`) berbasis INA219.
- `main/metrics.c`: hanya formatting output CSV.

## 3) Lineage Metrics Scenario 1 (`s1`)

Header CSV (`main/metrics.c`):

```
scenario,variant,iter,keygen_cycles,decaps_cycles,keygen_us,decaps_us,handshake_us,net_latency_us,jitter_us,energy_uj,reconnect_us,reconnect_energy_uj,rssi_dbm,packet_loss,ss_dec_hex,pub_bytes,ct_bytes,overhead_bytes,heap_free,heap_min,stack_hwm_bytes,cpu_util_percent
```

### Field-by-field

- `scenario`: Konstanta string `s1` saat emit row.
- `variant`: `MLKEM_VARIANT_NAME` dari `main/mlkem_config.h`.
- `iter`: Counter loop `for (iter=0; iter<CONFIG_MLKEM_ITERATIONS; iter++)`.
- `keygen_cycles`: `esp_cpu_get_cycle_count()` sebelum/sesudah keygen, lalu `mlkem_cycle_diff`.
- `decaps_cycles`: Cycle delta untuk decaps.
- `keygen_us`: `esp_timer_get_time()` delta sekitar `mlkem_keypair`.
- `decaps_us`: timer delta sekitar `mlkem_decaps` (jika paket valid).
- `handshake_us`: window total dari start iterasi sampai akhir handshake processing.
- `net_latency_us`: `t_resp - t_pub` (response arrival minus publish time).
- `jitter_us`: absolut selisih latency iterasi saat ini terhadap iterasi sebelumnya.
- `energy_uj`: dari `energy_sampler_start/stop` (integrasi daya pada window handshake).
- `reconnect_us`: waktu reconnect MQTT per iterasi (µs); 0 untuk mode session.
- `reconnect_energy_uj`: energi reconnect per iterasi (µJ); 0 untuk mode session.
- `rssi_dbm`: dari `esp_wifi_sta_get_ap_info`.
- `packet_loss`: set `1` jika timeout, payload invalid, atau `iter_id` mismatch.
- `ss_dec_hex`: shared secret hasil `decaps` (hex). Kosong untuk iterasi gagal.
- `pub_bytes`: ukuran payload publish = `4 + MLKEM_PUBLICKEY_BYTES`.
- `ct_bytes`: ukuran payload ciphertext expected = `4 + MLKEM_CIPHERTEXT_BYTES`.
- `overhead_bytes`: estimasi overhead MQTT dari helper `mqtt_overhead_bytes(topic)`.
- `heap_free`: `esp_get_free_heap_size()`.
- `heap_min`: `esp_get_minimum_free_heap_size()`.
- `stack_hwm_bytes`: `uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t)`.
- `cpu_util_percent`: aproksimasi `(keygen_us + decaps_us) / handshake_us * 100`.

## 4) Lineage Metrics Scenario 2 (`s2`)

Header CSV (`main/metrics.c`):

```
scenario,variant,iter,op,cycles,time_us,energy_uj,heap_free,heap_min,stack_hwm_bytes
```

### Field-by-field

- `scenario`: konstanta `s2`.
- `variant`: `MLKEM_VARIANT_NAME`.
- `iter`: counter loop.
- `op`: `keygen`, `encaps`, `decaps` (atau `mb_ntt`, `mb_invntt`, `mb_basemul` jika microbench on).
- `cycles`: cycle delta dari `esp_cpu_get_cycle_count`.
- `time_us`: timer delta dari `esp_timer_get_time`.
- `energy_uj`: hasil energy sampler window per operasi.
- `heap_free`, `heap_min`, `stack_hwm_bytes`: snapshot resource saat logging.

## 5) Energy Metric Dihitung Bagaimana?

Di `main/energy_sampler.c`:

1. Task sampler membaca INA219 berkala (`bus_mv`, `cur_ma`).
2. Hitung daya: `power_w = V * I`.
3. Integrasi energi diskrit: `energy_uj += power_w * delta_us`.
4. Saat `energy_sampler_stop`, nilai kumulatif dipaketkan ke `energy_result_t`.

Artinya `energy_uj` adalah estimasi energi **fisik** selama interval operasi yang dibungkus `start/stop`. Ini berbeda dari estimasi berbasis clock cycles yang digunakan paper lain — INA219 mengukur arus nyata, termasuk overhead Wi-Fi radio (untuk Scenario 1).

Catatan penting:
- `energy_uj` untuk Scenario 1 mencakup energi window handshake (keygen + network wait + decaps), dan **tidak** mencakup reconnect.
	- Jika mode `reconn` aktif, energi reconnect dicatat terpisah pada `reconnect_energy_uj`.
- `energy_uj` untuk Scenario 2 diukur per operasi (keygen saja, atau encaps saja, atau decaps saja).
- CV energi (~1–9%) lebih tinggi dari CV cycles (~0.2–0.6%) karena fluktuasi sampling INA219.

## 6) Metrics untuk Skenario 1 dan 2 Dihitung Dari Mana?

### Scenario 1

- Compute metrics: dari call keygen/decaps di ESP.
- Network metrics: dari timestamp publish/receive MQTT.
- Resource metrics: dari API heap/stack dan RSSI.

### Scenario 2

- Murni compute + energy + resource lokal.
- Tidak ada latency/jitter jaringan.

## 7) Fungsi Opsi Konfigurasi terhadap Metrics

Konfigurasi yang paling memengaruhi hasil:

- `ML-KEM variant`: Ukuran parameter set berubah -> cycles/time/energy berubah.
- `MLKEM_PIE_OPT` + `MLKEM_PIE_AGGRESSIVE`: Mengubah implementasi kernel -> langsung mengubah performa.
- `SCENARIO_1` vs `SCENARIO_2`: Menentukan keluarga metrics yang keluar.
- `MLKEM_ITERATIONS`: Mempengaruhi kestabilan statistik distribusi.
- `MLKEM_MICROBENCH` + `MLKEM_MICROBENCH_ITERS`: Menambah metrics granular kernel-level.
- `MQTT_*` + `WIFI_*`: Mempengaruhi `net_latency_us`, `jitter_us`, `packet_loss`.
- `MQTT_RECONNECT_EACH_ITER`: Mengaktifkan reconnect per iterasi dan menghasilkan metrik terpisah `reconnect_us` dan `reconnect_energy_uj`.
	- Window `handshake_us`/`energy_uj` tidak mencakup reconnect.
	- Untuk biaya total mode `reconn`, hitung offline: `total_latency_us = handshake_us + reconnect_us`, `total_energy_uj = energy_uj + reconnect_energy_uj`.
- `INA219_*`: Mempengaruhi noise/akurasi estimasi energi.
- `MLKEM_UART_BAUDRATE`: Tidak mengubah performa kripto langsung, tapi mempengaruhi reliability output telemetry saat throughput log tinggi.

Catatan metodologi Scenario 1: Broker MQTT disarankan mengaktifkan `TCP_NODELAY` (lihat `tools/fedora_mosquitto.conf`) untuk menghindari artefak Nagle/Delayed ACK pada payload kecil yang dapat membesar-besarkan `net_latency_us`.

## 8) Menafsirkan Metrics dengan Benar

- Gunakan mean ± σ sebagai baseline; CV (coefficient of variation = σ/mean × 100%) sebagai indikator kestabilan.
- Gunakan p50 sebagai baseline stabil dan p95 untuk melihat tail latency/cost.
- Bandingkan hanya antar-run dengan konfigurasi identik selain variabel yang diuji.
- Untuk Scenario 1, analisis `packet_loss` terpisah dari latency agar tidak bias.
- Untuk PIE study, gunakan pasangan run `pieoff` vs `pieon` dengan varian sama.
- Untuk reconnect study, gunakan pasangan run `reconn` vs `session` dengan varian + PIE sama.
- Energy CV yang lebih tinggi (~9%) wajar untuk metrik INA219; gunakan n=100 untuk CI yang ketat.

## 9) Shared Secret Correctness vs Metrics

- Scenario 2 punya self-test eksplisit (`memcmp(ss_enc, ss_dec)`).
- Scenario 1 tidak melakukan perbandingan shared secret secara real-time di firmware.
	- Namun dataset menyediakan `ss_dec_hex` (ESP) dan `ss_enc_hex` (subscriber). Setelah di-join, kolom `ss_valid` dapat dipakai untuk verifikasi correctness secara offline.

## 10) Subscriber CSV Columns (Scenario 1)

File `data/csv_subscriber/` berisi output dari `tools/fedora_subscriber.py`:

```
iter,variant,encaps_us,ss_enc_hex
```

- `iter`: nomor iterasi yang diterima dari payload pubkey.
- `variant`: varian ML-KEM dari argumen subscriber.
- `encaps_us`: waktu eksekusi `encaps` di sisi subscriber (µs).
- `ss_enc_hex`: shared secret hasil `encaps` (hex).

Setelah di-join dengan `join_subscriber_csv.py`, kolom `encaps_us`, `ss_enc_hex`, dan `ss_valid` ditambahkan ke row Scenario 1 yang bersesuaian.
`ss_valid=1` berarti `ss_dec_hex` (ESP) cocok dengan `ss_enc_hex` (subscriber).

---

Referensi operasional analisis ada di `docs/03-operational-runbook.md`.
Panduan analisis end-to-end (tools + workflows) ada di `docs/08-analysis-tools-and-workflows.md`.
