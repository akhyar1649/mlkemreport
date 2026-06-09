# Analisis Final Data Scenario 1 dan Scenario 2

Dokumen ini merangkum karakter data, praproses, hasil komparasi PIE vs No PIE, analisis mode `session` vs `reconn`, rancangan QERS, rekomendasi visualisasi, serta sumber referensi akademik yang digunakan untuk penulisan Bab 1–4.

## 1. Sumber Data Eksperimen

Data final penelitian berasal dari dua skenario pengujian:

- **Scenario 1 — MQTT Handshake**
  - Lokasi data: `data/csv_joined/`
  - Jumlah file: 12 file.
  - Matriks: 3 varian ML-KEM (`512`, `768`, `1024`) × 2 status PIE (`pieoff`, `pieon`) × 2 mode koneksi (`session`, `reconn`).
  - Jumlah iterasi: 100 iterasi per konfigurasi.
  - Status validasi: seluruh konfigurasi memiliki 100 baris, `encaps_us` lengkap, dan `packet_loss = 0`.

- **Scenario 2 — Local Compute**
  - Lokasi data: `data/csv/`
  - Jumlah file: 6 file.
  - Matriks: 3 varian ML-KEM (`512`, `768`, `1024`) × 2 status PIE (`pieoff`, `pieon`).
  - Operasi: `keygen`, `encaps`, dan `decaps`.
  - Jumlah iterasi: 100 iterasi per operasi.

## 2. Sumber Referensi Akademik RAG/Chroma

Selain daftar referensi manual pada `tmp/ref.md`, penulisan draft menggunakan korpus PDF jurnal yang sudah tersedia dan di-embedding pada project RAG berikut:

| Komponen | Path |
|---|---|
| `PYTHONPATH` | `/home/me/projects/MCP/RAG` |
| `PROJECT` | `/home/me/projects/MCP/RAG/rag_pdf` |
| `DATA` | `/home/me/projects/MCP/RAG/data` |
| `RAG_PDF_DIR` | `/home/me/projects/MCP/RAG/data/pdfs` |
| `RAG_CHROMA_DIR` | `/home/me/projects/MCP/RAG/data/chroma` |

Korpus PDF tersebut mencakup sumber utama seperti standar MQTT v5.0 (Banks et al., 2019), FIPS 203 ML-KEM (NIST, 2024), ESP32-S3 Technical Reference Manual (Espressif Systems, 2025), artikel QERS (Rassekhnia, 2026), benchmarking ML-KEM/ML-DSA pada RP2040 (Chhetri, 2026), implementasi Kyber pada ESP32 (Segatz & Hafiz, 2025), evaluasi PQC pada MQTT (Malina et al., 2024), serta literatur optimasi NTT dan PQC pada perangkat terbatas.

## 3. Tahap Praproses Data

Tahapan praproses yang digunakan untuk analisis final adalah sebagai berikut:

1. **Validasi schema**
   - S1 joined harus memiliki 23 kolom termasuk `reconnect_us`, `reconnect_energy_uj`, dan `encaps_us`.
   - S2 harus memiliki 10 kolom termasuk `op`, `cycles`, `time_us`, dan `energy_uj`.

2. **Validasi jumlah iterasi**
   - Setiap konfigurasi S1 harus memiliki 100 iterasi.
   - Setiap operasi pada konfigurasi S2 harus memiliki 100 iterasi.

3. **Filter packet loss**
   - Untuk S1, metrik latency, energy, jitter, dan CPU dihitung pada iterasi sukses.
   - Karena seluruh file final memiliki `packet_loss = 0`, tidak ada baris S1 yang dibuang.

4. **Perlakuan jitter**
   - `jitter_us` pada iterasi pertama tidak dipakai untuk median jitter karena iterasi awal secara desain tidak memiliki pembanding latency sebelumnya.

5. **Outlier**
   - Outlier tidak dihapus otomatis dari QERS karena S1 merepresentasikan kondisi operasional jaringan nyata.
   - Untuk S2, median digunakan sebagai ringkasan utama karena lebih tahan terhadap spike pembacaan energi INA219.

6. **Unit**
   - Waktu komputasi tetap ditampilkan dalam mikrodetik.
   - Handshake ditampilkan dalam milidetik.
   - Total latency mode `reconn` ditampilkan dalam detik karena mencakup proses reconnect.
   - Energi handshake ditampilkan dalam mJ, sedangkan total energi siklus `reconn` ditampilkan dalam J.

## 4. Karakter Data Scenario 2

