# BAB III
# METODOLOGI PENELITIAN

## 3.1 Alur Penelitian

Penelitian ini dilakukan untuk menganalisis performa dan QERS pada implementasi standar ML-KEM di protokol MQTT menggunakan instruksi vektor ESP32-S3. Alur penelitian dimulai dari studi literatur, persiapan perangkat, integrasi ML-KEM, implementasi optimasi PIE, perancangan komunikasi MQTT, pengumpulan data, praproses data, dan analisis hasil. Setiap tahap disusun agar pengaruh optimasi komputasi dapat dibedakan dari pengaruh komunikasi jaringan.

Tahap studi literatur digunakan untuk memahami standar ML-KEM, protokol MQTT v5.0, arsitektur ESP32-S3, NTT, pengukuran energi, dan QERS. Tahap persiapan perangkat dilakukan dengan menyiapkan ESP32-S3, sensor INA219, adaptor daya, modul CP2102, router OpenWrt, laptop broker, dan laptop subscriber. Tahap integrasi dilakukan dengan memasukkan implementasi ML-KEM ke dalam proyek ESP-IDF. Tahap optimasi dilakukan dengan menambahkan jalur PIE pada operasi polinomial yang berkaitan dengan NTT.

Gambar 3.1. Alur penelitian.

Keterangan: Gambar 3.1 perlu dibuat sebagai flowchart dari studi literatur sampai analisis QERS. Kotak utama yang disarankan adalah studi literatur, persiapan alat, integrasi ML-KEM, implementasi PIE, konfigurasi MQTT, pengambilan data Scenario 1 dan Scenario 2, praproses data, analisis statistik, dan perhitungan QERS.

## 3.2 Alat dan Bahan

### 3.2.1 Perangkat Keras

Perangkat keras penelitian dipilih untuk merepresentasikan lingkungan IoT lokal yang dapat dikendalikan. ESP32-S3 digunakan sebagai end-node yang menjalankan operasi ML-KEM. INA219 digunakan untuk membaca arus dan tegangan selama pengujian. Router OpenWrt dan broker MQTT digunakan untuk membentuk jaringan lokal. Daftar perangkat keras ditunjukkan pada Tabel 3.1.

Tabel 3.1. Perangkat keras penelitian.

| No | Perangkat | Fungsi |
|---:|---|---|
| 1 | ESP32-S3 DevKit | End-node IoT untuk KeyGen, Decaps, dan benchmark lokal. |
| 2 | INA219 | Sensor arus dan tegangan untuk pengukuran energi. |
| 3 | Adaptor 5V | Sumber daya utama ESP32-S3 selama pengujian. |
| 4 | CP2102 | Antarmuka UART untuk pengambilan log. |
| 5 | Router OpenWrt | Infrastruktur Wi-Fi dan LAN pengujian. |
| 6 | Laptop A Fedora | Menjalankan Mosquitto broker. |
| 7 | Laptop B Fedora | Menjalankan Python subscriber. |
| 8 | Kabel jumper dan breadboard | Koneksi daya, I2C, dan UART. |

ESP32-S3 dipilih karena menyediakan instruksi vektor PIE pada arsitektur Xtensa LX7 (Espressif Systems, 2025). INA219 digunakan karena pengukuran energi diperlukan untuk menilai kelayakan implementasi ML-KEM pada perangkat IoT. CP2102 digunakan hanya sebagai jalur UART agar sumber daya utama ESP32-S3 tetap berasal dari adaptor 5V.

### 3.2.2 Perangkat Lunak

Perangkat lunak penelitian digunakan untuk membangun firmware, menjalankan komunikasi MQTT, menggabungkan data, dan menghitung hasil analisis. ESP-IDF digunakan sebagai framework pengembangan ESP32-S3. Mosquitto digunakan sebagai broker MQTT. Python digunakan untuk subscriber, penggabungan CSV, analisis statistik, dan perhitungan QERS. Daftar perangkat lunak ditunjukkan pada Tabel 3.2.

Tabel 3.2. Perangkat lunak penelitian.

| No | Perangkat Lunak | Fungsi |
|---:|---|---|
| 1 | ESP-IDF v5.4 | Build, flash, dan runtime firmware ESP32-S3. |
| 2 | PQClean | Sumber implementasi ML-KEM berbasis C. |
| 3 | Mosquitto | Broker MQTT v5.0. |
| 4 | Python 3 | Subscriber, join CSV, dan analisis data. |
| 5 | Paho MQTT | Library MQTT untuk subscriber Python. |
| 6 | pandas dan numpy | Pengolahan CSV dan perhitungan QERS. |

