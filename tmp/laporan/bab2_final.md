# BAB II
# TINJAUAN PUSTAKA

## 2.1 Dasar Teori

### 2.1.1 Internet of Things dan Protokol MQTT

Internet of Things (IoT) merupakan paradigma jaringan yang memungkinkan perangkat fisik mengumpulkan, mengirimkan, dan memproses data melalui koneksi internet atau jaringan lokal (Kumar et al., 2019). Perangkat IoT umumnya bekerja dengan sumber daya terbatas sehingga rancangan keamanan harus mempertimbangkan daya, memori, dan kemampuan komputasi (Fitzgibbon & Ottaviani, 2024). Keterbatasan tersebut membuat pemilihan protokol komunikasi dan algoritma kriptografi menjadi bagian penting dari desain sistem IoT.

Message Queuing Telemetry Transport (MQTT) adalah protokol komunikasi berbasis pola *publish/subscribe* yang menggunakan broker sebagai perantara pesan (Banks et al., 2019). Pola ini memisahkan pengirim dan penerima sehingga perangkat IoT tidak harus berkomunikasi langsung satu sama lain. MQTT v5.0 menyediakan mekanisme protokol yang lebih lengkap melalui reason code, property, dan pengaturan koneksi yang lebih eksplisit (Banks et al., 2019). Karakteristik tersebut membuat MQTT relevan untuk mengevaluasi pertukaran material kunci pada sistem IoT.

Penelitian ini menggunakan MQTT v5.0 dengan QoS 1. QoS 1 memberikan jaminan *at-least-once delivery* melalui mekanisme acknowledgment (Banks et al., 2019). QoS 0 tidak digunakan karena tidak menjamin pesan diterima oleh subscriber. QoS 2 tidak dipilih karena prosedur konfirmasinya lebih panjang dan dapat menambah overhead pada pengukuran handshake.

Pada QoS 1, pengirim wajib memperlakukan paket *PUBLISH* sebagai “belum diakui” sampai menerima paket *PUBACK* yang sesuai. Mekanisme ini menjadi dasar pemilihan QoS 1 pada penelitian ini karena memungkinkan pengukuran handshake yang reliabel tanpa kompleksitas tambahan QoS 2 (Banks et al., 2019).

Gambar 2.1. Arsitektur *publish/subscribe* MQTT untuk pertukaran material ML-KEM.

Keterangan: Gambar 2.1 perlu menampilkan ESP32-S3 sebagai *publisher* public key, Mosquitto broker sebagai perantara, dan Python subscriber sebagai pihak yang menjalankan Encaps. Gambar juga perlu menunjukkan alur balasan ciphertext dari subscriber ke ESP32-S3 melalui broker.

### 2.1.2 Kriptografi Pasca-Kuantum

Post-Quantum Cryptography (PQC) adalah kelompok algoritma kriptografi yang dirancang untuk tetap aman terhadap serangan komputer kuantum (Almutairi & Sheldon, 2025). Ancaman utama terhadap skema kunci publik klasik berasal dari algoritma Shor yang dapat memecahkan masalah faktorisasi integer dan logaritma diskrit secara efisien pada komputer kuantum berskala besar (Almutairi & Sheldon, 2025). Karena itu, PQC menjadi arah transisi penting untuk melindungi komunikasi digital jangka panjang.

PQC terdiri dari beberapa keluarga algoritma, seperti lattice-based, code-based, hash-based, multivariate, dan isogeny-based cryptography (Asif, 2021). Lattice-based cryptography menjadi salah satu pendekatan yang menonjol karena menawarkan keseimbangan antara performa dan keamanan (Asif, 2021). Namun, penerapan PQC pada perangkat IoT tetap menimbulkan tantangan karena ukuran parameter dan beban komputasi dapat meningkat.

### 2.1.3 Standar ML-KEM FIPS 203

Module-Lattice-Based Key-Encapsulation Mechanism (ML-KEM) adalah standar mekanisme enkapsulasi kunci yang ditetapkan dalam FIPS 203 (NIST, 2024). ML-KEM berasal dari CRYSTALS-Kyber yang menggunakan masalah Module Learning With Errors sebagai dasar keamanan (Bos et al., 2018). Sebagai KEM, ML-KEM memiliki tiga operasi utama, yaitu KeyGen, Encaps, dan Decaps.

KeyGen menghasilkan pasangan public key dan secret key. Encaps menggunakan public key untuk menghasilkan ciphertext dan shared secret. Decaps menggunakan secret key dan ciphertext untuk memulihkan shared secret. Tiga operasi tersebut membentuk alur dasar pertukaran kunci yang dapat digunakan sebelum komunikasi simetris dilakukan.

ML-KEM memiliki tiga parameter set utama, yaitu ML-KEM-512, ML-KEM-768, dan ML-KEM-1024 (NIST, 2024). Ketiga varian tersebut umumnya dipetakan pada level keamanan NIST 1, 3, dan 5 (NIST, 2024). Peningkatan level keamanan diikuti peningkatan ukuran public key, secret key, ciphertext, dan beban komputasi. Hubungan parameter tersebut ditunjukkan pada Tabel 2.1.

