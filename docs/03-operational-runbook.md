# Operational Runbook — Scenarios & Tools

Dokumen ini adalah panduan operasional lengkap: langkah demi langkah menjalankan Scenario 1 dan Scenario 2, setup infrastruktur, dan cara menggunakan semua tools.

## 1) Prasyarat

- Board: `ESP32-S3-DevKitC-1 N16R8`
- ESP-IDF v5.4 environment siap (`idf.py` tersedia di PATH).
- Koneksi serial CP2102 ke Linux host.
- Untuk Scenario 1: jaringan Wi-Fi LAN, broker MQTT + subscriber aktif sebelum flash.

## 2) Step-by-Step Umum (Semua Skenario)

### Step 1 — Set target chip

```bash
idf.py set-target esp32s3
```

Kenapa penting: build output dan opsi hardware-specific disesuaikan untuk ESP32-S3.

### Step 2 — Konfigurasi

```bash
idf.py menuconfig
```

Atur minimal:
- `ML-KEM variant`
- `Scenario`
- `Enable PIE kernels`
- `Iterations per run`
- `CSV UART baudrate`

Jika Scenario 1:
- Set `WIFI_*` dan `MQTT_*` lengkap.
- Set `Reconnect MQTT each iteration` sesuai mode yang ingin diukur (`y`=reconn, `n`=session).

### Step 3 — Build

```bash
idf.py build
```

Hasil build akhirnya apa?
- Folder `build/` berisi artefak firmware image.
- Saat flash, yang ditulis ke ESP adalah image binary yang dihasilkan dari proses ini (bootloader, partition table, app image), bukan source code langsung.

### Step 4 — Tentukan `/dev/ttyUSBx`

Cara cepat:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Cara lebih yakin:

1. Cabut board, cek list port.
2. Colok board, cek list lagi.
3. Port baru itulah device board kamu.

### Step 5 — Flash ke board

```bash
idf.py -p /dev/ttyUSB0 flash
```

Ganti `/dev/ttyUSB0` sesuai port kamu.

### Step 6 — Monitor UART

```bash
idf.py -p /dev/ttyUSB0 -b 921600 monitor
```

Baud sesuaikan dengan `ML-KEM App -> CSV UART baudrate`.

## 3) Kenapa Harus Tekan BOOT/RESET Saat Upload?

ESP32 perlu masuk mode bootloader serial agar flash tool bisa upload firmware.

- Jika auto-reset wiring lengkap (DTR+RTS): proses biasanya otomatis.
- Jika tidak lengkap:
  - Tekan dan tahan `BOOT`, lalu tekan `RESET`, lalu lepas `BOOT` saat flashing mulai.
- Jika hanya sebagian wiring:
  - Kadang cukup tekan `RESET` sekali saat upload dimulai.

Inti penyebab: board belum otomatis masuk state bootloader tanpa bantuan sinyal hardware.

## 4) Kenapa Sering Reset Sebelum Capture UART?

Karena kamu butuh log run yang dimulai dari awal eksekusi (`app_main`), termasuk header CSV.

Jika tidak reset:
- Bisa masuk tengah loop lama.
- Header CSV bisa tidak ikut tertangkap.
- Dataset menjadi tidak sinkron.

Jadi reset dipakai untuk membuat start point pengukuran deterministik.

## 5) UART Itu Apa?

UART (Universal Asynchronous Receiver/Transmitter) adalah antarmuka serial sederhana untuk kirim data byte-per-byte antara ESP dan host. Di project ini UART dipakai sebagai kanal telemetry:

- Firmware mencetak CSV metrics via `printf`.
- Host menangkap stream ini jadi file `.log` dan `.csv`.

---

## 6) Alur Lengkap Scenario 1 (MQTT Handshake)

### Tujuan

Meniru handshake terdistribusi:
- ESP melakukan `keygen` + `decaps`.
- Subscriber host melakukan `encaps`.

### Data Flow

