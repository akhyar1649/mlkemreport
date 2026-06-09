# [Digantikan] Analysis Tools: Inputs, Outputs, Statistics, and Interpretation

> Dokumen ringkas ini telah digabung ke panduan terpadu: `docs/08-analysis-tools-and-workflows.md`. Tetap dipertahankan sebagai referensi cepat. Untuk langkah lengkap (commands) dan workflow analisis, silakan rujuk ke dokumen terpadu.

Dokumen ini menjelaskan alat analisis yang digunakan pada pipeline data, termasuk definisi statistik (p50, p95, speedup), struktur input/output utama, serta cara membaca tabel keluaran. Alat yang tercakup:

- tools/analyze_campaign.py
- tools/analyze_pie_metrics.py
- tools/compute_qers.py
- tools/derive_weights.py

## Definisi Statistik
- p50: Median (persentil 50). Lebih stabil terhadap outlier dibanding rata-rata untuk waktu/energi.
- p95: Persentil 95. Menangkap ekor distribusi (worst tail) untuk menilai kestabilan.
- sd: Simpangan baku. cv_pct: koefisien variasi (sd/mean×100%).
- speedup: Perbandingan p50 PIE off terhadap p50 PIE on (off/on). speedup > 1 berarti PIE on lebih cepat/efisien.

## tools/analyze_campaign.py
- Tujuan:
  - Mengolah seluruh kampanye S1/S2: deskriptif (n, mean, sd, cv, p50, p95, min, max), tabel speedup PIE, perbandingan reconnect, dan KPI S1 untuk QERS.
- Input:
  - data/csv/ berisi CSV S2 per-run.
  - data/csv_joined/ berisi CSV S1 yang sudah di-join per-run.
- Opsi CLI (utama):
  - --s2-dir, --s1-joined-dir, --out-dir (default: data/analysis)
  - --drop-warmup N (default 5): membuang iterasi awal.
  - --include-loss-samples: sertakan baris packet_loss=1 pada distribusi metrik (default: tidak disertakan).
- Output (ke data/analysis/):
  - s2_summary_stats.{csv,md}
  - s1_summary_stats.{csv,md}
  - s1_quality_summary.{csv,md} (packet_loss_rate, ss_valid_rate)
  - pie_speedup_s2.{csv,md}
  - pie_speedup_s1.{csv,md}
  - reconnect_comparison.{csv,md} (session vs reconn)
  - s1_kpi_table.{csv,md} (wide table; input untuk QERS)
- Interpretasi kunci:
  - p50/p95 menjadi representasi pusat dan tail; speedup dihitung dari p50 agar robust.
  - Untuk reconnect, total_latency_us = handshake_us + reconnect_us, total_energy_uj = energy_uj + reconnect_energy_uj.

Contoh pemakaian:

```bash
python tools/analyze_campaign.py \
  --s2-dir data/csv \
  --s1-joined-dir data/csv_joined \
  --out-dir data/analysis \
  --drop-warmup 5
```

## tools/analyze_pie_metrics.py
- Tujuan: Bandingkan metrik PIE on/off dan keluarkan tabel ringkas per-skenario.
- Opsi CLI:
  - --scenario {s1|s2} (wajib)
  - --pie-off, --pie-on (path CSV)
  - --format md (saat ini hanya md)
  - --drop-warmup, --include-loss-samples
- Output: Tabel markdown berisi p50/p95 untuk off/on, delta, delta_percent.

Contoh pemakaian:

```bash
python tools/analyze_pie_metrics.py \
  --scenario s1 \
  --pie-off data/csv_joined/s1_512_pieoff_session_YYYYMMDD_HHMMSS_joined.csv \
  --pie-on  data/csv_joined/s1_512_pieon_session_YYYYMMDD_HHMMSS_joined.csv \
  --out data/analysis/pie_compare_s1_512_session.md
```

## tools/compute_qers.py
- Tujuan: Hitung skor QERS (Fusion) untuk konfigurasi S1 dari KPI table.
- Input: data/analysis/s1_kpi_table.csv
- Opsi CLI:
  - --kpi path (default di atas)
  - --out-dir (default: data/analysis)
  - --normalize-within-mode (disarankan): normalisasi min–maks per-mode (session/reconn) agar adil per problem keputusan.
  - --mode {session|reconn|all}: filter alternatif berdasarkan mode sebelum normalisasi (gunakan 'all' untuk menggabungkan keduanya; ranking per-mode tetap dihasilkan jika --normalize-within-mode aktif).