Tabel 2.1. Parameter utama ML-KEM berdasarkan FIPS 203.

| Parameter | ML-KEM-512 | ML-KEM-768 | ML-KEM-1024 | Implikasi |
|---|---:|---:|---:|---|
| Security level | 1 | 3 | 5 | Level keamanan semakin tinggi. |
| Module rank `k` | 2 | 3 | 4 | Dimensi masalah lattice meningkat. |
| Public key | 800 B | 1.184 B | 1.568 B | Payload MQTT dari ESP32-S3 meningkat. |
| Ciphertext | 768 B | 1.088 B | 1.568 B | Payload MQTT menuju ESP32-S3 meningkat. |
| Shared secret | 32 B | 32 B | 32 B | Ukuran kunci bersama tetap. |

### 2.1.4 Number Theoretic Transform

Number Theoretic Transform (NTT) adalah transformasi yang digunakan untuk mempercepat perkalian polinomial pada skema lattice-based (Sonbul et al., 2025). Perkalian polinomial secara langsung membutuhkan biaya komputasi besar. NTT mengubah polinomial ke domain transformasi sehingga perkalian dapat dilakukan melalui operasi point-wise multiplication. Karena operasi polinomial muncul berulang pada ML-KEM, NTT menjadi salah satu bagian yang menentukan performa implementasi.

Operasi inti NTT menggunakan pola *butterfly* yang melibatkan penjumlahan, pengurangan, perkalian dengan konstanta zeta, dan reduksi modular. Pola tersebut cocok untuk optimasi vektor karena beberapa koefisien dapat diproses secara paralel. Pada penelitian ini, karakteristik tersebut menjadi dasar pemanfaatan PIE pada ESP32-S3.

Gambar 2.2. Alur *butterfly* NTT dan inverse NTT pada ML-KEM.

Keterangan: Gambar 2.2 perlu dibuat sebagai diagram konseptual yang menunjukkan input dua koefisien, perkalian dengan zeta, operasi penjumlahan, operasi pengurangan, dan reduksi modular. Gambar ini dapat dibuat sendiri berdasarkan deskripsi algoritmik FIPS 203 dan literatur NTT, bukan mengambil langsung gambar dari jurnal.

### 2.1.5 ESP32-S3 dan Processor Instruction Extensions

ESP32-S3 menggunakan prosesor Xtensa LX7 dan mendukung Processor Instruction Extensions untuk operasi vektor 128-bit (Espressif Systems, 2025). Register vektor pada ESP32-S3 dapat memuat beberapa elemen data kecil dalam satu operasi. Fitur ini relevan untuk ML-KEM karena banyak operasi polinomial menggunakan koefisien 16-bit.

Pada level arsitektur, PIE menyediakan register 128-bit dan instruksi SIMD yang memungkinkan beberapa elemen 8/16/32-bit diproses secara paralel dalam satu instruksi. Kemampuan ini sesuai dengan karakter koefisien ML-KEM (umumnya 16-bit) sehingga bagian komputasi yang dominan seperti NTT, inverse NTT, dan perkalian polinomial berpotensi memperoleh percepatan (Espressif Systems, 2025).

Pada penelitian ini, PIE digunakan untuk mempercepat sebagian jalur NTT, inverse NTT, dan operasi polynomial base multiplication. Implementasi tanpa PIE menjalankan operasi menggunakan loop skalar. Implementasi dengan PIE memproses beberapa koefisien secara paralel melalui instruksi vektor. Perbedaan ini diharapkan mengurangi siklus CPU pada bagian komputasi yang sering dijalankan.

Gambar 2.3. Perbandingan alur skalar dan alur PIE pada operasi polinomial ML-KEM.

Keterangan: Gambar 2.3 perlu menampilkan dua jalur. Jalur pertama menunjukkan pemrosesan koefisien satu per satu menggunakan register umum. Jalur kedua menunjukkan delapan koefisien `int16_t` yang dimuat ke register vektor 128-bit dan diproses secara paralel.

### 2.1.6 Sensor INA219 dan Pengukuran Energi

Evaluasi perangkat IoT tidak cukup hanya menggunakan waktu eksekusi karena perangkat sering dibatasi oleh sumber energi. INA219 dapat digunakan untuk membaca tegangan dan arus pada jalur daya. Energi kemudian dihitung dari akumulasi daya terhadap waktu selama operasi berlangsung. Pada penelitian ini, INA219 dipasang secara in-line pada jalur daya ESP32-S3 agar konsumsi energi sistem dapat diamati selama pengujian.

Pengukuran energi dilakukan pada dua level. Pada Scenario 2, energi diukur untuk operasi KeyGen, Encaps, dan Decaps. Pada Scenario 1, energi diukur untuk satu siklus handshake MQTT. Pada mode reconnect, energi proses koneksi ulang juga dicatat agar biaya koneksi dapat dipisahkan dari biaya handshake.

### 2.1.7 Quantum Encryption Resilience Score

