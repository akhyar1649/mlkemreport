# ML-KEM (FIPS 203) on ESP32-S3

This project runs ML-KEM (PQClean, clean C) on ESP32-S3 using ESP-IDF v5.4 with two scenarios:

- Scenario 1: MQTT v5.0 handshake (KeyGen + Decaps on ESP32, Encaps on subscriber)
- Scenario 2: Local compute benchmark (KeyGen/Encaps/Decaps all on ESP32)

Raw metrics are printed as CSV over UART.

## Experiment Matrix

Total **18 run** (6 Scenario 2 + 12 Scenario 1) — ✅ done:

| Scenario | Variant | PIE | Reconnect Mode | Status |
|----------|---------|-----|----------------|--------|
| S2 (local) | 512 | off | — | ✅ done |
| S2 (local) | 512 | on  | — | ✅ done |
| S2 (local) | 768 | off | — | ✅ done |
| S2 (local) | 768 | on  | — | ✅ done |
| S2 (local) | 1024 | off | — | ✅ done |
| S2 (local) | 1024 | on  | — | ✅ done |
| S1 (MQTT) | 512 | off | reconn (auth per-iter) | ✅ done |
| S1 (MQTT) | 512 | off | session (session-reuse) | ✅ done |
| S1 (MQTT) | 512 | on  | reconn | ✅ done |
| S1 (MQTT) | 512 | on  | session | ✅ done |
| S1 (MQTT) | 768 | off | reconn | ✅ done |
| S1 (MQTT) | 768 | off | session | ✅ done |
| S1 (MQTT) | 768 | on  | reconn | ✅ done |
| S1 (MQTT) | 768 | on  | session | ✅ done |
| S1 (MQTT) | 1024 | off | reconn | ✅ done |
| S1 (MQTT) | 1024 | off | session | ✅ done |
| S1 (MQTT) | 1024 | on  | reconn | ✅ done |
| S1 (MQTT) | 1024 | on  | session | ✅ done |

Setiap run S1 menghasilkan dua file CSV: satu dari ESP (`data/csv/`) dan satu dari subscriber (`data/csv_subscriber/`).
Detail checklist ada di `docs/06-experiment-plan.md`.

## Hardware wiring

### Wiring Scenario 1 (Wi-Fi aktif) — Gunakan Adaptor Terpisah

Wi-Fi ESP32-S3 membutuhkan peak current 300–500 mA. CP2102 tidak cukup kuat untuk ini dan dapat menyebabkan brownout. **Untuk Scenario 1, gunakan adaptor USB 5V ≥1A sebagai sumber daya, dan CP2102 hanya untuk UART.**

Power path (Scenario 1):

```
Adaptor USB 5V ≥1A -> INA219 VIN+
INA219 VIN-        -> ESP32 5V (VBUS)
GND (adaptor, INA219, ESP32) semua terhubung
```

INA219 logic (I2C) — sama untuk semua skenario:

```
ESP32 3V3  -> INA219 VCC
ESP32 GPIO8 (SDA) -> INA219 SDA
ESP32 GPIO9 (SCL) -> INA219 SCL
ESP32 GND  -> INA219 GND
```

CP2102 — hanya UART untuk Scenario 1 (5V CP2102 tidak disambungkan ke ESP32):

```
CP2102 TXD -> ESP32 RX0 (GPIO44 on most ESP32-S3 devkits)
CP2102 RXD -> ESP32 TX0 (GPIO43 on most ESP32-S3 devkits)
CP2102 GND -> ESP32 GND
Optional auto-reset:
  CP2102 DTR -> ESP32 IO0 (BOOT)
  CP2102 RTS -> ESP32 EN (RESET)
If only DTR:
  CP2102 DTR -> ESP32 IO0 (BOOT)
  RTS not connected (use manual RESET during flashing)
```

### Wiring Scenario 2 (local compute) — Kebijakan daya sama seperti Scenario 1

Untuk konsistensi pengukuran energi dan menghindari brownout, gunakan adaptor USB 5V ≥1A sebagai sumber daya melalui INA219, dan gunakan CP2102 hanya untuk UART (TX/RX/GND). Jalur daya sama seperti Scenario 1; 5V dari CP2102 tidak disambungkan ke ESP32.

Notes (berlaku semua skenario):

- Gunakan hanya satu sumber daya. Jangan biarkan USB-C dan adaptor terhubung bersamaan.
- Hindari powering dari USB-C langsung ke ESP32 saat mengukur energi (USB-C melewati INA219).
- Jika DTR terhubung tapi RTS tidak: tekan RESET sekali saat flashing dimulai.
- Jika tidak ada DTR/RTS: tahan BOOT, tekan RESET, lepas BOOT saat flashing mulai.

