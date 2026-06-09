# BAB IV
# HASIL DAN PEMBAHASAN

Bab ini menyajikan hasil pengujian implementasi standar ML-KEM pada ESP32-S3. Pembahasan dimulai dari pengumpulan data, praproses data, hasil benchmark komputasi lokal, hasil handshake MQTT, perhitungan QERS, dan interpretasi akhir. Urutan tersebut digunakan agar pembaca dapat memahami kualitas data sebelum melihat analisis performa dan rekomendasi konfigurasi.

## 4.1 Pengumpulan Data Hasil Pengujian

Pengumpulan data dilakukan melalui dua skenario. Scenario 2 digunakan untuk mengukur komputasi lokal ML-KEM pada ESP32-S3. Scenario 1 digunakan untuk mengukur performa ML-KEM ketika public key dan ciphertext dipertukarkan melalui MQTT v5.0. Pemisahan skenario ini penting karena performa komputasi lokal belum tentu sama dengan performa komunikasi end-to-end.

Scenario 2 menghasilkan enam file CSV. File tersebut berasal dari kombinasi tiga varian ML-KEM dan dua status PIE. Setiap file memuat hasil KeyGen, Encaps, dan Decaps sebanyak 100 iterasi per operasi. Metrik yang dicatat meliputi waktu eksekusi, siklus CPU, energi, heap, dan stack high-water mark.

Tabel 4.1. Matriks dataset Scenario 2.

| Varian | PIE | Operasi | Iterasi per operasi |
|---|---|---|---:|
| ML-KEM-512 | off/on | KeyGen, Encaps, Decaps | 100 |
| ML-KEM-768 | off/on | KeyGen, Encaps, Decaps | 100 |
| ML-KEM-1024 | off/on | KeyGen, Encaps, Decaps | 100 |

Scenario 1 menghasilkan 12 file joined CSV. File tersebut berasal dari kombinasi tiga varian ML-KEM, dua status PIE, dan dua mode koneksi. Pada skenario ini, ESP32-S3 menjalankan KeyGen dan Decaps, sedangkan subscriber Python menjalankan Encaps. Data subscriber kemudian digabung dengan data ESP32-S3 berdasarkan nomor iterasi.

Tabel 4.2. Matriks dataset Scenario 1.

| Faktor | Nilai | Jumlah kombinasi |
|---|---|---:|
| Varian ML-KEM | 512, 768, 1024 | 3 |
| Status PIE | pieoff, pieon | 2 |
| Mode koneksi | session, reconnect | 2 |
| Total konfigurasi | 3 × 2 × 2 | 12 |
| Iterasi per konfigurasi | 100 | 1.200 baris |

Gambar 4.1. Alur pembentukan dataset Scenario 1 dan Scenario 2.

Keterangan: Gambar 4.1 perlu menampilkan dua cabang. Cabang pertama menunjukkan Scenario 2 dari ESP32-S3 menuju CSV lokal. Cabang kedua menunjukkan Scenario 1 dari ESP32-S3, broker, subscriber, proses join CSV, dan CSV joined. Gambar ini dibuat sendiri berdasarkan alur eksperimen.

## 4.2 Praproses Data Hasil Pengujian

Praproses data dilakukan untuk memastikan hasil pengujian dapat dibandingkan secara adil. Langkah pertama adalah memeriksa struktur kolom pada setiap CSV. Langkah kedua adalah memeriksa jumlah iterasi. Langkah ketiga adalah memeriksa packet loss pada Scenario 1. Langkah keempat adalah menyiapkan metrik turunan seperti total latency dan total energy untuk mode reconnect.

Hasil validasi menunjukkan bahwa seluruh konfigurasi Scenario 1 memiliki 100 iterasi. Seluruh file joined juga memiliki nilai Encaps dari subscriber. Packet loss bernilai nol pada semua konfigurasi. Kondisi ini menunjukkan bahwa hasil Scenario 1 dapat dianalisis tanpa membuang baris akibat kegagalan handshake.

Tabel 4.3. Validasi data Scenario 1.