Scenario 2 mengisolasi operasi ML-KEM dari pengaruh jaringan. Oleh karena itu, metrik `cycles` dan `time_us` adalah indikator paling langsung untuk menilai efek PIE terhadap jalur komputasi.

### 4.1 Ringkasan PIE vs No PIE Scenario 2

| Varian | Operasi | Time No PIE (µs) | Time PIE (µs) | Δ Time | Cycles No PIE | Cycles PIE | Δ Cycles | Energy No PIE (µJ) | Energy PIE (µJ) | Δ Energy |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| ML-KEM-512 | KeyGen | 7,765.0 | 7,487.0 | -3.58% | 1,242,119 | 1,197,602 | -3.58% | 1,395.5 | 1,606.0 | +15.08% |
| ML-KEM-512 | Encaps | 9,211.5 | 8,840.0 | -4.03% | 1,473,624 | 1,414,228 | -4.03% | 1,456.5 | 1,676.5 | +15.10% |
| ML-KEM-512 | Decaps | 11,205.0 | 10,646.0 | -4.99% | 1,792,732 | 1,703,231 | -4.99% | 1,825.5 | 1,411.5 | -22.68% |
| ML-KEM-768 | KeyGen | 12,344.5 | 11,826.0 | -4.20% | 1,974,784 | 1,891,753 | -4.20% | 1,727.5 | 1,928.5 | +11.64% |
| ML-KEM-768 | Encaps | 14,520.0 | 13,874.5 | -4.45% | 2,322,944 | 2,219,601 | -4.45% | 2,848.0 | 2,584.5 | -9.25% |
| ML-KEM-768 | Decaps | 17,164.5 | 16,251.0 | -5.32% | 2,746,047 | 2,599,957 | -5.32% | 3,859.0 | 3,281.5 | -14.97% |
| ML-KEM-1024 | KeyGen | 18,721.0 | 17,993.0 | -3.89% | 2,995,247 | 2,878,482 | -3.90% | 2,891.0 | 3,808.0 | +31.72% |
| ML-KEM-1024 | Encaps | 21,423.0 | 20,485.0 | -4.38% | 3,427,424 | 3,277,282 | -4.38% | 5,593.5 | 4,606.5 | -17.65% |
| ML-KEM-1024 | Decaps | 24,759.0 | 23,513.0 | -5.03% | 3,960,992 | 3,761,773 | -5.03% | 4,797.5 | 5,422.0 | +13.02% |

### 4.2 Interpretasi Scenario 2

PIE secara konsisten mempercepat waktu eksekusi dan menurunkan jumlah siklus CPU pada semua operasi dan semua varian. Rentang penurunan `time_us` dan `cycles` berada pada sekitar 3.58% hingga 5.32%. Efek paling besar muncul pada operasi `decaps`, khususnya ML-KEM-768 dengan penurunan 5.32%. Hal ini sesuai dengan desain optimasi karena jalur PIE mempercepat operasi polinomial yang banyak digunakan dalam NTT, inverse NTT, dan perkalian polinomial.

Namun, energi tidak selalu membaik. Pada beberapa kombinasi seperti KeyGen ML-KEM-512, KeyGen ML-KEM-768, KeyGen ML-KEM-1024, dan Decaps ML-KEM-1024, nilai median energi PIE justru lebih tinggi. Hal ini menunjukkan bahwa percepatan siklus CPU tidak selalu berbanding lurus dengan penghematan energi fisik, terutama karena pembacaan INA219 menangkap konsumsi sistem secara keseluruhan, bukan hanya konsumsi inti CPU.

## 5. Karakter Data Scenario 1

Scenario 1 mengevaluasi ML-KEM dalam konteks komunikasi MQTT v5.0. Pada skenario ini, ESP32-S3 menjalankan KeyGen dan Decaps, sedangkan subscriber menjalankan Encaps. Broker MQTT menjadi perantara publish/subscribe.

### 5.1 Ringkasan Median Scenario 1

