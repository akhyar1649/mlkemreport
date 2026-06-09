# Experiment Plan — Matriks, Checklist Data CSV, QERS, dan Analisis PIE

Dokumen ini mencakup rencana eksperimen lengkap, checklist data CSV, justifikasi 100 iterasi, metrik perbandingan PIE, template tabel, dan QERS Fusion.

---

## 1) Matriks Eksperimen dan Checklist Data CSV

### Aturan eksperimen

- 1 build + 1 flash = 1 Scenario + 1 konfigurasi PIE + 1 varian ML-KEM + (untuk S1) 1 reconnect mode.
- Total: **18 run** (6 S2 + 12 S1).
- Setiap run S1 menghasilkan dua file CSV: ESP (`data/csv/`) + Subscriber (`data/csv_subscriber/`).

### Scenario 2 — Local Compute (6 run)

| # | File CSV (`data/csv/`) | Status |
|---|------------------------|--------|
| 1 | `s2_512_pieoff_*.csv` | ✅ done |
| 2 | `s2_512_pieon_*.csv` | ✅ done |
| 3 | `s2_768_pieoff_*.csv` | ✅ done |
| 4 | `s2_768_pieon_*.csv` | ✅ done |
| 5 | `s2_1024_pieoff_*.csv` | ✅ done |
| 6 | `s2_1024_pieon_*.csv` | ✅ done |

### Scenario 1 — MQTT Handshake (12 run, masing-masing perlu 2 CSV)

| # | File CSV ESP (`data/csv/`) | File CSV Subscriber (`data/csv_subscriber/`) | MQTT_RECONNECT | Status |
|---|----------------------------|----------------------------------------------|----------------|--------|
| 1 | `s1_512_pieoff_reconn_*.csv` | `s1_512_pieoff_reconn_*.csv` | `y` | ✅ done |
| 2 | `s1_512_pieoff_session_*.csv` | `s1_512_pieoff_session_*.csv` | `n` | ✅ done |
| 3 | `s1_512_pieon_reconn_*.csv` | `s1_512_pieon_reconn_*.csv` | `y` | ✅ done |
| 4 | `s1_512_pieon_session_*.csv` | `s1_512_pieon_session_*.csv` | `n` | ✅ done |
| 5 | `s1_768_pieoff_reconn_*.csv` | `s1_768_pieoff_reconn_*.csv` | `y` | ✅ done |
| 6 | `s1_768_pieoff_session_*.csv` | `s1_768_pieoff_session_*.csv` | `n` | ✅ done |
| 7 | `s1_768_pieon_reconn_*.csv` | `s1_768_pieon_reconn_*.csv` | `y` | ✅ done |
| 8 | `s1_768_pieon_session_*.csv` | `s1_768_pieon_session_*.csv` | `n` | ✅ done |
| 9 | `s1_1024_pieoff_reconn_*.csv` | `s1_1024_pieoff_reconn_*.csv` | `y` | ✅ done |
| 10 | `s1_1024_pieoff_session_*.csv` | `s1_1024_pieoff_session_*.csv` | `n` | ✅ done |
| 11 | `s1_1024_pieon_reconn_*.csv` | `s1_1024_pieon_reconn_*.csv` | `y` | ✅ done |
| 12 | `s1_1024_pieon_session_*.csv` | `s1_1024_pieon_session_*.csv` | `n` | ✅ done |

**Total file CSV yang perlu ada**: 6 (S2) + 12 ESP (S1) + 12 subscriber (S1) = **30 file CSV**

File naming pattern:
- S2: `s2_<variant>_<pieon|pieoff>_YYYYMMDD_HHMMSS.csv`
- S1: `s1_<variant>_<pieon|pieoff>_<reconn|session>_YYYYMMDD_HHMMSS.csv`
- S1 subscriber: sama dengan S1 ESP (tanpa suffix tambahan)
- S1 joined: `s1_<variant>_<pieon|pieoff>_<reconn|session>_YYYYMMDD_HHMMSS_joined.csv` (hasil `join_subscriber_csv.py`)

---

## 2) Justifikasi Jumlah Iterasi: 100x

Jumlah iterasi ditetapkan 100x per operasi per varian, berdasarkan:

### Referensi literatur

- **Chhetri et al. (RP2040, 2025)**: menggunakan 100 iterasi per security level untuk ML-KEM FIPS 203, melaporkan mean, standard deviation, dan coefficient of variation (CV).
- **Comprehensive PQ eval (IoT, 2024)**: 30 repetisi, dalam mean ±1.96×SE dengan 95% probabilitas.
- **V2X Kyber+Saber**: minimum 10 kali, 95% confidence interval.

