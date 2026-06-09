# Fallback Evaluation Plan when QERS Weights Are Unresolved

Jika pembobotan AHP untuk QERS tidak menemukan titik terang (mis. perdebatan orientasi K/Co atau CR tidak memadai), gunakan rencana evaluasi alternatif berikut untuk menjaga obyektivitas dan reprodusibilitas.

## Tujuan
- Memberikan hasil peringkat/seleksi alternatif yang robust tanpa mengunci bobot tunggal.
- Memanfaatkan beberapa metode MCDA dan analisis sensitivitas untuk menyajikan konsensus peringkat.

## Metode Alternatif
- TOPSIS (Equal/Entropy Weights)
  - Langkah: normalisasi min–maks per‑mode; tentukan bobot sama rata atau pakai Entropy untuk bobot objektif; hitung jarak ke solusi ideal positif/negatif; ranking berdasarkan kedekatan relatif.
  - Output: tabel skor TOPSIS per alternatif; sensitivitas bobot (equal vs entropy).
- Pareto Front + Security Tiebreak
  - Bangun front Pareto pada metrik performa utama (L, E, C). Filter alternatif non‑dominated. Tiebreak pertama: skor keamanan (`Pr`, lalu `K/Co` sesuai kebijakan), kedua: `R`.
  - Output: himpunan solusi Pareto per‑mode dan urutan tiebreak.
- Robust Ranking via Weight-Space Sampling
  - Sampling acak bobot makro `P∼U(0.3,0.8)`, `S=1−P`; sub‑bobot P dan S dari Dirichlet (prior domain). Jalankan QERS untuk N ribu sampel; hitung frekuensi top‑1/top‑3 (Borda/Condorcet approximation).
  - Output: probabilitas menjadi juara/top‑k; grafik sensitivitas terhadap bobot.
- PCA‑Index + Security Constraint
  - PCA pada metrik performa (L, E, C, J); indeks = komponen utama pertama (dibalik agar “lebih baik=lebih besar”); saring alternatif dengan `Pr` di bawah ambang; urutkan berdasarkan indeks.
  - Output: indeks performa dan ranking constrained oleh keamanan.
- PROMETHEE II (opsional, jika ingin outranking)
  - Definisikan fungsi preferensi per metrik (cost/benefit) dan ambang p,q; hitung leaving/entering flows; net flow untuk peringkat.

## Data dan I/O
- Input: `data/analysis/s1_kpi_table.csv`, `data/analysis/s1_quality_summary.csv` (mode terpisah: session, reconnect).
- Output (disarankan): `data/analysis/fallback/{topsis,pareto,robust,pca,promethee}/` berisi csv+md.

## Implementasi Minimal
- Equal‑weight TOPSIS per‑mode.
- Pareto L/E/C per‑mode dengan tiebreak keamanan.
- Weight‑sampling (1–5k sampel) untuk QERS guna robustifikasi.

## Kapan Menggunakan
- Ketika CR pairwise > 0.1 dan tidak dapat diturunkan tanpa kehilangan rasional domain.
- Ketika orientasi metrik K/Co belum disepakati; jalankan dua skenario dan bandingkan hasil (report divergence).

## Catatan Replikasi
- Semua metode menggunakan normalisasi per‑mode yang sama seperti QERS.
- Dokumentasikan parameter (bobot, ambang preferensi, rentang sampling) dalam artefak output agar dapat dikutip pada Bab 3/4.

## Referensi Singkat
- Hwang, C.-L., & Yoon, K. (1981). Multiple Attribute Decision Making (TOPSIS).
- Brans, J.‑P., & Vincke, P. (1985). PROMETHEE: A new family of outranking methods.
- Deb, K. et al. (2002). A fast and elitist multiobjective genetic algorithm: NSGA‑II (untuk konsep Pareto & knee points).
- Saaty, T. L. (1980). The Analytic Hierarchy Process.
