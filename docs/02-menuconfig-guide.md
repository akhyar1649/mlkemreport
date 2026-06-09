# Menuconfig Guide (ML-KEM App)

Dokumen ini adalah panduan lengkap untuk `idf.py menuconfig` — mencakup gambaran umum, penjelasan per opsi, nilai default, alasan default, dan dampak jika diubah.

## 1) Kenapa Menuconfig Banyak Sekali?

`idf.py menuconfig` menyatukan semua Kconfig dari:

- ESP-IDF core/components (FreeRTOS, Wi-Fi, MQTT, driver, power, logging, dll).
- Project components kamu sendiri.

Di project ini, menu custom utama berasal dari:

- `main/Kconfig.projbuild` -> menu `ML-KEM App`.

Jadi benar: sebagian besar menu adalah default framework, tetapi `ML-KEM App` adalah menu buatan project ini.

## 2) Peta Cepat: Mana Yang Wajib Diubah?

Untuk mayoritas eksperimen, prioritas edit:

1. `ML-KEM variant`
2. `Scenario`
3. `Enable PIE kernels`
4. `Iterations per run`
5. Scenario 1 saja: `WIFI_*` + `MQTT_*` + `MQTT_RECONNECT_EACH_ITER`
6. Pengukuran energi: `INA219_*`
7. `CSV UART baudrate`

Menu lain tetap penting dipahami, tapi tidak selalu perlu diubah tiap run.

## 3) Deep Dive Semua Opsi di `ML-KEM App`

## A. Core Experiment Controls

| Opsi | Default | Kenapa default begitu | Jika diubah | Dampak |
|------|---------|------------------------|-------------|--------|
| `ML-KEM variant` | `ML-KEM-512` | Varian paling ringan; cocok untuk iterasi awal/debug cepat | `768` / `1024` | Ukuran kunci/ciphertext naik; waktu komputasi & energi naik |
| `Scenario` | `Scenario 2` | Bisa langsung benchmark tanpa broker/subscriber/Wi-Fi | `Scenario 1` | Butuh infrastruktur MQTT + subscriber; metrik keluaran jadi `s1` |
| `Iterations per run` | `100` | Kompromi durasi run vs stabilitas statistik (p50/p95); lihat justifikasi di `docs/06-experiment-plan.md` | 10–1000 | Terlalu kecil: varians tinggi; terlalu besar: log panjang, risiko timeout/handshake gagal (S1) |
| `CSV UART baudrate` | `921600` | Minimalkan bottleneck serial saat banyak baris CSV | 115200–921600 | Baud rendah bisa drop/truncate log saat burst output |

## B. PIE Optimization Controls

| Opsi | Default | Kenapa default begitu | Jika diubah | Dampak |
|------|---------|------------------------|-------------|--------|
| `Enable PIE kernels` | `n` (off) | Baseline referensi PQClean clean lebih portabel dan mudah dibandingkan | `y` | Build memakai `mlkem_pie`; NTT diganti `ntt_pie.c`; biasanya `time_us/cycles` turun di operasi ber-NTT |
| `Enable aggressive PIE optimizations` | `y` (hanya saat PIE on) | Jika PIE sudah diaktifkan, default mengejar performa maksimum | `n` | Tetap PIE, tapi tanpa batching `8x2` + constant vector tables; biasanya sedikit lebih lambat, lebih konservatif |

### PIE vs Aggressive PIE: Bedanya Apa?

- **PIE (normal)**: NTT/reduce scalar diganti operasi vector 8-lane (`EE.VADDS`, `EE.VMUL`, dll) di `ntt_pie.c` + `pie_vec.h`.
- **Aggressive PIE**: optimasi tambahan batch `8x2`, constant vector tables, dan fewer SAR setup di helper yang sama.

Penjelasan lengkap (before/after code path, dampak praktis): lihat `docs/05-pie-optimization.md`.

## C. Scenario 2 Special Toggles

| Opsi | Default | Kenapa default begitu | Jika diubah | Dampak |
|------|---------|------------------------|-------------|--------|
| `Run KEM self-test before Scenario 2` | `n` | Tidak menambah waktu sebelum benchmark utama | `y` | 1x keygen+encaps+decaps + `memcmp` shared secret; log pass/fail di UART (bukan CSV row) |
| `Enable NTT/invNTT/basemul microbench` | `n` | Mode diagnostik, bukan jalur eksperimen utama | `y` | Menambah baris CSV `mb_ntt`, `mb_invntt`, `mb_basemul` sebelum loop normal |
| `Microbench iterations` | `10` | Cukup untuk smoke-test kernel tanpa memperpanjang run | 30–200 | Lebih stabil statistik kernel-level, tapi run lebih lama |