1. ESP keygen -> hasil `pk/sk`.
2. ESP publish payload `iter_id + pk` ke topic pubkey.
3. Subscriber menerima payload, jalankan encaps -> hasil `ct` (+ shared secret subscriber).
4. Subscriber publish `iter_id + ct` ke topic ciphertext.
5. ESP menerima ct, validasi `iter_id`, jalankan decaps -> shared secret ESP.
6. ESP log metrik `s1` ke UART CSV.

### Kenapa skenario ini penting?

Karena melibatkan komponen jaringan nyata: latency, jitter, packet loss, overhead MQTT. Sesuai dengan kondisi deployment IoT nyata di mana ESP32 berkomunikasi ke broker cloud/on-premise via Wi-Fi.

### Setup Jaringan (LAN)

ESP32-S3 dan host subscriber harus berada di jaringan Wi-Fi yang sama. Untuk setup dengan OpenWrt router, ikuti panduan lengkap di [11-openwrt-router-setup.md](11-openwrt-router-setup.md). Ringkasan minimum:

- Pastikan port 1883 tidak diblokir firewall untuk traffic intra-LAN.
- Gunakan IP statis atau DHCP static lease untuk host broker agar URI tetap konsisten antar run.
- Broker dan subscriber bisa di laptop yang sama (single-machine) atau laptop berbeda (Laptop A = broker, Laptop B = subscriber), asal di jaringan yang sama dengan ESP32.

### Dua Mode Koneksi: `reconn` vs `session`

Scenario 1 mendukung dua mode yang masing-masing merepresentasikan skenario nyata berbeda:

| Mode | Config | Representasi nyata | File tag |
|------|--------|--------------------|----------|
| **reconn** | `MQTT_RECONNECT_EACH_ITER=y` | Sensor baterai yang tidur antar pengukuran, reconnect tiap sesi | `reconn` |
| **session** | `MQTT_RECONNECT_EACH_ITER=n` | Sensor industri yang online terus, satu sesi MQTT persisten | `session` |

Keduanya harus dijalankan untuk setiap kombinasi (varian × PIE). Total 12 run Scenario 1.

---

## 7) Alur Lengkap Scenario 2 (Local Compute)

### Tujuan

Mengisolasi performa komputasi KEM tanpa variabel jaringan.

### Data Flow

1. ESP keygen lokal.
2. ESP encaps lokal.
3. ESP decaps lokal.
4. Log `s2` (`keygen`, `encaps`, `decaps`) ke UART CSV.

Kenapa bisa dalam 1 mesin? Karena semua input/output kripto tersedia di memori internal ESP, tidak perlu round-trip network.

---

## 8) Verifikasi Shared Secret: Scenario 1 vs 2

### Scenario 2

- Ada verifikasi eksplisit lewat opsi `Run KEM self-test before Scenario 2`.
- Firmware membandingkan `ss_enc` dan `ss_dec` (`memcmp`).

### Scenario 1

- Firmware mencetak `ss_dec_hex` (hex shared secret hasil decaps) pada setiap iterasi sukses.
- Subscriber mencetak `ss_enc_hex` (hex shared secret hasil encaps) pada CSV subscriber.
- Script `tools/join_subscriber_csv.py` menggabungkan keduanya dan menghasilkan kolom `ss_valid` (1 jika secret cocok, 0 jika tidak/kurang data).

Catatan: ini adalah verifikasi **offline** (host-side) untuk dataset, bukan perbandingan real-time di firmware.

---

## 9) Konfigurasi Final Pengambilan Data (Recommended)

### Scenario 1 final-data profile

- `Scenario = 1`
- `Iterations per run >= 100`
- `MQTT_RECONNECT_EACH_ITER`:
  - `y` untuk run mode `reconn`.
  - `n` untuk run mode `session`.
- Wi-Fi sinyal stabil, broker/subscriber start dulu sebelum flash.
- Jalankan 12 run total: 3 varian × 2 PIE × 2 reconnect mode.

### Scenario 2 final-data profile

- `Scenario = 2`
- `Iterations per run >= 100`
- Opsional:
  - Self-test `y` di awal validasi campaign.
  - Microbench `y` hanya saat diagnosis kernel.
- Hindari workload lain di board saat capture.

### Praktik konsistensi untuk kedua skenario