## 3.3 Perancangan Sistem

### 3.3.1 Rancangan Pengukuran Energi

Pengukuran energi dilakukan dengan memasang INA219 secara in-line pada jalur daya ESP32-S3. Arus dari adaptor 5V masuk ke terminal `Vin+` INA219 dan keluar melalui `Vin-` menuju pin 5V ESP32-S3. Jalur GND adaptor, INA219, ESP32-S3, dan CP2102 disatukan sebagai common ground. Komunikasi INA219 ke ESP32-S3 menggunakan I2C dengan SDA pada GPIO8 dan SCL pada GPIO9.

Gambar 3.2. Skema wiring INA219, adaptor 5V, ESP32-S3, dan CP2102.

Keterangan: Gambar 3.2 perlu menunjukkan adaptor 5V sebagai sumber utama, INA219 pada jalur positif, ESP32-S3 sebagai beban, CP2102 sebagai UART, serta koneksi SDA GPIO8 dan SCL GPIO9. Gambar ini perlu dibuat sendiri berdasarkan rangkaian aktual penelitian.

### 3.3.2 Integrasi ML-KEM dan PIE

Implementasi ML-KEM diintegrasikan ke dalam proyek ESP-IDF dengan tiga varian, yaitu ML-KEM-512, ML-KEM-768, dan ML-KEM-1024. Setiap varian dikompilasi dalam dua konfigurasi, yaitu tanpa PIE dan dengan PIE. Konfigurasi tanpa PIE digunakan sebagai baseline. Konfigurasi dengan PIE digunakan untuk mengukur pengaruh instruksi vektor ESP32-S3.

Optimasi PIE difokuskan pada jalur operasi polinomial yang berkaitan dengan NTT dan inverse NTT. Jalur tanpa PIE menjalankan operasi menggunakan loop skalar. Jalur dengan PIE menggunakan register vektor 128-bit untuk memproses delapan koefisien `int16_t` secara paralel. Optimasi ini tidak mengubah parameter keamanan, format kunci, format ciphertext, atau alur standar ML-KEM.

Tabel 3.3. Perbandingan jalur tanpa PIE dan dengan PIE.

| Komponen | Tanpa PIE | Dengan PIE |
|---|---|---|
| Register | Register umum | Register vektor 128-bit |
| Granularitas data | Koefisien diproses skalar | Delapan koefisien `int16_t` per operasi vektor |
| NTT/inverse NTT | Loop C skalar | Vector load, vector store, add, sub, dan multiply helper |
| Tujuan | Baseline | Mengurangi waktu eksekusi dan siklus CPU |

### 3.3.3 Rancangan Handshake MQTT

Scenario 1 menggunakan MQTT v5.0 untuk menguji pertukaran material ML-KEM secara end-to-end. ESP32-S3 membuat pasangan kunci melalui KeyGen dan mempublikasikan public key ke broker. Subscriber menerima public key, menjalankan Encaps, dan mengirim ciphertext kembali melalui broker. ESP32-S3 menerima ciphertext dan menjalankan Decaps untuk memperoleh shared secret.

QoS 1 digunakan agar setiap pesan memiliki mekanisme acknowledgment. Penggunaan QoS 1 menjaga reliabilitas pengiriman tanpa menambah prosedur konfirmasi sebanyak QoS 2. Dengan rancangan ini, penelitian dapat mengamati dampak ukuran public key dan ciphertext terhadap handshake MQTT.

Gambar 3.3. Alur handshake ML-KEM melalui MQTT v5.0.

Keterangan: Gambar 3.3 perlu menampilkan urutan ESP32-S3 KeyGen, publish public key, broker meneruskan pesan, subscriber Encaps, publish ciphertext, broker meneruskan ciphertext, dan ESP32-S3 Decaps. Gambar juga perlu memberi label topic public key dan topic ciphertext.

## 3.4 Skenario Pengujian

### 3.4.1 Scenario 2 — Local Compute

Scenario 2 dirancang untuk mengukur performa komputasi murni ML-KEM pada ESP32-S3. Pada skenario ini, KeyGen, Encaps, dan Decaps dijalankan seluruhnya di ESP32-S3. Pengujian dilakukan pada tiga varian ML-KEM dan dua status PIE. Matriks pengujian ditunjukkan pada Tabel 3.4.

Tabel 3.4. Matriks pengujian Scenario 2.