| Varian | PIE | Mode | Baris | Packet loss | Status |
|---|---|---|---:|---:|---|
| ML-KEM-512 | pieoff | reconnect | 100 | 0 | Valid |
| ML-KEM-512 | pieoff | session | 100 | 0 | Valid |
| ML-KEM-512 | pieon | reconnect | 100 | 0 | Valid |
| ML-KEM-512 | pieon | session | 100 | 0 | Valid |
| ML-KEM-768 | pieoff | reconnect | 100 | 0 | Valid |
| ML-KEM-768 | pieoff | session | 100 | 0 | Valid |
| ML-KEM-768 | pieon | reconnect | 100 | 0 | Valid |
| ML-KEM-768 | pieon | session | 100 | 0 | Valid |
| ML-KEM-1024 | pieoff | reconnect | 100 | 0 | Valid |
| ML-KEM-1024 | pieoff | session | 100 | 0 | Valid |
| ML-KEM-1024 | pieon | reconnect | 100 | 0 | Valid |
| ML-KEM-1024 | pieon | session | 100 | 0 | Valid |

Median digunakan sebagai statistik utama. Median dipilih karena metrik komunikasi dan energi dapat mengalami spike. Spike tersebut tidak dihapus secara otomatis karena dapat mencerminkan kondisi operasional jaringan lokal. Dengan pendekatan ini, hasil analisis tetap mewakili performa sistem selama pengujian.

Rumus 4.1. Speed-up PIE untuk metrik cost.

`speed-up = nilai_pieoff / nilai_pieon`

Rumus 4.2. Total latency mode reconnect.

`total_latency_us = reconnect_us + handshake_us`

Rumus 4.3. Total energy mode reconnect.

`total_energy_uj = reconnect_energy_uj + energy_uj`

## 4.3 Hasil Pengujian Scenario 2

Scenario 2 digunakan untuk mengisolasi efek PIE dari pengaruh jaringan. Hasil pengujian menunjukkan bahwa PIE menurunkan waktu eksekusi dan siklus CPU pada seluruh varian dan seluruh operasi. Penurunan ini menunjukkan bahwa jalur vektor pada ESP32-S3 berhasil mempercepat bagian komputasi ML-KEM.

Tabel 4.4. Perbandingan PIE dan No PIE pada Scenario 2.

| Varian | Operasi | Time No PIE (µs) | Time PIE (µs) | Δ Time | Cycles No PIE | Cycles PIE | Δ Cycles | Energy No PIE (µJ) | Energy PIE (µJ) | Δ Energy |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| ML-KEM-512 | KeyGen | 7.765,0 | 7.487,0 | -3,58% | 1.242.119 | 1.197.602 | -3,58% | 1.395,5 | 1.606,0 | +15,08% |
| ML-KEM-512 | Encaps | 9.211,5 | 8.840,0 | -4,03% | 1.473.624 | 1.414.228 | -4,03% | 1.456,5 | 1.676,5 | +15,10% |
| ML-KEM-512 | Decaps | 11.205,0 | 10.646,0 | -4,99% | 1.792.732 | 1.703.231 | -4,99% | 1.825,5 | 1.411,5 | -22,68% |
| ML-KEM-768 | KeyGen | 12.344,5 | 11.826,0 | -4,20% | 1.974.784 | 1.891.753 | -4,20% | 1.727,5 | 1.928,5 | +11,64% |
| ML-KEM-768 | Encaps | 14.520,0 | 13.874,5 | -4,45% | 2.322.944 | 2.219.601 | -4,45% | 2.848,0 | 2.584,5 | -9,25% |
| ML-KEM-768 | Decaps | 17.164,5 | 16.251,0 | -5,32% | 2.746.047 | 2.599.957 | -5,32% | 3.859,0 | 3.281,5 | -14,97% |
| ML-KEM-1024 | KeyGen | 18.721,0 | 17.993,0 | -3,89% | 2.995.247 | 2.878.482 | -3,90% | 2.891,0 | 3.808,0 | +31,72% |
| ML-KEM-1024 | Encaps | 21.423,0 | 20.485,0 | -4,38% | 3.427.424 | 3.277.282 | -4,38% | 5.593,5 | 4.606,5 | -17,65% |
| ML-KEM-1024 | Decaps | 24.759,0 | 23.513,0 | -5,03% | 3.960.992 | 3.761.773 | -5,03% | 4.797,5 | 5.422,0 | +13,02% |

Gambar 4.2. Grafik batang persentase penurunan waktu eksekusi Scenario 2.

Keterangan: Gambar 4.2 perlu menampilkan sumbu X berupa kombinasi varian dan operasi. Sumbu Y berisi persentase penurunan waktu akibat PIE. Grafik ini dibuat dari kolom Δ Time pada Tabel 4.4.

Berdasarkan Tabel 4.4, penurunan waktu berada pada rentang 3,58% hingga 5,32%. Penurunan terbesar terjadi pada Decaps ML-KEM-768. Hasil ini sejalan dengan rancangan optimasi karena Decaps banyak melibatkan operasi polinomial yang berkaitan dengan NTT. Dengan demikian, Scenario 2 membuktikan bahwa PIE efektif untuk mempercepat komputasi ML-KEM pada ESP32-S3.