Apa itu microbench? Pengukuran operasi kecil/terisolasi (kernel-level), bukan end-to-end KEM penuh. Dipakai untuk melihat bottleneck granular. Penjelasan lengkap: lihat `docs/05-pie-optimization.md`.

## D. Scenario 1 Connectivity Controls

| Opsi | Default | Kenapa default begitu | Jika diubah | Dampak |
|------|---------|------------------------|-------------|--------|
| `Wi-Fi SSID` | `KUMALA_10` | Placeholder SSID lingkungan dev | SSID jaringan Anda | Tanpa match: ESP tidak connect, Scenario 1 gagal total |
| `Wi-Fi password` | `kumalasari10` | Placeholder password dev | Password jaringan Anda | Salah password -> gagal koneksi |
| `MQTT broker URI` | `mqtt://192.168.1.10:1883` | Contoh IP LAN dev | `mqtt://<ip-broker>:1883` | Harus reachable dari ESP; salah URI -> tidak publish/subscribe |
| `MQTT username` | `user` | Contoh auth broker | username broker Anda | Mismatch -> broker reject connection |
| `MQTT password` | `pass` | Contoh auth broker | password broker Anda | Mismatch -> auth gagal |
| `MQTT topic (pubkey)` | `pqc/handshake/pubkey` | Konvensi topic handshake project | topic custom | Harus sama dengan subscriber, atau handshake putus |
| `MQTT topic (ciphertext)` | `pqc/handshake/ciphertext` | Konvensi topic balasan ciphertext | topic custom | Harus sama dengan subscriber |
| `MQTT QoS` | `1` | Balance reliability vs overhead | `0` / `2` | QoS0 lebih cepat tapi bisa loss; QoS2 lebih reliable tapi overhead lebih besar |
| `Reconnect MQTT each iteration` | `y` | Mensimulasikan auth/session refresh per iterasi | `n` | Lihat penjelasan lengkap di bawah |
| `Handshake timeout (ms)` | `5000` | Cukup untuk LAN + subscriber Python | 1000–30000 | Terlalu kecil -> `packet_loss=1` naik; terlalu besar -> run lambat saat subscriber mati |

### `MQTT_RECONNECT_EACH_ITER`: Session Reuse vs Auth per-Iteration

Opsi ini menentukan mode koneksi MQTT selama eksperimen Scenario 1:

**`y` = Auth per-Iteration (`reconn` mode)**

Firmware reconnect ke broker MQTT sebelum setiap iterasi. Mensimulasikan skenario IoT baterai di mana perangkat:
- Tidur (deep sleep) antar pengukuran.
- Bangun, koneksi ulang, kirim data, tidur kembali.
- Setiap sesi memerlukan full ML-KEM key establishment baru.

Representasi nyata: smart meter, sensor lingkungan baterai, sensor pertanian periodik.

Gunakan tag `reconn` saat capture: `bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff reconn`

**`n` = Session Reuse (`session` mode)**

Firmware membuka satu koneksi MQTT dan menjalankan semua iterasi pada sesi yang sama. Mensimulasikan skenario IoT yang selalu online:
- Sensor pabrik yang stream data terus-menerus 24/7.
- Gateway IoT yang maintain koneksi persisten.
- PLC atau aktuator industri.

Representasi nyata: sensor vibration mesin, monitoring IIoT real-time.

Gunakan tag `session` saat capture: `bash tools/capture_uart.sh /dev/ttyUSB0 s1 512 pieoff session`

**Mengapa membandingkan keduanya penting?**

Perbandingan reconn vs session memungkinkan kuantifikasi overhead autentikasi ML-KEM per sesi. Jika energi per iterasi pada mode `reconn` jauh lebih tinggi dari mode `session`, maka implementasi session resumption (seperti TLS session tickets) sangat relevan untuk perangkat IoT baterai. Ini menjadi rekomendasi desain yang konkret untuk deployment post-quantum MQTT di dunia nyata.

## E. INA219 + Signal Sampling Controls