| Varian | PIE | Operasi | Iterasi |
|---|---|---|---:|
| ML-KEM-512 | off/on | KeyGen, Encaps, Decaps | 100 per operasi |
| ML-KEM-768 | off/on | KeyGen, Encaps, Decaps | 100 per operasi |
| ML-KEM-1024 | off/on | KeyGen, Encaps, Decaps | 100 per operasi |

Metrik yang dicatat pada Scenario 2 adalah `cycles`, `time_us`, `energy_uj`, `heap_free`, `heap_min`, dan `stack_hwm_bytes`. Metrik tersebut digunakan untuk menilai pengaruh PIE tanpa gangguan jaringan. Hasil Scenario 2 menjadi dasar untuk melihat apakah optimasi instruksi benar-benar mempercepat operasi ML-KEM.

### 3.4.2 Scenario 1 — MQTT Handshake

Scenario 1 dirancang untuk mengukur performa ML-KEM pada komunikasi MQTT. Pada skenario ini, ESP32-S3 menjalankan KeyGen dan Decaps, sedangkan subscriber menjalankan Encaps. Pengujian dilakukan pada tiga varian ML-KEM, dua status PIE, dan dua mode koneksi. Matriks pengujian ditunjukkan pada Tabel 3.5.

Tabel 3.5. Matriks pengujian Scenario 1.

| Faktor | Nilai |
|---|---|
| Varian ML-KEM | 512, 768, 1024 |
| Status PIE | off, on |
| Mode koneksi | session, reconnect |
| Iterasi | 100 per konfigurasi |
| Total konfigurasi | 12 konfigurasi |

Mode session mempertahankan koneksi MQTT selama seluruh iterasi. Mode reconnect melakukan koneksi ulang pada setiap iterasi. Perbandingan kedua mode digunakan untuk mengukur biaya koneksi ulang terhadap total latency dan total energi.

## 3.5 Teknik Pengumpulan Data

Data dikumpulkan melalui log CSV dari firmware ESP32-S3 dan subscriber Python. Pada Scenario 2, seluruh metrik berasal dari ESP32-S3. Pada Scenario 1, log ESP32-S3 digabung dengan log subscriber berdasarkan nomor iterasi. Penggabungan ini menghasilkan CSV joined yang memuat metrik KeyGen, Decaps, handshake, reconnect, dan Encaps.

Energi dihitung dari pembacaan INA219 selama jendela pengukuran. Pada Scenario 2, jendela pengukuran mengikuti durasi masing-masing operasi. Pada Scenario 1, jendela pengukuran mengikuti durasi handshake. Pada mode reconnect, waktu dan energi reconnect dicatat terpisah agar biaya koneksi ulang dapat dianalisis.

Rumus 3.1. Total latency pada mode reconnect.

`total_latency_us = reconnect_us + handshake_us`

Rumus 3.2. Total energi pada mode reconnect.

`total_energy_uj = reconnect_energy_uj + energy_uj`

Pada mode session, nilai reconnect bernilai nol sehingga total latency dan total energi sama dengan latency dan energi handshake. Rumus tersebut digunakan agar perbandingan session dan reconnect dilakukan pada siklus operasi yang sebanding.

## 3.6 Praproses Data

Praproses data dilakukan sebelum analisis statistik dan QERS. Langkah pertama adalah validasi schema CSV agar seluruh kolom yang diperlukan tersedia. Pada Scenario 1, kolom penting mencakup `reconnect_us`, `reconnect_energy_uj`, dan `encaps_us`. Pada Scenario 2, kolom penting mencakup `op`, `cycles`, `time_us`, dan `energy_uj`.

Langkah kedua adalah validasi jumlah iterasi. Setiap konfigurasi Scenario 1 harus memiliki 100 baris. Setiap operasi Scenario 2 harus memiliki 100 sampel. Validasi ini diperlukan agar perbandingan antar-konfigurasi tidak bias akibat jumlah sampel yang berbeda.

Langkah ketiga adalah validasi packet loss. Jika terdapat packet loss, baris tersebut tidak digunakan untuk menghitung latency dan energi, tetapi tetap dicatat sebagai metrik reliabilitas. Pada data final penelitian ini, seluruh konfigurasi Scenario 1 memiliki packet loss nol. Kondisi tersebut membuat seluruh baris valid untuk analisis latency dan energi.

Langkah keempat adalah pemilihan statistik ringkasan. Median digunakan sebagai ringkasan utama karena lebih tahan terhadap spike jaringan dan variasi pembacaan energi. Persentase perubahan digunakan untuk membandingkan PIE dan No PIE. Speed-up dihitung sebagai rasio nilai baseline terhadap nilai optimasi untuk metrik cost.