### Justifikasi empiris dari data Scenario 2

Metrik energi (`energy_uj`) memiliki CV ~9% yang lebih tinggi dari cycles (CV ~0.3%), karena fluktuasi sampling INA219. Ini membutuhkan lebih banyak sampel untuk confidence interval yang ketat:

| n | Mean energy keygen | 95% CI | ±% dari mean |
|---|-------------------|--------|--------------|
| 10 | 1878.7 µJ | ±74.9 µJ | ±3.99% |
| 20 | 1865.7 µJ | ±72.1 µJ | ±3.87% |
| 30 | 1900.8 µJ | ±70.7 µJ | ±3.72% |
| 50 | 1904.5 µJ | ±52.6 µJ | ±2.76% |
| **100** | **1909.6 µJ** | **±34.4 µJ** | **±1.80%** |

Dengan n=100, 95% CI untuk energi mencapai ±1.80% dari mean — setara dengan akurasi yang digunakan Chhetri (2025) untuk ML-KEM. Pengurangan lebar CI ini penting untuk keandalan perbandingan lintas varian dan mode optimasi.

---

## 3) Metrik yang Dibandingkan

### Scenario 2 — metrik utama per operasi

| Kolom CSV | Interpretasi |
|-----------|--------------|
| `cycles` | Cycle count CPU per op (paling stabil, CV ~0.3%) |
| `time_us` | Elapsed time µs per op |
| `energy_uj` | Energi fisik µJ per op (INA219, CV ~1–9%) |
| `heap_free`, `heap_min` | Overhead memori heap |
| `stack_hwm_bytes` | Stack high-water mark |

### Scenario 1 — metrik utama per iterasi

| Kolom CSV | Interpretasi |
|-----------|--------------|
| `keygen_us`, `decaps_us` | Compute time di ESP |
| `handshake_us` | Durasi end-to-end handshake (RTT) |
| `net_latency_us`, `jitter_us` | Karakteristik jaringan |
| `energy_uj` | Energi seluruh window iterasi (termasuk Wi-Fi) |
| `rssi_dbm` | Kualitas sinyal Wi-Fi |
| `packet_loss` | Indikator kegagalan iterasi |
| `cpu_util_percent` | Aproksimasi utilisasi CPU kripto |

### Metrik turunan (dihitung offline dari CSV)

- `ops_per_second` = 1e6 / time_us
- `energy_per_cycle` = energy_uj / cycles
- `failure_rate` = count(packet_loss=1) / total_iter (Scenario 1)
- `total_latency_us` = `handshake_us + reconnect_us` (Scenario 1, khusus mode `reconn`)
- `total_energy_uj` = `energy_uj + reconnect_energy_uj` (Scenario 1, khusus mode `reconn`)
- p50 dan p95 untuk time_us, cycles, energy_uj per op
- `handshake_us` (gunakan hanya packet_loss=0 baris)

---

## 4) Ruang Lingkup Perbandingan

### 4.1 PIE Study: PIE off vs PIE on

Untuk setiap kombinasi (scenario, variant, op):
1. Hitung mean, p50, p95 untuk cycles, time_us, energy_uj.
2. Hitung delta absolut dan persen: `delta_percent = (PIE_on - PIE_off) / PIE_off × 100`
3. Speedup ratio: `PIE_off / PIE_on` (angka >1 berarti PIE lebih cepat).

### 4.2 Reconnect Study: reconn vs session (Scenario 1)

Bandingkan per kombinasi (variant, PIE):
- `reconnect_us` dan `reconnect_energy_uj` (overhead langsung per sesi baru)
- `total_latency_us` dan `total_energy_uj` (biaya total per siklus penuh di mode `reconn`)
- Implikasi: penalty baterai untuk perangkat deep-sleep/intermiten dapat dihitung dari `reconnect_energy_uj` atau `total_energy_uj`.

### 4.3 Multi-Varian Study: 512 vs 768 vs 1024

Sajikan sebagai kurva tradeoff keamanan-kinerja-energi:
- x-axis: security level (512=Level 1, 768=Level 3, 1024=Level 5)
- y-axis: cycles, time_us, energy_uj
- Nilai relatif: `ratio_vs_512 = metric_varian / metric_512`

---

## 5) QERS Fusion (Scenario 1)

QERS (Quantum Encryption Resilience Score) Fusion menggunakan metrik Scenario 1 untuk menghitung skor trade-off performa vs keamanan.

### Komponen QERS

**Performance Subscore** (cost criteria — semakin kecil semakin baik):
- L: `handshake_us` (Latency)
- J: `jitter_us` (Jitter)
- P: `packet_loss` (Packet loss rate)
- E: `energy_uj` (Energy consumption)
- C: `cpu_util_percent` (CPU utilization)

