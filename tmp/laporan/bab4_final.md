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

Pada tahap analisis statistik, iterasi awal dibuang (*warmup drop*) untuk mengurangi bias kondisi *cold-start*. Pada pipeline analisis yang digunakan dalam penelitian ini, sampel dengan `iter < 5` tidak disertakan sehingga statistik p50/p95 dihitung dari 95 sampel per konfigurasi. Ringkasan kualitas data setelah *warmup drop* ditunjukkan pada Tabel 4.3.

Tabel 4.3. Ringkasan kualitas data Scenario 1 (setelah *warmup drop* `iter < 5`).

| variant     | pie    | mode    | n_total | n_success | packet_loss_rate | ss_valid_rate_all | ss_valid_rate_success |
| ----------- | ------ | ------- | ------- | --------- | ---------------- | ----------------- | --------------------- |
| ML-KEM-512  | pieoff | reconn  | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-512  | pieoff | session | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-512  | pieon  | reconn  | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-512  | pieon  | session | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-768  | pieoff | reconn  | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-768  | pieoff | session | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-768  | pieon  | reconn  | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-768  | pieon  | session | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-1024 | pieoff | reconn  | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-1024 | pieoff | session | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-1024 | pieon  | reconn  | 95      | 95        | 0                | 1                 | 1                     |
| ML-KEM-1024 | pieon  | session | 95      | 95        | 0                | 1                 | 1                     |

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
| ML-KEM-512  | KeyGen  | 7.850,0  | 7.579,0  | -3,45% | 1.255.693 | 1.212.297 | -3,46% | 2.008,0 | 2.019,0 | +0,55%  |
| ML-KEM-512  | Encaps  | 9.312,0  | 8.926,0  | -4,15% | 1.489.767 | 1.427.525 | -4,18% | 2.554,0 | 2.499,0 | -2,15%  |
| ML-KEM-512  | Decaps  | 11.329,0 | 10.769,0 | -4,94% | 1.812.454 | 1.722.814 | -4,95% | 3.095,0 | 3.066,0 | -0,94%  |
| ML-KEM-768  | KeyGen  | 12.451,0 | 11.957,0 | -3,97% | 1.991.664 | 1.912.848 | -3,96% | 3.634,0 | 3.196,0 | -12,05% |
| ML-KEM-768  | Encaps  | 14.681,0 | 14.026,0 | -4,46% | 2.348.736 | 2.243.433 | -4,48% | 4.287,0 | 3.822,0 | -10,85% |
| ML-KEM-768  | Decaps  | 17.345,0 | 16.487,0 | -4,95% | 2.775.160 | 2.637.772 | -4,95% | 4.934,0 | 4.736,0 | -4,01%  |
| ML-KEM-1024 | KeyGen  | 19.001,0 | 18.239,0 | -4,01% | 3.040.048 | 2.918.012 | -4,01% | 5.639,0 | 5.000,0 | -11,33% |
| ML-KEM-1024 | Encaps  | 21.695,0 | 20.742,0 | -4,39% | 3.471.041 | 3.318.460 | -4,40% | 6.145,0 | 5.996,0 | -2,42%  |
| ML-KEM-1024 | Decaps  | 25.092,0 | 23.790,0 | -5,19% | 4.014.391 | 3.806.091 | -5,19% | 7.344,0 | 6.753,0 | -8,05%  |

Gambar 4.2. Grafik batang persentase penurunan waktu eksekusi Scenario 2.

Keterangan: Gambar 4.2 perlu menampilkan sumbu X berupa kombinasi varian dan operasi. Sumbu Y berisi persentase penurunan waktu akibat PIE. Grafik ini dibuat dari kolom Δ Time pada Tabel 4.4.

Berdasarkan Tabel 4.4, penurunan waktu berada pada rentang 3,45% hingga 5,19%. Penurunan terbesar terjadi pada Decaps ML-KEM-1024. Hasil ini sejalan dengan rancangan optimasi karena Decaps banyak melibatkan operasi polinomial yang berkaitan dengan NTT. Dengan demikian, Scenario 2 membuktikan bahwa PIE efektif untuk mempercepat komputasi ML-KEM pada ESP32-S3.

Hasil energi tidak selalu mengikuti tren waktu dan siklus CPU. Beberapa operasi mengalami penurunan energi, tetapi beberapa operasi lain mengalami kenaikan energi. Perbedaan ini menunjukkan bahwa pengukuran energi sistem dipengaruhi oleh durasi operasi, kondisi peripheral, dan variasi pembacaan sensor. Oleh karena itu, waktu eksekusi dan siklus CPU menjadi indikator utama efek PIE terhadap komputasi, sedangkan energi dibaca sebagai konsumsi fisik sistem.