Quantum Encryption Resilience Score (QERS) adalah kerangka evaluasi multi-kriteria untuk menilai kesiapan sistem dalam menerapkan kriptografi pasca-kuantum (Rassekhnia, 2026). QERS menggabungkan metrik yang memiliki satuan berbeda, seperti latency, energi, CPU utilization, packet loss, jitter, ukuran kunci, RSSI, dan tingkat resistensi kriptografi. Penggabungan tersebut membantu peneliti melihat trade-off sistem secara lebih menyeluruh.

Perhitungan QERS membutuhkan normalisasi karena setiap metrik memiliki skala berbeda. Metrik cost seperti latency dan energi diharapkan bernilai rendah. Metrik benefit seperti proven resistance dan ketersediaan sumber daya diharapkan bernilai tinggi. Setelah normalisasi, metrik diberi bobot sesuai prioritas sistem.

Penetapan bobot dapat menggunakan prinsip Multi-Criteria Decision Analysis. Analytic Hierarchy Process menjadi salah satu pendekatan yang sering digunakan untuk menyusun prioritas antar-kriteria (Saaty, 1987). Dalam penelitian ini, bobot QERS digunakan untuk menyeimbangkan kebutuhan performa, energi, reliabilitas, sumber daya, dan tingkat keamanan.

Rumus 2.1. Normalisasi metrik cost dan benefit untuk QERS.

$$X_{norm} = MS \cdot \dfrac{X - X_{min}}{X_{max} - X_{min}},\quad MS = 100$$

Keterangan: Normalisasi QERS menggunakan skema min–max dengan skor maksimum $MS=100$ (Rassekhnia, 2026). Untuk **cost criteria** (mis. latency dan energy), nilai penalti menggunakan $X_{norm}$ lalu skor akhir dibentuk dengan $MS - P$ agar nilai lebih kecil menjadi lebih baik. Untuk **benefit criteria** (mis. proven resistance dan RSSI), nilai menggunakan $X_{norm}$ secara langsung. Pada QERS fusion, penalti performa $P$ dan benefit keamanan/sumber daya $S$ digabungkan menjadi $QERS_{fusion}=\alpha(MS-P)+\beta S$ dengan $\alpha+\beta=1$ (Rassekhnia, 2026).

## 2.2 Penelitian Terkait

Penelitian terkait digunakan untuk menunjukkan posisi penelitian ini terhadap kajian sebelumnya. Beberapa penelitian membahas implementasi Kyber atau ML-KEM pada perangkat terbatas. Beberapa penelitian lain membahas penggunaan PQC pada MQTT dan metode evaluasi multi-kriteria. Ringkasan penelitian terkait ditunjukkan pada Tabel 2.2.

Tabel 2.2. Ringkasan penelitian terkait.

| No | Penelitian | Fokus | Keterkaitan dengan penelitian ini |
|---:|---|---|---|
| 1 | Segatz dan Hafiz (2025) | Implementasi CRYSTALS-Kyber pada ESP32 | Menunjukkan bahwa optimasi arsitektur dapat mempercepat Kyber pada platform ESP32. |
| 2 | Malina et al. (2024) | MQTT tahan kuantum | Menjadi dasar bahwa PQC perlu diuji pada protokol MQTT, bukan hanya secara lokal. |
| 3 | Sonbul et al. (2025) | Akselerasi NTT untuk CRYSTALS-Kyber | Menguatkan alasan menjadikan NTT sebagai target optimasi performa. |
| 4 | Ji et al. (2024) | Optimasi CRYSTALS pada RISC-V | Mendukung pendekatan optimasi level instruksi meskipun arsitektur berbeda. |
| 5 | Chhetri (2026) | Benchmark ML-KEM dan ML-DSA pada RP2040 | Menjadi pembanding metodologi pengukuran performa, memori, dan energi pada mikrokontroler. |
| 6 | Rassekhnia (2026) | QERS untuk MQTT, HTTP, dan HTTPS | Menjadi dasar penggunaan skor multi-kriteria untuk menilai kesiapan sistem pasca-kuantum. |
| 7 | Fitzgibbon dan Ottaviani (2024) | Benchmark PQC pada perangkat terbatas | Mendukung pentingnya evaluasi performa dan sumber daya pada constrained device. |
| 8 | Banks et al. (2019) | Spesifikasi MQTT v5.0 | Menjadi rujukan formal untuk protokol MQTT v5.0 dan QoS. |

Berdasarkan penelitian terkait tersebut, penelitian ini memiliki posisi pada evaluasi empiris implementasi standar ML-KEM di ESP32-S3 dengan membandingkan konfigurasi tanpa PIE dan dengan PIE. Perbedaan utama penelitian ini terletak pada pemisahan evaluasi menjadi benchmark komputasi lokal dan handshake MQTT end-to-end. Pemisahan tersebut memungkinkan analisis yang lebih jelas antara efek optimasi instruksi dan efek komunikasi jaringan. Selain itu, penggunaan QERS membantu menilai konfigurasi terbaik berdasarkan trade-off, bukan hanya berdasarkan waktu eksekusi tercepat.