| Opsi | Default | Kenapa default begitu | Jika diubah | Dampak |
|------|---------|------------------------|-------------|--------|
| `INA219 I2C port` | `0` | Port I2C umum di board devkit | port lain | Sensor tidak terbaca jika wiring/port salah |
| `INA219 SDA GPIO` | `8` | Sesuai wiring contoh README | GPIO sesuai board | Salah pin -> energi invalid/0 |
| `INA219 SCL GPIO` | `9` | Sesuai wiring contoh README | GPIO sesuai board | Salah pin -> energi invalid/0 |
| `INA219 I2C address` | `0x40` | Address umum modul INA219 | sesuai jumper A0/A1 | Address salah -> init gagal |
| `INA219 shunt resistor (mOhm)` | `100` | Nilai shunt umum modul breakout | nilai fisik shunt Anda | Salah kalibrasi -> arus/daya salah -> `energy_uj` bias |
| `INA219 max expected current (mA)` | `1000` | Headroom untuk devkit + beban normal | sesuai supply | Mempengaruhi skala kalibrasi arus |
| `INA219 ADC mode` | `12-bit 8-sample` | Kompromi noise vs latency sampling | `16-sample` / `1-sample` | 16S lebih halus tapi lebih lambat; 1S lebih cepat tapi lebih berisik |
| `INA219 sample period (ms)` | `2` | Cukup granular tanpa membebani CPU/I2C | 1–20 | Terlalu kecil -> watchdog/task starvation risk; terlalu besar -> underestimasi energi transient |
| `INA219 I2C frequency (Hz)` | `400000` | Fast-mode umum dan stabil | 100000 | Lebih lambat, sedikit kurangi beban bus |
| `RSSI_SAMPLE_EVERY_ITER` | `y` | Data kualitas link berguna untuk analisis S1 | `n` | Kolom `rssi_dbm` tetap ada tapi tidak di-update tiap iter |

## 4) Menu Lain di Luar `ML-KEM App` (Ringkas)

Menu default ESP-IDF yang sering relevan:

- `Component config -> FreeRTOS -> Tick rate (Hz)`
  - Bisa memengaruhi granularitas timing task sampler.
- `Component config -> Power Management`
  - Berpengaruh jika ingin lock frekuensi CPU (terutama untuk kestabilan benchmark).
- `Serial flasher config`
  - Opsi terkait flashing/monitor.
- `Component config -> ESP System / Log output`
  - Untuk level log jika ingin kurangi noise.

## 5) Rekomendasi Profil Konfigurasi

## Profil A: Baseline Non-PIE (perbandingan adil)

- PIE off
- Self-test on (sekali awal validasi), microbench off
- Iterasi >= 100
- Scenario sesuai kebutuhan (s1/s2)

## Profil B: PIE Impact Study

- Jalankan 3 set:
  1. PIE off
  2. PIE on + aggressive off
  3. PIE on + aggressive on
- Gunakan parameter lain identik.

## Profil C: Kernel Diagnosis

- Scenario 2
- Microbench on
- Iterasi microbench cukup (mis. 30-100) untuk distribusi stabil.

## Profil D: Scenario 1 Auth Study

- Jalankan dua set dengan varian, PIE, dan kondisi jaringan identik:
  1. `MQTT_RECONNECT_EACH_ITER = y` (auth per-iter / `reconn`)
  2. `MQTT_RECONNECT_EACH_ITER = n` (session reuse / `session`)

## 6) Prinsip Memilih Nilai untuk Eksperimen Final

1. **Jangan pakai default Wi-Fi/MQTT mentah** — ganti ke environment Anda.
2. **Baseline dulu PIE off**, lalu bandingkan PIE on (normal/aggressive terpisah).
3. **Iterasi >= 100** untuk laporan stabil; drop warmup saat analisis (mis. 5 iter pertama).
4. **Microbench hanya saat diagnosis**, bukan untuk dataset performa end-to-end.
5. **Self-test on** di awal campaign Scenario 2 untuk validasi correctness sebelum collect data besar.
6. **Tag reconnect sesuai konfigurasi** (`reconn` untuk `y`, `session` untuk `n`) agar nama file CSV konsisten.

---

Detail teknis PIE ada di `docs/05-pie-optimization.md`.
Panduan eksekusi langkah terminal ada di `docs/03-operational-runbook.md`.