## 4.4 Hasil Pengujian Scenario 1

Scenario 1 digunakan untuk mengevaluasi ML-KEM pada komunikasi MQTT end-to-end. Pada skenario ini, hasil tidak hanya dipengaruhi oleh komputasi ML-KEM. Hasil juga dipengaruhi oleh Wi-Fi, broker MQTT, subscriber, fragmentasi payload, dan mode koneksi. Oleh karena itu, interpretasi Scenario 1 perlu dibedakan dari Scenario 2.

Tabel 4.5. Ringkasan median Scenario 1.

| Varian | PIE | Mode | Loss | KeyGen (µs) | Decaps (µs) | Encaps (µs) | Handshake (ms) | Total Latency (s) | Energy (mJ) | Total Energy (J) | CPU (%) | Jitter (ms) | RSSI |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| ML-KEM-512  | pieoff | reconnect | 0 | 8.475  | 12.579 | 997   | 215,6 | 12,178 | 83,9 | 2,979 | 9,4  | 271,7 | -24 |
| ML-KEM-512  | pieoff | session   | 0 | 8.007  | 12.412 | 998   | 246,8 | 0,247  | 94,3 | 0,094 | 8,3  | 3,1   | -28 |
| ML-KEM-512  | pieon  | reconnect | 0 | 8.168  | 12.106 | 1.004 | 236,6 | 16,199 | 87,5 | 3,880 | 8,6  | 268,6 | -25 |
| ML-KEM-512  | pieon  | session   | 0 | 7.732  | 11.328 | 998   | 246,5 | 0,246  | 93,0 | 0,093 | 7,8  | 3,4   | -34 |
| ML-KEM-768  | pieoff | reconnect | 0 | 13.291 | 18.426 | 1.482 | 198,2 | 11,196 | 92,5 | 2,656 | 16,0 | 1,1   | -30 |
| ML-KEM-768  | pieoff | session   | 0 | 12.652 | 18.442 | 1.486 | 247,3 | 0,247  | 96,0 | 0,096 | 12,6 | 4,0   | -28 |
| ML-KEM-768  | pieon  | reconnect | 0 | 12.811 | 17.597 | 1.494 | 212,3 | 12,717 | 97,5 | 3,122 | 14,3 | 331,9 | -24 |
| ML-KEM-768  | pieon  | session   | 0 | 12.125 | 17.548 | 1.443 | 247,3 | 0,247  | 96,0 | 0,096 | 12,0 | 5,6   | -29 |
| ML-KEM-1024 | pieoff | reconnect | 0 | 19.840 | 25.637 | 2.080 | 115,7 | 11,859 | 65,9 | 2,902 | 39,4 | 283,0 | -25 |
| ML-KEM-1024 | pieoff | session   | 0 | 20.233 | 26.183 | 2.064 | 85,3  | 0,085  | 52,3 | 0,052 | 54,2 | 10,0  | -28 |
| ML-KEM-1024 | pieon  | reconnect | 0 | 19.052 | 24.228 | 2.073 | 114,1 | 11,842 | 65,1 | 2,901 | 38,2 | 201,9 | -24 |
| ML-KEM-1024 | pieon  | session   | 0 | 19.444 | 24.301 | 2.037 | 68,9  | 0,069  | 42,8 | 0,043 | 63,6 | 2,6   | -23 |

Seluruh konfigurasi memiliki packet loss nol. Kondisi ini menunjukkan bahwa proses pertukaran public key dan ciphertext berhasil pada semua varian. Hasil tersebut juga menunjukkan bahwa data latency, energi, dan QERS tidak terdistorsi oleh handshake yang gagal.

Pada metrik komputasi ESP32-S3, PIE menurunkan waktu KeyGen dan Decaps pada sebagian besar perbandingan yang sepadan. Penurunan ini konsisten dengan hasil Scenario 2. Namun, metrik handshake end-to-end tidak selalu turun ketika PIE diaktifkan. Perbedaan tersebut terjadi karena handshake MQTT dipengaruhi oleh jaringan, broker, subscriber, dan timing pengiriman pesan.

Tabel 4.6. Overhead reconnect terhadap session.

| Varian | PIE | Overhead latency | Overhead energy | Median reconnect time | Median reconnect energy |
|---|---|---:|---:|---:|---:|
| ML-KEM-512  | pieoff | 11,931 s | 2,885 J | 11,573 s | 2,789 J |
| ML-KEM-512  | pieon  | 15,953 s | 3,787 J | 16,007 s | 3,789 J |
| ML-KEM-768  | pieoff | 10,949 s | 2,560 J | 10,998 s | 2,562 J |
| ML-KEM-768  | pieon  | 12,469 s | 3,026 J | 11,729 s | 2,815 J |
| ML-KEM-1024 | pieoff | 11,773 s | 2,850 J | 11,649 s | 2,771 J |
| ML-KEM-1024 | pieon  | 11,773 s | 2,859 J | 11,703 s | 2,804 J |

