# Project Structure and File Roles

Dokumen ini menjelaskan peran tiap folder dan file inti pada project `mlkem`, dengan fokus pada jalur eksekusi Scenario 1/2 dan pipeline pengukuran.

## 1) Gambaran Besar Arsitektur

- `components/`: implementasi kriptografi ML-KEM (clean baseline + PIE optimized).
- `main/`: aplikasi ESP-IDF (entrypoint, scenario logic, metrics, energy sampling, Wi-Fi/MQTT).
- `tools/`: script host-side untuk capture UART, subscriber MQTT, join CSV, analisis PIE.
- `data/`: artefak hasil eksperimen.
- `build/`: hasil kompilasi sementara dan artefak firmware dari `idf.py build`.
- `docs/`: dokumentasi teknis.

## 2) Top-Level Files yang Penting

- `CMakeLists.txt`: root project CMake untuk ESP-IDF.
- `README.md`: quickstart wiring, build/flash/monitor, experiment matrix, dan alur eksperimen.
- `sdkconfig*`: hasil konfigurasi `menuconfig` (generated config snapshot).

## 3) Detail Folder dan File

## `components/`

### `components/pqclean/`

Peran: implementasi baseline ML-KEM clean C dari PQClean.

- `components/pqclean/CMakeLists.txt`
  - Memilih direktori source sesuai varian (`ML-KEM-512/768/1024`).
  - Mendaftarkan source KEM clean (`cbd.c`, `indcpa.c`, `kem.c`, `ntt.c`, `poly*.c`, dst).
  - Menambahkan `common/fips202.c` dan `esp_randombytes.c`.
  - Dipakai saat `CONFIG_MLKEM_PIE_OPT=n`.
- `components/pqclean/esp_randombytes.c`
  - Adapter RNG PQClean ke RNG ESP-IDF.
- `components/pqclean/crypto_kem/ml-kem-*/clean/*`
  - Implementasi matematis KEM (keygen/encaps/decaps dan primitive internal).
- `components/pqclean/common/fips202.c`
  - SHAKE/FIPS202 yang dipakai oleh flow ML-KEM.

### `components/mlkem_pie/`

Peran: versi akselerasi PIE untuk ESP32-S3, terutama pada kernel NTT.

- `components/mlkem_pie/CMakeLists.txt`
  - Memilih varian ML-KEM aktif.
  - Mengganti source `ntt.c` clean dengan `ntt_pie.c`.
  - Men-set `MLKEM_PIE=1` dan opsional `MLKEM_PIE_AGGRESSIVE=1`.
  - Dipakai saat `CONFIG_MLKEM_PIE_OPT=y`.
- `components/mlkem_pie/pie_vec.h`
  - Helper intrinsics/inline untuk operasi vektor PIE.
- `components/mlkem_pie/ml-kem-*/ntt_pie.c`
  - Implementasi NTT/invNTT berbasis optimisasi PIE per varian.

## `main/`

Peran: orkestrasi runtime eksperimen dan logging.

- `main/CMakeLists.txt`
  - Mendaftarkan semua source aplikasi.
  - Menentukan dependency KEM:
    - `pqclean` jika PIE off.
    - `mlkem_pie` jika PIE on.
- `main/Kconfig.projbuild`
  - Mendefinisikan menu `ML-KEM App` di `idf.py menuconfig`.
  - Berisi pilihan varian, scenario, MQTT/Wi-Fi, INA219, self-test, microbench, PIE.
- `main/app_main.c`
  - Entrypoint firmware.
  - Set baud UART, init energy sampler, lock CPU policy (Scenario 2 + PM), dispatch ke scenario.
- `main/scenario_mqtt.c`
  - Scenario 1: keygen di ESP -> publish pubkey via MQTT -> terima ciphertext -> decaps.
  - Menghitung metric handshake/network/compute + resource usage.
- `main/scenario_local.c`
  - Scenario 2: keygen+encaps+decaps semua lokal di ESP.
  - Opsi self-test shared secret dan microbench.
- `main/microbench.c`
  - Benchmark mikro untuk `ntt`, `invntt`, `basemul` (saat diaktifkan).
- `main/metrics.c`
  - Satu titik formatting output CSV (header + row Scenario 1/2).
- `main/energy_sampler.c`
  - Sampling INA219 periodik dan integrasi energi (uJ) pada window operasi.