## Build and flash (step by step)

1. Set target:
   - `idf.py set-target esp32s3`
2. Configure:
   - `idf.py menuconfig`
     - `ML-KEM App` menu
       - Select scenario and ML-KEM variant
       - Set Wi-Fi SSID/PASSWORD (Scenario 1 only)
       - Set MQTT broker URI and credentials (Scenario 1 only)
       - Set INA219 settings (I2C pins and sample period)
3. Build:
   - `idf.py build`
4. Flash:
   - `idf.py -p /dev/ttyUSB0 flash`
   - DTR only: press RESET once when flashing starts (after the command is running).
   - No DTR/RTS: hold BOOT, tap RESET, then release BOOT when flashing starts.
5. Monitor:
   - `idf.py -p /dev/ttyUSB0 -b 921600 monitor`

## Configuration quick reference (menuconfig)

All settings are in `ML-KEM App`:

- Scenario selection: `Scenario`
- Variant selection: `ML-KEM variant`
- Iterations: `Iterations per run`
- UART baudrate: `CSV UART baudrate`
- Wi-Fi: `Wi-Fi SSID`, `Wi-Fi password`
- MQTT: `MQTT broker URI`, `MQTT username`, `MQTT password`, `MQTT QoS`
- Topics: `MQTT topic (pubkey)`, `MQTT topic (ciphertext)`
- Auth per-iteration: `Reconnect MQTT each iteration`
- INA219: I2C port/pins, address, shunt value, ADC mode, sample period, I2C frequency
- FreeRTOS tick rate (for energy resolution): `Component config -> FreeRTOS -> Tick rate (Hz)`

## Scenario selection

### Scenario 1 (MQTT handshake)

1. Choose `Scenario 1 (MQTT handshake)` in menuconfig.
2. Set broker URI, username, password, topics, and QoS.
3. Set `Reconnect MQTT each iteration`:
   - `y` = auth per iteration (`reconn` mode) — reconnect ke broker setiap iterasi.
   - `n` = session reuse (`session` mode) — satu sesi MQTT untuk semua iterasi.

### Scenario 2 (local compute)

1. Choose `Scenario 2 (local compute)` in menuconfig.
2. No MQTT required.

## Scenario 2 metrics (CSV columns)

The Scenario 2 CSV header is:

```
scenario,variant,iter,op,cycles,time_us,energy_uj,heap_free,heap_min,stack_hwm_bytes
```

Meaning of each column and how it is measured:

- `scenario`: Always `s2` for Scenario 2 output.
- `variant`: ML-KEM variant string (e.g., `ML-KEM-512`).
- `iter`: Iteration index (0 to `CONFIG_MLKEM_ITERATIONS - 1`).
- `op`: Operation name, one of `keygen`, `encaps`, `decaps`.
- `cycles`: CPU cycle count for the operation; measured by `esp_cpu_get_cycle_count()` before and after the call.
- `time_us`: Elapsed time in microseconds for the operation; measured by `esp_timer_get_time()` before and after.
- `energy_uj`: Energy in microjoules for the operation window; computed by sampling INA219 power and integrating over time between `energy_sampler_start()` and `energy_sampler_stop()`.
- `heap_free`: Current free heap (bytes) at log time; from `esp_get_free_heap_size()`.
- `heap_min`: Minimum free heap (bytes) seen since boot; from `esp_get_minimum_free_heap_size()`.
- `stack_hwm_bytes`: Stack high-water mark (bytes) for the current task; from `uxTaskGetStackHighWaterMark()`.

## UART CSV output

UART baudrate is configured in menuconfig under `ML-KEM App -> CSV UART baudrate`.
The CSV headers are printed at the start of each run. Scenario 1 rows start with `s1`,
Scenario 2 rows start with `s2`.

## Mosquitto (Fedora)

Example config: see [tools/fedora_mosquitto.conf](tools/fedora_mosquitto.conf).
Create password file:

```
mosquitto_passwd -c /etc/mosquitto/pwfile <username>
```

Konfigurasi contoh proyek sudah menyetel `set_tcp_nodelay true` untuk menonaktifkan Nagle/Delayed ACK sehingga latency kecil tidak terdistorsi. Gunakan file contoh ini apa adanya untuk Scenario 1.

## Subscriber

Script utama: **[tools/fedora_subscriber.py](tools/fedora_subscriber.py)** — dijalankan di Laptop B (atau host yang sama) saat Scenario 1. Subscribes ke topic pubkey, lakukan Encapsulation, publish ciphertext balik, dan simpan timing encaps ke `data/csv_subscriber/`.