| Varian | PIE | Mode | Loss | KeyGen (µs) | Decaps (µs) | Encaps Subscriber (µs) | Handshake (ms) | Total Latency (s) | Energy Handshake (mJ) | Total Energy (J) | CPU (%) | Jitter (ms) | RSSI |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| ML-KEM-512 | pieoff | reconn | 0 | 8,238 | 12,360 | 1,026 | 332.9 | 16.91 | 117.5 | 3.831 | 6.1 | 445.7 | -30 |
| ML-KEM-512 | pieoff | session | 0 | 7,904 | 12,404 | 1,020 | 277.9 | 0.28 | 104.6 | 0.105 | 7.2 | 88.3 | -30 |
| ML-KEM-512 | pieon | reconn | 0 | 7,968 | 11,910 | 1,026 | 373.1 | 16.95 | 126.5 | 3.847 | 5.2 | 263.8 | -29 |
| ML-KEM-512 | pieon | session | 0 | 7,588 | 11,891 | 1,023 | 317.6 | 0.32 | 108.5 | 0.109 | 6.2 | 202.3 | -29 |
| ML-KEM-768 | pieoff | reconn | 0 | 13,038 | 18,242 | 1,516 | 190.4 | 16.19 | 84.0 | 3.520 | 16.4 | 0.4 | -33 |
| ML-KEM-768 | pieoff | session | 0 | 12,528 | 18,218 | 1,506 | 247.1 | 0.25 | 88.4 | 0.088 | 12.5 | 2.3 | -33 |
| ML-KEM-768 | pieon | reconn | 0 | 12,350 | 17,379 | 1,502 | 189.4 | 16.19 | 83.0 | 3.521 | 15.7 | 0.3 | -33 |
| ML-KEM-768 | pieon | session | 0 | 12,021 | 17,401 | 1,500 | 247.3 | 0.25 | 89.2 | 0.089 | 11.9 | 2.5 | -34 |
| ML-KEM-1024 | pieoff | reconn | 0 | 19,516 | 25,906 | 2,115 | 111.5 | 16.13 | 59.2 | 3.608 | 40.8 | 1.9 | -33 |
| ML-KEM-1024 | pieoff | session | 0 | 19,986 | 25,907 | 2,045 | 68.7 | 0.07 | 39.7 | 0.040 | 66.9 | 2.1 | -28 |
| ML-KEM-1024 | pieon | reconn | 0 | 18,722 | 24,637 | 2,096 | 109.4 | 16.13 | 59.1 | 3.606 | 39.6 | 1.7 | -29 |
| ML-KEM-1024 | pieon | session | 0 | 19,052 | 24,656 | 2,036 | 66.2 | 0.07 | 39.1 | 0.039 | 65.9 | 1.7 | -28 |

### 5.2 Interpretasi Scenario 1

Seluruh konfigurasi S1 valid karena tidak ada packet loss. PIE tetap terlihat menurunkan `keygen_us` dan `decaps_us`, tetapi pengaruhnya terhadap `handshake_us` lebih kecil dan tidak selalu searah. Hal ini wajar karena `handshake_us` tidak hanya berisi komputasi ML-KEM, tetapi juga publish/subscribe MQTT, scheduling Wi-Fi, broker forwarding, dan respons subscriber.

Mode `session` jauh lebih efisien untuk operasi normal. Mode `reconn` menambahkan sekitar 15.95–16.63 detik latency total dan sekitar 3.43–3.74 J energi total per siklus. Oleh karena itu, `reconn` tidak cocok sebagai mode operasi periodik hemat energi, tetapi berguna sebagai skenario worst-case untuk mengukur biaya autentikasi/koneksi ulang.

## 6. QERS Fusion

QERS dihitung dari data S1 dengan normalisasi min-max pada 12 konfigurasi. Metrik cost adalah total latency, total energy, CPU utilization, packet loss, jitter, dan payload size. Metrik benefit adalah proven resistance, heap minimum tersedia, dan RSSI. Bobot yang digunakan:

- **Subskor Performa (P)**
  - Latency: 0.35
  - Energy: 0.25
  - CPU utilization: 0.20
  - Packet loss: 0.10
  - Jitter: 0.10

- **Subskor Security/Resource (S)**
  - Proven resistance: 0.40
  - Heap/resource availability: 0.30
  - Payload/key size: 0.20
  - RSSI: 0.10

- **Fusion**
  - α = 0.70 untuk performa.
  - β = 0.30 untuk security/resource.

### 6.1 Ranking QERS