- `main/ina219.c`
  - Driver/config pembacaan sensor INA219.
- `main/wifi_manager.c`
  - Init STA mode, event handler koneksi, retry.
- `main/mlkem_config.h`
  - Binding compile-time varian -> symbol PQClean (`mlkem_keypair`, `mlkem_encaps`, `mlkem_decaps`) + ukuran buffer.

## `tools/`

Peran: menjalankan eksperimen host-side dan post-processing data.

- `tools/capture_uart.sh`: monitor UART + simpan raw log + ekstrak CSV. Mendukung reconnect tag untuk Scenario 1 (`reconn`/`session`).
- `tools/fedora_subscriber.py`: subscriber MQTT utama untuk Scenario 1 (Fedora/Linux); encaps dan kirim ciphertext balik; simpan timing ke `data/csv_subscriber/`.
- `tools/windows_subscriber.py`: subscriber MQTT dengan default Windows-friendly, fungsionalitas inti sama.
- `tools/temporary/fedora_subscriber_local.py`: varian lokal/testing subscriber untuk single-machine.
- `tools/join_subscriber_csv.py`: gabung CSV ESP Scenario 1 dengan CSV encaps subscriber.
- `tools/analyze_pie_metrics.py`: bandingkan PIE on/off berbasis p50/p95.
- `tools/fedora_mosquitto.conf`: contoh config broker Mosquitto (auth + listener).

## `data/`

- `data/raw/`: log UART penuh yang sudah disanitasi ANSI.
- `data/csv/`: CSV metric dari ESP (Scenario 1 dan 2).
- `data/csv_subscriber/`: CSV timing encaps dari subscriber (Scenario 1).
- `data/csv_joined/`: hasil join CSV ESP + subscriber (Scenario 1, setelah `join_subscriber_csv.py`).

File naming pattern:
- Scenario 2: `s2_<variant>_<pieon|pieoff>_YYYYMMDD_HHMMSS.csv`
- Scenario 1: `s1_<variant>_<pieon|pieoff>_<reconn|session>_YYYYMMDD_HHMMSS.csv`

## `build/` itu apa?

`build/` adalah output folder dari ESP-IDF build system. Isinya object file, cache CMake/Ninja, artifact firmware seperti binary image dan metadata flash. Folder ini bukan kode sumber yang diedit manual.

## 4) Penjelasan Khusus Semua CMakeLists.txt

### `CMakeLists.txt` (root)

Fungsi:
- Menentukan minimum CMake version.
- Memanggil `project.cmake` milik ESP-IDF.
- Mendefinisikan nama project (`mlkem_esp32`).

Makna praktis: ini pintu masuk agar seluruh mekanisme component discovery, Kconfig merge, dan proses build ESP-IDF berjalan.

### `main/CMakeLists.txt`

Fungsi:
- Memilih backend KEM komponen berdasarkan `CONFIG_MLKEM_PIE_OPT`.
- Mendaftarkan source aplikasi (`app_main.c`, `scenario_*`, `metrics.c`, dsb).
- Menetapkan dependency framework (Wi-Fi, MQTT, timer, driver, PM).

Makna praktis: menghubungkan logika aplikasi dengan backend kripto yang dipilih di menuconfig.

### `components/pqclean/CMakeLists.txt`

Fungsi:
- Menentukan varian KEM aktif.
- Meng-include source clean implementation + common hash + RNG bridge.

Makna praktis: baseline referensi (non-PIE) yang lebih portabel.

### `components/mlkem_pie/CMakeLists.txt`

Fungsi:
- Menentukan varian KEM aktif.
- Substitusi kernel NTT ke file PIE (`ntt_pie.c`).
- Menambahkan compile define untuk mode PIE normal/aggressive.

Makna praktis: jalur optimisasi khusus ESP32-S3 tanpa mengubah API ML-KEM pada layer aplikasi.

## 5) Ringkas Alur Build-Time Selection

1. User set opsi di `menuconfig` (`Kconfig.projbuild`).
2. `CONFIG_MLKEM_PIE_OPT` dibaca `main/CMakeLists.txt`.
3. Build memilih komponen `pqclean` atau `mlkem_pie`.
4. `mlkem_config.h` memastikan symbol API varian tetap konsisten di aplikasi.

---

Lanjutan detail konfigurasi menu ada di `docs/02-menuconfig-guide.md`.