Hasil energi tidak selalu mengikuti tren waktu dan siklus CPU. Beberapa operasi mengalami penurunan energi, tetapi beberapa operasi lain mengalami kenaikan energi. Perbedaan ini menunjukkan bahwa pengukuran energi sistem dipengaruhi oleh durasi operasi, kondisi peripheral, dan variasi pembacaan sensor. Oleh karena itu, waktu eksekusi dan siklus CPU menjadi indikator utama efek PIE terhadap komputasi, sedangkan energi dibaca sebagai konsumsi fisik sistem.

## 4.4 Hasil Pengujian Scenario 1

Scenario 1 digunakan untuk mengevaluasi ML-KEM pada komunikasi MQTT end-to-end. Pada skenario ini, hasil tidak hanya dipengaruhi oleh komputasi ML-KEM. Hasil juga dipengaruhi oleh Wi-Fi, broker MQTT, subscriber, fragmentasi payload, dan mode koneksi. Oleh karena itu, interpretasi Scenario 1 perlu dibedakan dari Scenario 2.

Tabel 4.5. Ringkasan median Scenario 1.

| Varian | PIE | Mode | Loss | KeyGen (µs) | Decaps (µs) | Encaps (µs) | Handshake (ms) | Total Latency (s) | Energy (mJ) | Total Energy (J) | CPU (%) | Jitter (ms) | RSSI |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| ML-KEM-512 | pieoff | reconnect | 0 | 8.238 | 12.360 | 1.026 | 332,9 | 16,91 | 117,5 | 3,831 | 6,1 | 445,7 | -30 |
| ML-KEM-512 | pieoff | session | 0 | 7.904 | 12.404 | 1.020 | 277,9 | 0,28 | 104,6 | 0,105 | 7,2 | 88,3 | -30 |
| ML-KEM-512 | pieon | reconnect | 0 | 7.968 | 11.910 | 1.026 | 373,1 | 16,95 | 126,5 | 3,847 | 5,2 | 263,8 | -29 |
| ML-KEM-512 | pieon | session | 0 | 7.588 | 11.891 | 1.023 | 317,6 | 0,32 | 108,5 | 0,109 | 6,2 | 202,3 | -29 |
| ML-KEM-768 | pieoff | reconnect | 0 | 13.038 | 18.242 | 1.516 | 190,4 | 16,19 | 84,0 | 3,520 | 16,4 | 0,4 | -33 |
| ML-KEM-768 | pieoff | session | 0 | 12.528 | 18.218 | 1.506 | 247,1 | 0,25 | 88,4 | 0,088 | 12,5 | 2,3 | -33 |
| ML-KEM-768 | pieon | reconnect | 0 | 12.350 | 17.379 | 1.502 | 189,4 | 16,19 | 83,0 | 3,521 | 15,7 | 0,3 | -33 |
| ML-KEM-768 | pieon | session | 0 | 12.021 | 17.401 | 1.500 | 247,3 | 0,25 | 89,2 | 0,089 | 11,9 | 2,5 | -34 |
| ML-KEM-1024 | pieoff | reconnect | 0 | 19.516 | 25.906 | 2.115 | 111,5 | 16,13 | 59,2 | 3,608 | 40,8 | 1,9 | -33 |
| ML-KEM-1024 | pieoff | session | 0 | 19.986 | 25.907 | 2.045 | 68,7 | 0,07 | 39,7 | 0,040 | 66,9 | 2,1 | -28 |
| ML-KEM-1024 | pieon | reconnect | 0 | 18.722 | 24.637 | 2.096 | 109,4 | 16,13 | 59,1 | 3,606 | 39,6 | 1,7 | -29 |
| ML-KEM-1024 | pieon | session | 0 | 19.052 | 24.656 | 2.036 | 66,2 | 0,07 | 39,1 | 0,039 | 65,9 | 1,7 | -28 |

Seluruh konfigurasi memiliki packet loss nol. Kondisi ini menunjukkan bahwa proses pertukaran public key dan ciphertext berhasil pada semua varian. Hasil tersebut juga menunjukkan bahwa data latency, energi, dan QERS tidak terdistorsi oleh handshake yang gagal.