Gambar 4.3. Box plot distribusi handshake latency Scenario 1.

Keterangan: Gambar 4.3 perlu menampilkan distribusi `handshake_us` untuk 12 konfigurasi. Grafik ini digunakan untuk melihat median, sebaran, dan potensi outlier pada setiap varian, status PIE, dan mode koneksi.

Gambar 4.4. Line plot reconnect time per iterasi.

Keterangan: Gambar 4.4 perlu menampilkan `reconnect_us` terhadap nomor iterasi untuk konfigurasi mode reconnect. Grafik ini digunakan untuk menunjukkan stabilitas atau spike reconnect selama 100 iterasi.

Mode session jauh lebih efisien dibandingkan mode reconnect. Mode reconnect menambahkan sekitar 10,95 hingga 15,95 detik latency per siklus. Mode reconnect juga menambahkan sekitar 2,56 hingga 3,79 J energi per siklus. Dengan demikian, reconnect merupakan komponen biaya yang lebih dominan daripada komputasi ML-KEM.

## 4.5 Perhitungan Quantum Encryption Resilience Score

QERS dihitung menggunakan data Scenario 1 karena skenario tersebut merepresentasikan komunikasi end-to-end. Perhitungan QERS pada penelitian ini mengikuti pendekatan **QERS fusion** yang memisahkan penalti performa ($P$) dan benefit keamanan/sumber daya ($S$) (Rassekhnia, 2026). Metrik penalti performa mencakup total latency ($L$), jitter ($J$), packet loss rate ($P_{loss}$), total energy ($E$), dan CPU utilization ($C$). Metrik benefit keamanan/sumber daya mencakup proven resistance level ($Pr$), key size exchanged ($K=pub\_bytes+ct\_bytes$), RSSI ($R$), dan crypto overhead bytes ($Co=overhead\_bytes$).

Bobot metrik ditentukan menggunakan Analytic Hierarchy Process (AHP) melalui matriks perbandingan berpasangan dan diuji konsistensinya dengan *consistency ratio* (Saaty, 1987). Koefisien fusion digunakan seimbang ($\alpha=0,5$ dan $\beta=0,5$). Bobot yang digunakan pada penelitian ini adalah: $w_L=0,369$, $w_J=0,105$, $w_{Ploss}=0,303$, $w_E=0,163$, $w_C=0,060$, serta $w_{Pr}=0,654$, $w_K=0,204$, $w_R=0,096$, $w_{Co}=0,046$ dengan CR(perf)=0,030 dan CR(sec)=0,063.

Tabel 4.7. Ranking QERS Fusion.

| Rank | Varian | PIE | Mode | QERS | P | S | Total Latency (s) | Total Energy (J) | CPU (%) | Jitter (ms) |
|---:|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 1  | ML-KEM-1024 | pieon  | session | 94,710 | 6,022  | 95,443 | 0,069  | 0,043 | 63,6 | 2,6   |
| 2  | ML-KEM-1024 | pieoff | session | 92,886 | 5,330  | 91,101 | 0,085  | 0,052 | 54,2 | 10,0  |
| 3  | ML-KEM-1024 | pieon  | reconnect | 72,938 | 48,698 | 94,574 | 11,842 | 2,901 | 38,2 | 201,9 |
| 4  | ML-KEM-768  | pieoff | session | 72,932 | 1,249  | 47,112 | 0,247  | 0,096 | 12,6 | 4,0   |
| 5  | ML-KEM-768  | pieon  | session | 72,507 | 1,230  | 46,244 | 0,247  | 0,096 | 12,0 | 5,6   |
| 6  | ML-KEM-1024 | pieoff | reconnect | 71,135 | 51,436 | 93,706 | 11,859 | 2,902 | 39,4 | 283,0 |
| 7  | ML-KEM-768  | pieoff | reconnect | 53,971 | 37,434 | 45,376 | 11,196 | 2,656 | 16,0 | 1,1   |
| 8  | ML-KEM-512  | pieoff | session | 52,233 | 0,744  | 5,209  | 0,247  | 0,094 | 8,3  | 3,1   |
| 9  | ML-KEM-512  | pieon  | session | 49,655 | 0,691  | 0,000  | 0,246  | 0,093 | 7,8  | 3,4   |
| 10 | ML-KEM-768  | pieon  | reconnect | 48,693 | 53,199 | 50,585 | 12,717 | 3,122 | 14,3 | 331,9 |
| 11 | ML-KEM-512  | pieoff | reconnect | 29,881 | 48,919 | 8,682  | 12,178 | 2,979 | 9,4  | 271,7 |
| 12 | ML-KEM-512  | pieon  | reconnect | 23,027 | 61,760 | 7,814  | 16,199 | 3,880 | 8,6  | 268,6 |