**Security Subscore** (campuran sesuai implementasi saat ini):
- K: Key size (COST; konstan per varian: 512/768/1024)
- R: `rssi_dbm` (BENEFIT; kualitas sinyal)
- Pr: Proven resistance (BENEFIT; skor tetap per level ML-KEM)
- Co: `pub_bytes + ct_bytes + overhead_bytes` (COST; cryptographic overhead)

### Normalisasi min-max

```
r_ij = 100 × (x_max - x_ij) / (x_max - x_min)  [cost criteria]
r_ij = 100 × (x_ij - x_min) / (x_max - x_min)  [benefit criteria]
```

### Fusion Score

```
P = Σ_{k ∈ {L,J,P_loss,E,C}} w_k × r_ik
S = Σ_{k ∈ {K,R,Pr,Co}} w_k × r_ik
F = α × (MaxScore - P) + β × S
```

Syarat: Σw_k(performance) = 1, Σw_k(security) = 1, α + β = 1, MaxScore = 100.

Mapping CSV ke QERS:
- L = `handshake_us`
- J = `jitter_us`
- P_loss = `packet_loss` rate
- E = `energy_uj`
- C = `cpu_util_percent`
- R = `rssi_dbm`
- K = konstan per varian
- Co = `pub_bytes + ct_bytes + overhead_bytes`

Catatan: `handshake_us` sudah mencakup waktu encaps subscriber (karena dihitung dari publish sampai ciphertext diterima + decaps selesai). QERS tidak membutuhkan `encaps_us` subscriber secara langsung.

---

## 6) Template Tabel Hasil

### Scenario 2 (per op, per varian)

| variant | op | metric | PIE off (mean ± σ) | PIE on (mean ± σ) | speedup | delta% |
|---------|----|---------|--------------------|-------------------|---------|--------|
| 512 | keygen | cycles | | | | |
| 512 | keygen | time_us | | | | |
| 512 | keygen | energy_uj | | | | |
| 512 | encaps | ... | | | | |
| 512 | decaps | ... | | | | |
| 768 | ... | | | | | |
| 1024 | ... | | | | | |

### Scenario 1 (per varian, per reconnect mode)

| variant | reconnect | metric | PIE off (p50/p95) | PIE on (p50/p95) | delta% |
|---------|-----------|---------|-------------------|------------------|--------|
| 512 | reconn | handshake_us | | | |
| 512 | reconn | energy_uj | | | |
| 512 | session | handshake_us | | | |
| 512 | session | energy_uj | | | |
| 768 | ... | | | | |
| 1024 | ... | | | | |

### Reconnect Comparison (per varian, PIE fixed)

| variant | PIE | metric | reconn (mean) | session (mean) | overhead_reconn | overhead% |
|---------|-----|---------|---------------|----------------|-----------------|-----------|
| 512 | off | energy_uj | | | | |
| 512 | off | handshake_us | | | | |
| ... | | | | | | |

---

## 7) Aturan Pembersihan Data

- Buang beberapa iterasi awal (misalnya 5) untuk menghindari warm-up effect.
- Scenario 1: buang baris `packet_loss=1` untuk metrik latency dan jitter. Simpan sebagai statistik terpisah.
- Untuk energy outlier: gunakan Z-score threshold 3.0 (karena CV energi ~9%, outlier nyata ada di ±3σ).

---

## 8) Keputusan Desain yang Sudah Disepakati

- Scope optimasi: full pipeline selama masih menggunakan fitur PIE.
- Prioritas: maximum speed, tetapi code tetap rapi dan readable.
- Varian: implementasi tiga varian berjalan paralel (512/768/1024).
- Integration: Kconfig option `CONFIG_MLKEM_PIE_OPT` dan `CONFIG_MLKEM_PIE_AGGRESSIVE`.
- Reconnect study: dua mode `reconn` dan `session` untuk Scenario 1.

---

## 9) Status Update

- **May 10–21, 2026**: Data Scenario 2 terkumpul untuk semua 6 kombinasi (512/768/1024 × PIE on/off).
- **May 28–29, 2026**: Data Scenario 1 terkumpul (12 ESP CSV + 12 subscriber CSV) dan hasil join dibuat (12 file di `data/csv_joined/`).
- **Tahap berikutnya**: Preprocessing + analisis S1/S2 dan finalisasi model QERS berdasarkan paper QERS primer.

---

Panduan analisis end-to-end (preprocessing, perintah terminal, QERS, dan what-if) ada di `docs/08-analysis-tools-and-workflows.md`.