Pada metrik komputasi ESP32-S3, PIE menurunkan waktu KeyGen dan Decaps pada sebagian besar perbandingan yang sepadan. Penurunan ini konsisten dengan hasil Scenario 2. Namun, metrik handshake end-to-end tidak selalu turun ketika PIE diaktifkan. Perbedaan tersebut terjadi karena handshake MQTT dipengaruhi oleh jaringan, broker, subscriber, dan timing pengiriman pesan.

Tabel 4.6. Overhead reconnect terhadap session.

| Varian | PIE | Overhead latency | Overhead energy | Median reconnect time | Median reconnect energy |
|---|---|---:|---:|---:|---:|
| ML-KEM-512 | pieoff | 16,63 s | 3,726 J | 16,57 s | 3,710 J |
| ML-KEM-512 | pieon | 16,63 s | 3,738 J | 16,47 s | 3,694 J |
| ML-KEM-768 | pieoff | 15,95 s | 3,432 J | 16,00 s | 3,437 J |
| ML-KEM-768 | pieon | 15,95 s | 3,432 J | 16,00 s | 3,437 J |
| ML-KEM-1024 | pieoff | 16,06 s | 3,568 J | 16,02 s | 3,548 J |
| ML-KEM-1024 | pieon | 16,06 s | 3,567 J | 16,02 s | 3,545 J |

Gambar 4.3. Box plot distribusi handshake latency Scenario 1.

Keterangan: Gambar 4.3 perlu menampilkan distribusi `handshake_us` untuk 12 konfigurasi. Grafik ini digunakan untuk melihat median, sebaran, dan potensi outlier pada setiap varian, status PIE, dan mode koneksi.

Gambar 4.4. Line plot reconnect time per iterasi.

Keterangan: Gambar 4.4 perlu menampilkan `reconnect_us` terhadap nomor iterasi untuk konfigurasi mode reconnect. Grafik ini digunakan untuk menunjukkan stabilitas atau spike reconnect selama 100 iterasi.

Mode session jauh lebih efisien dibandingkan mode reconnect. Mode reconnect menambahkan sekitar 15,95 hingga 16,63 detik latency per siklus. Mode reconnect juga menambahkan sekitar 3,43 hingga 3,74 J energi per siklus. Dengan demikian, reconnect merupakan komponen biaya yang lebih dominan daripada komputasi ML-KEM.

## 4.5 Perhitungan Quantum Encryption Resilience Score

QERS dihitung menggunakan data Scenario 1 karena skenario tersebut merepresentasikan komunikasi end-to-end. Metrik performa mencakup total latency, total energy, CPU utilization, packet loss, dan jitter. Metrik keamanan dan sumber daya mencakup proven resistance, ketersediaan heap, ukuran payload, dan RSSI. Seluruh metrik dinormalisasi sebelum digabungkan menjadi skor akhir.

Bobot performa terdiri dari latency 0,35, energy 0,25, CPU utilization 0,20, packet loss 0,10, dan jitter 0,10. Bobot keamanan dan sumber daya terdiri dari proven resistance 0,40, heap/resource availability 0,30, payload size 0,20, dan RSSI 0,10. Skor akhir menggunakan bobot fusion 0,70 untuk performa dan 0,30 untuk keamanan serta sumber daya. Bobot performa lebih besar karena penelitian ini menekankan kelayakan operasional ML-KEM pada ESP32-S3.

Tabel 4.7. Ranking QERS Fusion.

| Rank | Varian | PIE | Mode | QERS | P | S | Total Latency (s) | Total Energy (J) | CPU (%) | Jitter (ms) |
|---:|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 1 | ML-KEM-768 | pieoff | session | 85,72 | 96,89 | 59,65 | 0,25 | 0,088 | 12,5 | 2,3 |
| 2 | ML-KEM-768 | pieon | session | 85,34 | 97,07 | 57,98 | 0,25 | 0,089 | 11,9 | 2,5 |
| 3 | ML-KEM-512 | pieoff | session | 84,55 | 96,51 | 56,67 | 0,28 | 0,105 | 7,2 | 88,3 |
| 4 | ML-KEM-512 | pieon | session | 83,24 | 94,16 | 57,77 | 0,32 | 0,109 | 6,2 | 202,3 |
| 5 | ML-KEM-1024 | pieon | session | 76,21 | 80,29 | 66,69 | 0,07 | 0,039 | 65,9 | 1,7 |
| 6 | ML-KEM-1024 | pieoff | session | 75,98 | 79,95 | 66,72 | 0,07 | 0,040 | 66,9 | 2,1 |
| 7 | ML-KEM-768 | pieoff | reconnect | 44,48 | 40,08 | 54,77 | 16,19 | 3,520 | 16,4 | 0,4 |
| 8 | ML-KEM-768 | pieon | reconnect | 44,42 | 40,29 | 54,04 | 16,19 | 3,521 | 15,7 | 0,3 |
| 9 | ML-KEM-512 | pieon | reconnect | 40,98 | 34,08 | 57,09 | 16,95 | 3,847 | 5,2 | 263,8 |
| 10 | ML-KEM-512 | pieoff | reconnect | 37,72 | 29,86 | 56,04 | 16,91 | 3,831 | 6,1 | 445,7 |
| 11 | ML-KEM-1024 | pieon | reconnect | 36,96 | 32,09 | 48,33 | 16,13 | 3,606 | 39,6 | 1,7 |
| 12 | ML-KEM-1024 | pieoff | reconnect | 34,70 | 31,70 | 41,72 | 16,13 | 3,608 | 40,8 | 1,9 |