Gambar 4.5. Grafik batang ranking QERS Fusion.

Keterangan: Gambar 4.5 perlu menampilkan skor QERS untuk 12 konfigurasi. Urutan grafik mengikuti ranking pada Tabel 4.7 agar pembaca dapat melihat jarak skor antar-konfigurasi.

Hasil QERS (global normalization) menunjukkan bahwa konfigurasi dengan skor tertinggi adalah **ML-KEM-1024 pieon session** dengan QERS 94,710, diikuti **ML-KEM-1024 pieoff session** dengan QERS 92,886. Skor tinggi pada varian 1024 terutama dipengaruhi oleh komponen benefit keamanan ($Pr$ dan $K$) pada QERS fusion. Untuk pembacaan yang lebih operasional, ranking dapat juga dilihat terpisah per-mode (session vs reconnect) sehingga perbandingan dilakukan pada konteks use-case yang sama.

## 4.6 Pembahasan Hasil Penelitian

Hasil Scenario 2 menunjukkan bahwa PIE memberikan manfaat yang konsisten pada waktu eksekusi dan siklus CPU. Penurunan waktu pada seluruh operasi membuktikan bahwa jalur vektor ESP32-S3 berhasil mempercepat operasi polinomial ML-KEM. Namun, konsumsi energi tidak selalu menurun. Temuan ini menunjukkan bahwa optimasi komputasi dan efisiensi energi perlu dianalisis sebagai dua aspek yang berbeda.

Hasil Scenario 1 menunjukkan bahwa manfaat PIE terhadap komputasi ESP32-S3 tetap terlihat pada KeyGen dan Decaps. Akan tetapi, efek tersebut tidak selalu tampak pada handshake end-to-end. Hal ini terjadi karena handshake MQTT dipengaruhi oleh proses publish, broker forwarding, subscriber Encaps, transmisi ciphertext, dan scheduling jaringan. Dengan demikian, percepatan pada level instruksi tidak selalu langsung menjadi percepatan pada level sistem.

Perbandingan session dan reconnect menunjukkan bahwa biaya koneksi ulang sangat dominan. Mode reconnect menambahkan latency dan energi yang jauh lebih besar daripada biaya komputasi ML-KEM. Oleh karena itu, mode session lebih sesuai untuk operasi IoT periodik yang membutuhkan efisiensi. Mode reconnect tetap penting sebagai skenario worst-case karena menunjukkan biaya sistem ketika koneksi harus dibangun ulang pada setiap siklus.

Perbandingan antarvarian menunjukkan trade-off antara keamanan dan beban sistem. ML-KEM-512 memiliki beban CPU rendah, tetapi level keamanannya lebih rendah. ML-KEM-1024 memiliki level keamanan lebih tinggi, tetapi CPU utilization jauh lebih besar. ML-KEM-768 berada di tengah kedua varian tersebut sehingga menghasilkan keseimbangan yang lebih baik pada QERS.

Berdasarkan seluruh hasil, konfigurasi dengan skor QERS tertinggi (menggunakan bobot AHP yang dipilih) adalah **ML-KEM-1024 pieon session**. Jika prioritas utama adalah *security-first* dan ranking QERS, maka ML-KEM-1024 (session) menjadi rekomendasi utama. Namun, jika prioritas utama adalah *performance-first* pada mode session (latency/energi lebih rendah dan CPU lebih ringan), konfigurasi ML-KEM-768 session dapat menjadi alternatif yang lebih efisien. Kesimpulan ini menunjukkan bahwa keputusan konfigurasi perlu mempertimbangkan tujuan sistem, bukan hanya satu metrik tunggal.

Penelitian ini memiliki beberapa keterbatasan. Pengujian dilakukan pada jaringan lokal sehingga hasilnya belum mewakili jaringan internet publik. Pengukuran energi menggunakan INA219 membaca konsumsi sistem secara keseluruhan sehingga tidak memisahkan konsumsi CPU dan Wi-Fi. Optimasi PIE juga masih difokuskan pada jalur operasi polinomial sehingga masih terdapat ruang pengembangan pada bagian lain seperti sampling, hashing, dan tata letak memori.