- Proses inti (ringkas):
  - Normalisasi min–maks ke skala 0..100 per metrik.
  - Performance penalty P = Σ w_perf[j] · norm_j untuk L, J, Ploss, E, C (semua sebagai biaya melalui penalti).
  - Security S = Σ w_sec[k] · norm_k untuk Pr, K, R, Co. Mulai sekarang default: K dan Co diperlakukan sebagai COST (penalti) dalam S.
  - Skor QERS_fusion = α·(100 − P) + β·S, dengan α≈0.6, β≈0.4 default (set melalui pairwise top-level Perf/Sec=1.5).
- Output (ke out-dir):
  - qers_weights.md (bobot performance/security dan CR)
  - qers_fusion_scores.{csv,md} (skor, P, S, peringkat global dan per-mode)
- Catatan:
  - Gunakan --normalize-within-mode untuk memperoleh ranking per-mode yang konsisten (session vs reconnect diproses set terpisah via normalisasi per-mode).

Contoh pemakaian:

```bash
# Session + Reconnect bersama: normalisasi per-mode untuk fairness (satu file, dua ranking)
python tools/compute_qers.py --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_mode \
  --normalize-within-mode --mode all

# Hasil dipisah per-mode (direktori terpisah)
python tools/compute_qers.py --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session \
  --normalize-within-mode --mode session

python tools/compute_qers.py --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_reconn \
  --normalize-within-mode --mode reconn
```

- Opsi lanjutan (what-if analysis):
  - `--alpha/--beta` untuk Fusion P vs S.
  - `--w-perf` dan `--w-sec` untuk override sub-bobot (format `L=..,E=..,C=..,J=..,Ploss=..` dan `Pr=..,K=..,R=..,Co=..`).
  - `--k-orientation` dan `--co-orientation` untuk cost/benefit (default sekarang: cost).

## tools/derive_weights.py
- Tujuan: Menurunkan bobot AHP yang kuat secara metodologis melalui hybrid Entropy × Prior dan menyarankan split makro P vs S berbasis dispersi data. Skrip ini tidak mengubah pipeline QERS; artefaknya dipakai untuk justifikasi akademik dan verifikasi.
- Input:
  - --kpi (default: data/analysis/s1_kpi_table.csv)
  - --quality (default: data/analysis/s1_quality_summary.csv)
- Opsi:
  - --mode {session|reconn|all}: ruang keputusan (normalisasi per-mode untuk sesi terkait).
  - --alpha 0..1: faktor blending Entropy (objektif) vs prior (subjektif), default 0.5.
  - --k-orientation {cost|benefit}, --co-orientation {cost|benefit}: orientasi K/Co untuk perhitungan objektif (eksploratif; QERS utama tetap mengikuti implementasi paper pada layer Fusion).
  - --out (default: data/analysis/weights)
- Output:
  - data/analysis/weights/weights_summary.json: bobot objektif, prior, final (hybrid), dan saran P/S berbasis dispersi.
  - data/analysis/weights/weights_summary.md: ringkasan siap kutip di tesis.
- Cara baca:
  - Performance objective menunjukkan kontribusi informasi masing‑masing metrik (L/E/C/J/Ploss) terhadap pembedaan alternatif di data Anda (via Entropy). Jika Ploss konstan=0, bobot objektifnya→0.
  - Security objective untuk Pr/K/Co/R. Hasil ini kemudian di-blend dengan prior domain menjadi bobot final yang bisa disitasi.
  - Macro weights suggested memberi rasio P/S berbasis dispersi; angka ini memvalidasi pemilihan P≈0.6, S≈0.4 pada data aktual.

Contoh pemakaian:

```bash
# Derive weights untuk session
python tools/derive_weights.py --mode session --alpha 0.5 \
  --k-orientation cost --co-orientation cost \
  --out data/analysis/weights/session

# Derive weights untuk reconnect
python tools/derive_weights.py --mode reconn --alpha 0.5 \
  --k-orientation cost --co-orientation cost \
  --out data/analysis/weights/reconn
```

## Cara Mengutip pada Bab 3/4
- Nyatakan sumber data input (folder CSV), parameter penting (drop_warmup, include_loss), dan alasan metodologis (median p50, tail p95, normalisasi per-mode, bobot dari Entropy × Prior).
- Lampirkan artefak MD/CSV yang dihasilkan sebagai bukti replikasi, dan sebutkan bahwa QERS mengikuti implementasi paper (normalisasi per metrik, penalti performa, dan konstruksi fusi) dengan bobot makro α≈0.6, β≈0.4 yang telah divalidasi oleh derive_weights.