Gambar 4.5. Grafik batang ranking QERS Fusion.

Keterangan: Gambar 4.5 perlu menampilkan skor QERS untuk 12 konfigurasi. Urutan grafik mengikuti ranking pada Tabel 4.7 agar pembaca dapat melihat jarak skor antar-konfigurasi.

Hasil QERS menunjukkan bahwa ML-KEM-768 mode session menempati posisi terbaik. ML-KEM-768 pieoff session memperoleh skor 85,72. ML-KEM-768 pieon session memperoleh skor 85,34. Selisih kedua skor tersebut kecil sehingga keduanya dapat dianggap sebagai konfigurasi yang seimbang.

## 4.6 Pembahasan Hasil Penelitian

Hasil Scenario 2 menunjukkan bahwa PIE memberikan manfaat yang konsisten pada waktu eksekusi dan siklus CPU. Penurunan waktu pada seluruh operasi membuktikan bahwa jalur vektor ESP32-S3 berhasil mempercepat operasi polinomial ML-KEM. Namun, konsumsi energi tidak selalu menurun. Temuan ini menunjukkan bahwa optimasi komputasi dan efisiensi energi perlu dianalisis sebagai dua aspek yang berbeda.

Hasil Scenario 1 menunjukkan bahwa manfaat PIE terhadap komputasi ESP32-S3 tetap terlihat pada KeyGen dan Decaps. Akan tetapi, efek tersebut tidak selalu tampak pada handshake end-to-end. Hal ini terjadi karena handshake MQTT dipengaruhi oleh proses publish, broker forwarding, subscriber Encaps, transmisi ciphertext, dan scheduling jaringan. Dengan demikian, percepatan pada level instruksi tidak selalu langsung menjadi percepatan pada level sistem.

Perbandingan session dan reconnect menunjukkan bahwa biaya koneksi ulang sangat dominan. Mode reconnect menambahkan latency dan energi yang jauh lebih besar daripada biaya komputasi ML-KEM. Oleh karena itu, mode session lebih sesuai untuk operasi IoT periodik yang membutuhkan efisiensi. Mode reconnect tetap penting sebagai skenario worst-case karena menunjukkan biaya sistem ketika koneksi harus dibangun ulang pada setiap siklus.

Perbandingan antarvarian menunjukkan trade-off antara keamanan dan beban sistem. ML-KEM-512 memiliki beban CPU rendah, tetapi level keamanannya lebih rendah. ML-KEM-1024 memiliki level keamanan lebih tinggi, tetapi CPU utilization jauh lebih besar. ML-KEM-768 berada di tengah kedua varian tersebut sehingga menghasilkan keseimbangan yang lebih baik pada QERS.

Berdasarkan seluruh hasil, konfigurasi yang paling seimbang adalah ML-KEM-768 mode session. Jika prioritas utama adalah skor QERS, ML-KEM-768 pieoff session memiliki nilai tertinggi. Jika prioritas utama adalah pengurangan waktu komputasi pada ESP32-S3, ML-KEM-768 pieon session tetap layak dipilih. Kesimpulan ini menunjukkan bahwa keputusan konfigurasi perlu mempertimbangkan tujuan sistem, bukan hanya satu metrik performa.

Penelitian ini memiliki beberapa keterbatasan. Pengujian dilakukan pada jaringan lokal sehingga hasilnya belum mewakili jaringan internet publik. Pengukuran energi menggunakan INA219 membaca konsumsi sistem secara keseluruhan sehingga tidak memisahkan konsumsi CPU dan Wi-Fi. Optimasi PIE juga masih difokuskan pada jalur operasi polinomial sehingga masih terdapat ruang pengembangan pada bagian lain seperti sampling, hashing, dan tata letak memori.