| Rank | Varian | PIE | Mode | QERS | P | S | Total Latency (s) | Total Energy (J) | CPU (%) | Jitter (ms) |
|---:|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 1 | ML-KEM-768 | pieoff | session | 85.72 | 96.89 | 59.65 | 0.25 | 0.088 | 12.5 | 2.3 |
| 2 | ML-KEM-768 | pieon | session | 85.34 | 97.07 | 57.98 | 0.25 | 0.089 | 11.9 | 2.5 |
| 3 | ML-KEM-512 | pieoff | session | 84.55 | 96.51 | 56.67 | 0.28 | 0.105 | 7.2 | 88.3 |
| 4 | ML-KEM-512 | pieon | session | 83.24 | 94.16 | 57.77 | 0.32 | 0.109 | 6.2 | 202.3 |
| 5 | ML-KEM-1024 | pieon | session | 76.21 | 80.29 | 66.69 | 0.07 | 0.039 | 65.9 | 1.7 |
| 6 | ML-KEM-1024 | pieoff | session | 75.98 | 79.95 | 66.72 | 0.07 | 0.040 | 66.9 | 2.1 |
| 7 | ML-KEM-768 | pieoff | reconn | 44.48 | 40.08 | 54.77 | 16.19 | 3.520 | 16.4 | 0.4 |
| 8 | ML-KEM-768 | pieon | reconn | 44.42 | 40.29 | 54.04 | 16.19 | 3.521 | 15.7 | 0.3 |
| 9 | ML-KEM-512 | pieon | reconn | 40.98 | 34.08 | 57.09 | 16.95 | 3.847 | 5.2 | 263.8 |
| 10 | ML-KEM-512 | pieoff | reconn | 37.72 | 29.86 | 56.04 | 16.91 | 3.831 | 6.1 | 445.7 |
| 11 | ML-KEM-1024 | pieon | reconn | 36.96 | 32.09 | 48.33 | 16.13 | 3.606 | 39.6 | 1.7 |
| 12 | ML-KEM-1024 | pieoff | reconn | 34.70 | 31.70 | 41.72 | 16.13 | 3.608 | 40.8 | 1.9 |

### 6.2 Kesimpulan QERS

Berdasarkan bobot performa 70% dan security/resource 30%, konfigurasi terbaik adalah **ML-KEM-768 pieoff session** dengan QERS 85.72, diikuti sangat dekat oleh **ML-KEM-768 pieon session** dengan QERS 85.34. Perbedaan keduanya kecil, sehingga secara praktis ML-KEM-768 session adalah opsi paling seimbang. PIE tetap berguna untuk menurunkan beban komputasi ESP32, tetapi pada QERS final pengaruhnya tertutup oleh dominasi metrik end-to-end seperti latency total, energy total, dan karakter jaringan.

## 7. Rekomendasi Visualisasi

- **Tabel ringkasan**
  - Digunakan untuk nilai final median/p95 dan ranking QERS.
  - Cocok karena pembaca perlu melihat angka eksak.

- **Bar chart**
  - Digunakan untuk speed-up PIE vs No PIE pada S2.
  - Cocok untuk menampilkan perbandingan antar operasi dan varian.

- **Box plot**
  - Digunakan untuk distribusi `handshake_us` dan `energy_uj` S1.
  - Cocok untuk menunjukkan sebaran, median, dan outlier.

- **Line plot per iterasi**
  - Digunakan untuk `reconnect_us`, `total_latency_us`, RSSI, dan jitter.
  - Cocok untuk memperlihatkan stabilitas temporal dan spike.

- **Radar chart**
  - Opsional untuk profil QERS beberapa konfigurasi terbaik.
  - Tidak disarankan untuk seluruh 12 konfigurasi karena akan sulit dibaca.

## 8. Justifikasi Metodologi

- **ML-KEM**
  - Dipilih karena menjadi standar NIST FIPS 203 untuk mekanisme enkapsulasi kunci pasca-kuantum (NIST, 2024).

- **MQTT v5.0**
  - Dipilih karena relevan dengan IoT publish/subscribe dan memiliki spesifikasi formal modern yang sesuai untuk evaluasi handshake pada protokol ringan (Banks et al., 2019; Malina et al., 2024).

- **QoS 1**
  - Dipilih sebagai kompromi antara reliabilitas dan overhead. QoS 0 terlalu lemah untuk pengujian handshake karena tidak menjamin pengiriman, sedangkan QoS 2 menambahkan overhead dua fase yang dapat mendominasi metrik performa.

- **100 iterasi**
  - Dipilih agar estimasi median, p95, dan variasi cukup stabil, serta sejalan dengan praktik benchmarking sistem dan kriptografi pada perangkat terbatas (Jain, 1991; Georges et al., 2007; Chhetri, 2026).

- **S1 dan S2**
  - S2 dibutuhkan untuk melihat efek komputasi murni PIE.
  - S1 dibutuhkan untuk melihat dampak sistemik ketika ML-KEM dipakai dalam handshake MQTT nyata.

- **Session dan Reconnect**
  - `session` merepresentasikan operasi normal dengan koneksi dipertahankan.
  - `reconn` merepresentasikan worst-case atau autentikasi ulang per siklus.