Script alternatif: [tools/windows_subscriber.py](tools/windows_subscriber.py) — untuk host Windows, fungsionalitas inti sama.

Untuk single-machine testing (broker + subscriber di satu laptop): [tools/temporary/fedora_subscriber_local.py](tools/temporary/fedora_subscriber_local.py).

Recommended dependencies:

```
pip install paho-mqtt pqcrypto
```

Jika `pqcrypto` mengekspos nama Kyber bukan ML-KEM, script otomatis fallback ke `kyber512/768/1024`.

## Capture raw CSV data (step by step)

Output folders (created automatically by the script):

- `data/raw/` : full UART log (sanitized)
- `data/csv/` : filtered CSV rows only

File naming pattern:

```
Scenario 2:
  data/raw/s2_<variant>_<pieon|pieoff>_YYYYMMDD_HHMMSS.log
  data/csv/s2_<variant>_<pieon|pieoff>_YYYYMMDD_HHMMSS.csv

Scenario 1:
  data/raw/s1_<variant>_<pieon|pieoff>_<reconn|session>_YYYYMMDD_HHMMSS.log
  data/csv/s1_<variant>_<pieon|pieoff>_<reconn|session>_YYYYMMDD_HHMMSS.csv
   data/csv_subscriber/s1_<variant>_<pieon|pieoff>_<reconn|session>_YYYYMMDD_HHMMSS.csv
   data/csv_joined/s1_<variant>_<pieon|pieoff>_<reconn|session>_YYYYMMDD_HHMMSS_joined.csv
```

Recommended capture script:

```
Scenario 2: bash tools/capture_uart.sh /dev/ttyUSB0 s2 512 pieoff
Scenario 1: bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff reconn
```

### Scenario 1 (MQTT handshake)

Setup jaringan: ESP32-S3 dan Laptop B (host subscriber) harus berada di jaringan Wi-Fi yang sama. Jika menggunakan OpenWrt router, ikuti panduan lengkap di [docs/11-openwrt-router-setup.md](docs/11-openwrt-router-setup.md). Pastikan setidaknya:
- Port 1883 tidak diblokir firewall router untuk traffic LAN.
- Broker berjalan di Laptop A/B dengan IP statis (atau DHCP dengan static lease).

1. Start broker (Fedora example):
   - `sudo mosquitto -c tools/fedora_mosquitto.conf`
2. Start subscriber di host subscriber:
   - `python3 tools/fedora_subscriber.py --broker <broker-ip> --port 1883 --username <user> --password <pass> --variant 512 --pie pieoff`
3. Build dan flash ESP32-S3 dengan konfigurasi yang sesuai (scenario 1, variant, PIE, dan reconnect mode).
4. Capture UART CSV (sertakan reconnect tag):
   - Mode auth per-iter: `bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff reconn`
   - Mode session-reuse: `bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff session`

Notes:

- Subscriber mencetak `iter,variant,encaps_us,ss_enc_hex` dan menyimpan ke `data/csv_subscriber/` (gunakan `--csv-out` untuk path custom).
- Pastikan `Reconnect MQTT each iteration` di menuconfig sesuai dengan tag yang digunakan (`y` untuk `reconn`, `n` untuk `session`).

### Scenario 2 (local compute)

1. Build and flash the ESP32-S3 (see steps above).
2. Capture UART CSV:
   - `bash tools/capture_uart.sh /dev/ttyUSB0 s2 512 pieoff`

If you see task watchdog warnings, increase `INA219 sample period (ms)` in menuconfig (for example 10 ms).
For better energy resolution, set FreeRTOS tick rate to 1000 Hz in menuconfig.

## Analysis & QERS workflow

- Panduan end-to-end preprocessing, perintah terminal, dan QERS (termasuk what-if bobot dan orientasi K/Co): lihat `docs/08-analysis-tools-and-workflows.md`.
- Metodologi pembobotan AHP (Entropy × Prior) dan justifikasi akademik: lihat `docs/09-ahp-weighting-methodology.md`.
- Default implementasi QERS saat ini: K dan Co diperlakukan sebagai COST; normalisasi dilakukan per-mode (session/reconn) untuk fairness.

## Notes

- In `session` mode, Scenario 1 connects once and performs all handshake iterations on the same MQTT session.
- In `reconn` mode, Scenario 1 reconnects before each iteration; reconnect time/energy are recorded separately (`reconnect_us`, `reconnect_energy_uj`) and excluded from the handshake window (`handshake_us`, `energy_uj`).
- Energy per iteration is integrated by sampling INA219 during the measured window.
- CPU utilization for scenario 1 is approximated as (keygen_us + decaps_us) / handshake_us.