- Samakan varian, PIE mode, baud UART, dan sample config antar run perbandingan.
- Drop warmup samples saat analisis (contoh 5 iter awal).
- Catat metadata run: timestamp, varian, pieon/off, scenario, reconnect mode.

---

## 10) Urutan Eksekusi per Skenario

### Scenario 1

1. Start broker + subscriber.
2. `idf.py set-target esp32s3`
3. `idf.py menuconfig` (Scenario 1 + MQTT/Wi-Fi + reconnect mode).
4. `idf.py build`
5. `idf.py -p /dev/ttyUSBx flash`
6. Capture UART dengan reconnect tag:
   - `bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff reconn`
   - atau: `bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff session`
7. Stop subscriber; cek `data/csv_subscriber/` ada output-nya.
8. Join CSV: `python3 tools/join_subscriber_csv.py ...`

### Scenario 2

1. `idf.py set-target esp32s3`
2. `idf.py menuconfig` (Scenario 2 + opsional self-test/microbench).
3. `idf.py build`
4. `idf.py -p /dev/ttyUSBx flash`
5. `bash tools/capture_uart.sh /dev/ttyUSB0 s2 512 pieoff`

---

## 11) Setup Fedora Mosquitto (Scenario 1)

### Install

```bash
sudo dnf install -y mosquitto mosquitto-clients
```

### Siapkan password file

```bash
sudo mosquitto_passwd -c /etc/mosquitto/pwfile user
```

Masukkan password saat diminta.

### Pakai config contoh project

```bash
sudo cp tools/fedora_mosquitto.conf /etc/mosquitto/mosquitto.conf
```

Isi config:
- listener `1883`
- `allow_anonymous false`
- `password_file /etc/mosquitto/pwfile`
- `set_tcp_nodelay true` (disarankan; mencegah artefak Nagle/Delayed ACK pada payload kecil)

### Jalankan broker

```bash
sudo mosquitto -c tools/fedora_mosquitto.conf
```

Atau via systemd (opsional):

```bash
sudo systemctl enable --now mosquitto
sudo systemctl status mosquitto
```

## 12) Setup Python Environment untuk Subscriber/Analyzer

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install paho-mqtt pqcrypto
```

Catatan: script sudah punya fallback nama modul `ml_kem_*` ke `kyber*` bila paket expose API lama.

## 13) Menjalankan Subscriber (Fedora — Script Utama)

```bash
python3 tools/fedora_subscriber.py \
  --broker 192.168.1.10 \
  --port 1883 \
  --username user \
  --password pass \
  --variant 512 \
  --pie pieoff \
  --topic-pubkey pqc/handshake/pubkey \
  --topic-ciphertext pqc/handshake/ciphertext
```

Output:
- Publish ciphertext ke topic ciphertext.
- Simpan CSV ke `data/csv_subscriber/` (jika `--csv-out` tidak diset, nama otomatis).
- Print line `iter,variant,encaps_us,ss_enc_hex`.

## 14) Menjalankan Subscriber (Windows)

```bash
python tools/windows_subscriber.py --broker <broker-ip> --username user --password pass --variant 512
```

Behavior sama; perbedaan utama ada pada default argument.

## 15) Subscriber Lokal Temporary (Single-Machine)

Untuk single-machine testing (broker + subscriber di satu laptop):

```bash
python3 tools/temporary/fedora_subscriber_local.py \
  --broker 127.0.0.1 \
  --port 1883 \
  --username user \
  --password pass \
  --variant 512
```

Gunakan hanya untuk testing awal, bukan pengambilan data final.

## 16) Capture UART Metrics

```bash
bash tools/capture_uart.sh <port> <scenario:s1|s2> <variant:512|768|1024> <pie:pieon|pieoff> [reconn|session untuk s1] [baud]
```

Contoh:

```bash
# Scenario 2
bash tools/capture_uart.sh /dev/ttyUSB0 s2 768 pieon 921600

# Scenario 1 — auth per-iter
bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff reconn

