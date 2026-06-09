# AHP Weighting Methodology for QERS (Performance vs Security)

Ringkas: dokumen ini memformalakan dasar pembobotan AHP untuk QERS, menyertakan pendekatan data-driven (Entropy Weighting) yang dikombinasikan dengan prior pengetahuan domain (swing/policy), serta prosedur uji konsistensi (CR). Hasil akhir dapat direproduksi lewat skrip `tools/derive_weights.py` tanpa mengubah pipeline QERS.

## Tujuan
- Memberikan dasar metodologis yang kuat untuk bobot level atas `P` (Performance) dan `S` (Security), serta sub-bobot di dalam masing‑masing level.
- Menjaga keselarasan dengan paper QERS: normalisasi min–maks per metrik, orientasi metrik sesuai rumus paper, dan komputasi skor fusi.

## Definisi Kriteria dan Pemetaan Dataset
- Performance (P):
  - L (latency): `total_latency_us_p50` (cost)
  - E (energy): `total_energy_uj_p50` (cost)
  - C (CPU): `cpu_util_percent_p50` (cost)
  - J (jitter): `jitter_us_p50` (cost)
  - Ploss (packet loss rate): `packet_loss_rate` (cost)
- Security (S):
  - Pr (proven resistance): pemetaan varian ML‑KEM ke level NIST (512→1, 768→3, 1024→5) [FIPS 203]
  - K (key/payload size): `pub_bytes_p50 + ct_bytes_p50` (orientasi sesuai paper QERS)
  - Co (overhead bytes): `overhead_bytes_p50` (orientasi sesuai paper QERS)
  - R (RSSI): `rssi_dbm_p50` (benefit)

Semua normalisasi dilakukan per‑mode (session, reconnect) untuk menjaga fairness antar alternatif pada konteks problem keputusan yang sama.

## Dasar Bobot Level Atas (P vs S)
- Rasional penelitian: PIE memengaruhi metrik komputasi (L/E/C) secara signifikan, sedangkan keamanan kriptografis (Pr) berasal dari varian ML‑KEM dan tidak berubah oleh PIE. Dalam dataset akhir S1, `packet_loss_rate=0` dan jitter relatif kecil, sehingga leverage keputusan lebih besar pada dimensi kinerja.
- Metode kuantitatif yang disarankan:
  - Variance decomposition per makro‑kriteria: hitung dispersi terstandarisasi (mis. rata‑rata CV atau rentang ternormalisasi) dari metrik P dan S pada alternatif per‑mode, lalu set `w_P = D_P / (D_P + D_S)`, `w_S = 1 - w_P`.
  - Dampak toggling PIE: untuk setiap varian, bandingkan delta metrik P dan S antara PIE ON vs OFF; bobot proporsional terhadap kontribusi delta P vs S.
- Rekomendasi default berbasis sifat data: `P = 0.6`, `S = 0.4`. Angka ini menyeimbangkan fakta bahwa P mendominasi variasi keputusan (karena PIE), sambil tetap memberi porsi substansial pada S agar pilihan dengan Pr lebih tinggi dan beban transmisi/overhead yang wajar tetap dihargai. Skrip pendamping akan menghitung usulan `P,S` berbasis dispersi aktual, sehingga angka 0.6/0.4 bisa divalidasi/diadjust.

## Sub‑Bobot: Metode Hybrid (Entropy × Prior)
Untuk setiap makro‑kriteria (P dan S) gunakan kombinasi:
- Objective (Entropy Weighting) [Shannon, 1948; Hwang & Yoon, 1981]:
  - Ubah semua metrik ke orientasi benefit (cost di‑flip dengan komplement min–maks).
  - Normalisasi ke `p_ij` sehingga Σ_i p_ij = 1.
  - Entropy: `e_j = -k Σ_i p_ij ln(p_ij)`, `k = 1/ln(n)`.
  - Diversifikasi: `d_j = 1 - e_j`, bobot objektif: `w*_j = d_j / Σ_j d_j`.
  - Guard: jika varian mendekati nol (mis. packet loss = 0 di semua alternatif), atur `w*_j = 0` dan renormalisasi.
- Subjective prior (policy/swing):
  - Performance prior (sesuai fokus PIE dan karakter data): `L > E > C > (J ≈ Ploss)`.
  - Security prior (mengikuti perumusan paper QERS dan praktik keamanan): `Pr` dominan; `K` dan `Co` moderat; `R` untuk kualitas link.
- Hybrid: `w_j = α · w*_j + (1 − α) · w^prior_j` (default α=0.5), lalu renormalisasi. Nilai α dapat dinaikkan bila ingin lebih data‑driven, atau diturunkan bila ingin lebih policy‑driven.

Contoh prior yang kami gunakan sebagai titik awal (akan dikalibrasi oleh komponen Entropy di atas):
- Performance prior: `L=0.45, E=0.35, C=0.10, J=0.05, Ploss=0.05`.
- Security prior (kebijakan terbaru): `Pr=0.50, K=0.20, Co=0.20, R=0.10`.

Hasil akhir (post‑hybrid) bukan angka tetap; ia bergantung pada dispersi metrik di data Anda dan akan dioutput oleh skrip pendamping.

## Uji Konsistensi (CR) untuk Matriks Pairwise
- Dari bobot akhir `w_j`, bentuk matriks pairwise konsisten `a_ij = w_i / w_j` (memberi CR≈0 secara teoritis). Jika ingin menguji CR terhadap matriks pairwise subjektif (sebelum hybrid), gunakan prosedur AHP standar: hitung λ_max, CI, CR (`CR = CI/RI`); terima jika `CR < 0.1` [Saaty].

## Reproducibility dengan Skrip
- Skrip: `tools/derive_weights.py`
  - Input: `data/analysis/s1_kpi_table.csv`, `data/analysis/s1_quality_summary.csv`.
  - Opsi utama: `--mode {session|reconn|all}`, `--alpha 0..1`, `--k-cost/--k-benefit`, `--co-cost/--co-benefit`.
  - Output: `data/analysis/weights/weights_summary.{json,md}` berisi bobot objektif (Entropy), bobot prior, bobot hybrid final, serta usulan `P,S` berbasis dispersi makro.
  - Tidak mengubah pipeline QERS; artefak ini menjadi dasar justifikasi akademik untuk bobot.

## Referensi Singkat
- Saaty, T. L. (1980). The Analytic Hierarchy Process.
- Hwang, C.-L., & Yoon, K. (1981). Multiple Attribute Decision Making.
- Shannon, C. E. (1948). A Mathematical Theory of Communication.
- NIST FIPS 203 (ML‑KEM) untuk level keamanan varian (Pr).