Selain empat langkah tersebut, analisis statistik pada penelitian ini menerapkan *warmup drop*. Sampel dengan `iter < 5` tidak disertakan pada perhitungan statistik agar ringkasan p50/p95 lebih merepresentasikan kondisi *steady-state* (misalnya setelah stabilisasi Wi-Fi, cache, dan pembacaan INA219). Dengan 100 iterasi per konfigurasi, langkah ini menghasilkan 95 sampel efektif per konfigurasi untuk S1 dan S2.

## 3.7 Analisis Data dan QERS

Analisis data dilakukan dalam empat tahap. Tahap pertama membandingkan PIE off dan PIE on pada Scenario 2. Tahap kedua membandingkan PIE off dan PIE on pada Scenario 1. Tahap ketiga membandingkan mode session dan reconnect. Tahap keempat membandingkan ML-KEM-512, ML-KEM-768, dan ML-KEM-1024 sebagai trade-off antara keamanan, ukuran payload, latency, dan energi.

QERS dihitung menggunakan data Scenario 1 karena skenario tersebut merepresentasikan komunikasi end-to-end. Metrik QERS dibagi menjadi cost criteria dan benefit criteria. Cost criteria meliputi total latency, total energy, CPU utilization, packet loss, jitter, dan payload size. Benefit criteria meliputi proven resistance, ketersediaan sumber daya, dan RSSI.

Pada penelitian ini, QERS dihitung menggunakan pendekatan **QERS fusion** karena memberikan pemisahan yang jelas antara penalti performa (cost metrics) dan benefit keamanan/sumber daya (benefit metrics) (Rassekhnia, 2026). Seluruh metrik dinormalisasi menggunakan min–max scaling dengan skor maksimum $MS=100$. Penalti performa dirangkum sebagai $P=\sum_i w_i C_{i,norm}$ untuk $C_i\in\{L,J,P_{loss},E,C\}$ dan benefit keamanan dirangkum sebagai $S=\sum_j w_j B_{j,norm}$ untuk $B_j\in\{K,R,Pr,Co\}$. Skor akhir dihitung sebagai $QERS_{fusion}=\alpha(MS-P)+\beta S$ dengan $\alpha+\beta=1$ (Rassekhnia, 2026).

Tabel 3.6. Komponen perhitungan QERS.

| Jenis | Metrik | Arah Preferensi |
|---|---|---|
| Cost (P) | Total latency (L) | Semakin rendah semakin baik |
| Cost (P) | Jitter (J) | Semakin rendah semakin baik |
| Cost (P) | Packet loss rate (Ploss) | Semakin rendah semakin baik |
| Cost (P) | Total energy (E) | Semakin rendah semakin baik |
| Cost (P) | CPU utilization (C) | Semakin rendah semakin baik |
| Benefit (S) | Proven resistance level (Pr) | Semakin tinggi semakin baik |
| Benefit (S) | Key size exchanged (K = pub_bytes + ct_bytes) | Semakin tinggi semakin baik |
| Benefit (S) | RSSI (R) | Semakin tinggi semakin baik |
| Benefit (S) | Crypto overhead bytes (Co = overhead_bytes) | Semakin tinggi semakin baik |

Bobot QERS ditentukan menggunakan Analytic Hierarchy Process (AHP) melalui matriks perbandingan berpasangan dan dihitung menggunakan pendekatan *geometric mean* (Saaty, 1987). Konsistensi penilaian dievaluasi menggunakan *consistency ratio* (CR). Pada konfigurasi bobot yang digunakan dalam penelitian ini, nilai CR memenuhi batas kelayakan (CR(perf)=0,030 dan CR(sec)=0,063). Koefisien fusion juga dibuat seimbang dengan $\alpha=0,5$ dan $\beta=0,5$. Ringkasan bobot yang digunakan adalah sebagai berikut:

- Bobot performa: L=0,369; J=0,105; Ploss=0,303; E=0,163; C=0,060.
- Bobot keamanan/sumber daya: Pr=0,654; K=0,204; R=0,096; Co=0,046.

## 3.8 Output Analisis

Output analisis terdiri dari tabel ringkasan Scenario 2, tabel ringkasan Scenario 1, tabel overhead reconnect, dan ranking QERS. Selain tabel, hasil juga dapat divisualisasikan menggunakan grafik batang untuk speed-up PIE, box plot untuk distribusi latency dan energi, serta line plot untuk metrik per iterasi. Visualisasi tersebut ditempatkan pada Bab IV sesuai bagian pembahasan masing-masing agar pembaca dapat menghubungkan grafik dengan hasil yang sedang dibahas.