# Scenario 1 — session reuse
bash tools/capture_uart.sh /dev/ttyUSB0 s1 768 pieon session 921600
```

Output:
- Raw log: `data/raw/<scenario>_<variant>_<pie>[_<reconn|session>]_<timestamp>.log`
- CSV: `data/csv/<scenario>_<variant>_<pie>[_<reconn|session>]_<timestamp>.csv`

Tips:
- Reset board sebelum capture agar header CSV dan iterasi dari awal tercatat.
- Hentikan monitor sesuai instruksi script (`Ctrl+]` atau `Ctrl+C`).

## 17) Join CSV ESP + Subscriber

Tujuan: menambahkan `encaps_us` + `ss_enc_hex` dari subscriber ke tiap row ESP Scenario 1, dan menghitung `ss_valid`.

```bash
python3 tools/join_subscriber_csv.py \
  --esp-dir data/csv \
  --sub-dir data/csv_subscriber \
  --out-dir data/csv_joined \
  --scenario s1
```

Output:
- File `*_joined.csv` di `data/csv_joined`.

Cara pairing file:
- Berdasarkan pola nama: `s1_<variant>_<pie>_<reconn|session>_<timestamp>.csv`.
- Dicocokkan per `(scenario, variant, pie, reconnect_tag)` lalu urut timestamp.

## 18) Analyze PIE Metrics

### Scenario 2

```bash
python3 tools/analyze_pie_metrics.py \
  --scenario s2 \
  --pie-off data/csv/s2_512_pieoff_YYYYMMDD_HHMMSS.csv \
  --pie-on  data/csv/s2_512_pieon_YYYYMMDD_HHMMSS.csv \
  --drop-warmup 5 \
  --out data/analysis/s2_512_pie_compare.md
```

### Scenario 1

```bash
python3 tools/analyze_pie_metrics.py \
  --scenario s1 \
  --pie-off data/csv/s1_512_pieoff_reconn_YYYYMMDD_HHMMSS.csv \
  --pie-on  data/csv/s1_512_pieon_reconn_YYYYMMDD_HHMMSS.csv \
  --drop-warmup 5 \
  --out data/analysis/s1_512_reconn_pie_compare.md
```

Opsional: `--include-loss-samples` untuk ikutkan iterasi `packet_loss=1` pada distribusi metrik (mis. `handshake_us`/`energy_uj`). Nilai `net_latency_us`/`jitter_us` pada loss biasanya `-1` dan akan diabaikan.

Output: Tabel markdown p50/p95 + delta dan delta%.

## 19) End-to-End Terminal Sequence (Scenario 1, Lengkap)

Terminal A (broker):

```bash
sudo mosquitto -c tools/fedora_mosquitto.conf
```

Terminal B (subscriber):

```bash
source .venv/bin/activate
python3 tools/fedora_subscriber.py --broker 192.168.1.10 --username user --password pass --variant 512 --pie pieoff
```

Terminal C (ESP project):

```bash
idf.py set-target esp32s3
idf.py menuconfig      # Set S1, 512, pieoff, MQTT, reconnect=y
idf.py build
idf.py -p /dev/ttyUSB0 flash
bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff reconn
```

Setelah selesai, ulangi Terminal C dengan `menuconfig` reconnect=n dan capture tag `session`.

## 20) Troubleshooting Cepat

- `No compatible pqcrypto KEM module found`
  - Pastikan `pip install pqcrypto` sukses di environment aktif.
- Subscriber tidak menerima pesan
  - Cek topic pubkey/ciphertext harus identik di ESP dan subscriber.
  - Cek username/password broker.
- `join_subscriber_csv.py` tidak menemukan pasangan
  - Cek pola nama file harus sesuai format (termasuk reconnect tag).
  - Pastikan varian/pie/reconnect tag sama antara ESP dan subscriber.
- Capture UART kosong
  - Cek baud harus sesuai menuconfig.
  - Cek port `/dev/ttyUSBx` benar.
  - Reset board lalu ulang capture.
- ESP brownout / restart saat Scenario 1
  - Pastikan sumber daya adalah adaptor USB ≥1A, bukan CP2102. Lihat wiring di README.
- Task watchdog warning
  - Naikkan `INA219 sample period (ms)` ke 10ms di menuconfig.

---

Definisi dan asal setiap metrik dijelaskan di `docs/04-metrics-reference.md`.
