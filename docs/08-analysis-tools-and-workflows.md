# Panduan Analisis QERS: Workflow dan Tools

Dokumen ini memadukan referensi alat analisis dan tahapan langkah-demi-langkah untuk mereproduksi hasil QERS, melakukan what‑if analysis (perubahan bobot, orientasi metrik), dan menyiapkan artefak untuk pelaporan.

- Default penting saat ini:
  - Orientasi `K` (ukuran kunci/payload) dan `Co` (cryptographic overhead) = COST secara default.
  - Bobot makro AHP: `α=0.6` (Performance), `β=0.4` (Security).
  - Kebijakan prior keamanan terbaru (di `tools/derive_weights.py`): `Pr=0.50, K=0.20, Co=0.20, R=0.10`.

## Tahapan Analisis (End-to-End)

- **[1] Persiapan data**
  - Pastikan berkas berikut tersedia (sudah disiapkan oleh pipeline analisis):
    - `data/analysis/s1_kpi_table.csv`
    - `data/analysis/s1_quality_summary.csv`
  - Jika perlu regenerasi ringkasan KPI/quality, gunakan skrip analisis kampanye sesuai dokumentasi eksperimen (opsional, tidak dibahas rinci di sini).

- **[2] Turunkan bobot (justifikasi akademik) dengan `tools/derive_weights.py`**
  - Tujuan: hitung bobot objektif (Entropy) dan gabungkan dengan prior kebijakan untuk dokumentasi/justifikasi metodologis. Artefak ini tidak otomatis dipakai `compute_qers.py`, tetapi dapat diselaraskan manual via override bobot.
  - Contoh per-mode (disarankan normalisasi per-mode di tahap QERS):

```bash
# Session mode
python tools/derive_weights.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --quality data/analysis/s1_quality_summary.csv \
  --mode session --alpha 0.5 \
  --out data/analysis/weights

# Reconnect mode
python tools/derive_weights.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --quality data/analysis/s1_quality_summary.csv \
  --mode reconn --alpha 0.5 \
  --out data/analysis/weights
```

  - Hasil yang diharapkan:
    - `data/analysis/weights/session/weights_summary.json` dan `.md`
    - `data/analysis/weights/reconn/weights_summary.json` dan `.md`
  - Validasi cepat:
    - Pastikan bagian prior keamanan merefleksikan kebijakan terbaru `Pr=0.50, K=0.20, Co=0.20, R=0.10`.
    - Catat rekomendasi split makro P/S berbasis dispersi sebagai bahan pelaporan.

- **[3] Hitung QERS default (K/Co = COST) dengan `tools/compute_qers.py`**
  - Tujuan: menghasilkan skor QERS, penalti performa `P`, skor keamanan `S`, dan peringkat.
  - Gunakan normalisasi per-mode agar adil terhadap ruang keputusan yang berbeda.

```bash
# Session (default sub-bobot bawaan skrip)
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session \
  --normalize-within-mode --mode session

# Reconnect (default sub-bobot bawaan skrip)
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_reconn \
  --normalize-within-mode --mode reconn
```

  - Hasil yang diharapkan (per direktori out-dir):
    - `qers_weights.md` (ringkasan bobot dan orientasi metrik)
    - `qers_fusion_scores.csv` dan `qers_fusion_scores.md` (P, S, QERS_fusion, ranking)

- **[3b] Hitung QERS dengan bobot dari `derive_weights.py` (disarankan)**
  - Tujuan: memakai sub-bobot final (EWM+Prior) dari `weights_summary.json` agar sensitif pada data; makro `alpha/beta` bisa diambil dari JSON atau di-override.

```bash
# Session dengan bobot dari derive + makro kebijakan 0.6/0.4
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session \
  --normalize-within-mode --mode session \
  --weights-json data/analysis/weights/session/weights_summary.json \
  --alpha 0.6 --beta 0.4

# Reconnect dengan bobot dari derive + makro kebijakan 0.6/0.4
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_reconn \
  --normalize-within-mode --mode reconn \
  --weights-json data/analysis/weights/reconn/weights_summary.json \
  --alpha 0.6 --beta 0.4
```

  - Catatan:
    - Prioritas pemuatan bobot: CLI override (`--w-perf/--w-sec/--alpha/--beta`) > JSON (`--weights-json`) > default AHP skrip.
    - Rumus tetap sama: `P = Σ w_perf·norm`, `S = Σ w_sec·norm`, `QERS_fusion = α·(100−P) + β·S`.

- **[4] Selaraskan QERS dengan kebijakan prior keamanan terbaru (override sub-bobot Security)**
  - Agar konsisten dengan prior `Pr=0.50, K=0.20, Co=0.20, R=0.10`, lakukan override `--w-sec`:

```bash
# Session dengan prior Security terbaru
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session_pr50_k20_co20 \
  --normalize-within-mode --mode session \
  --w-sec "Pr=0.5,K=0.2,R=0.1,Co=0.2"

# Reconnect dengan prior Security terbaru
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_reconn_pr50_k20_co20 \
  --normalize-within-mode --mode reconn \
  --w-sec "Pr=0.5,K=0.2,R=0.1,Co=0.2"
```

  - Catatan:
    - `K`/`Co` sudah COST sebagai default; Anda bisa menegaskan ulang dengan `--k-orientation cost --co-orientation cost` bila perlu.

- **[5] Sensitivitas (opsional, untuk pembahasan Bab 4)**
  - Dominan K/Co (menguji penalti ukuran lebih kuat):

```bash
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session_kcodominant \
  --normalize-within-mode --mode session \
  --w-sec "Pr=0.3,K=0.35,R=0.05,Co=0.3"
```

  - Perf dominan L/E:

```bash
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session_perfLE \
  --normalize-within-mode --mode session \
  --w-perf "L=0.5,E=0.3,C=0.1,J=0.05,Ploss=0.05"
```

  - CPU heavy (contoh variasi performa):

```bash
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session_cpuheavy \
  --normalize-within-mode --mode session \
  --w-perf "L=0.35,E=0.25,C=0.3,J=0.05,Ploss=0.05"
```

- **[6] Membaca dan menafsirkan hasil**
  - Buka `qers_fusion_scores.csv` untuk melihat setiap alternatif beserta kolom P_penalty, S_security, QERS_fusion, dan ranking.
  - Tren yang umum (berdasarkan data S1):
    - Default COST pada K/Co menurunkan `S` untuk varian ber-artefak besar; jika `P` (kinerja) sangat dominan, peringkat bisa tetap tak berubah.
    - Membesarkan bobot K/Co dalam `--w-sec` akan menggeser peringkat ke varian artefak lebih kecil bila selisih `P` tidak terlalu besar.

## Referensi Alat dan Opsi CLI

### tools/compute_qers.py
- Tujuan: Hitung skor QERS (Fusion) untuk konfigurasi S1 dari KPI table.
- Input: `--kpi` (contoh: `data/analysis/s1_kpi_table.csv`)
- Opsi CLI utama:
  - `--out-dir` (direktori hasil)
  - `--normalize-within-mode` (disarankan) dan `--mode {session|reconn|all}`
  - `--weights-json path/to/weights_summary.json` untuk memuat sub-bobot `perf_final`/`sec_final` dan makro `P/S` dari keluaran `tools/derive_weights.py`
  - `--alpha`, `--beta` untuk bobot makro Fusion
  - `--w-perf`, `--w-sec` override sub-bobot
  - `--k-orientation {cost|benefit}`, `--co-orientation {cost|benefit}` (default: cost)
- Proses inti (ringkas):
  - Jika `--weights-json` diberikan, `w_perf/w_sec` diambil dari JSON (`perf_final`/`sec_final`) dan `alpha/beta` dari `macro_weights_suggested` (bisa di-override CLI).
  - Normalisasi min–maks ke skala 0..100 per metrik.
  - Performance penalty `P` = Σ w_perf[j] · norm_j untuk L, J, Ploss, E, C (semua sebagai biaya melalui penalti).
  - Security `S` = Σ w_sec[k] · norm_k untuk Pr, K, R, Co (K/Co = COST default).
  - Skor `QERS_fusion = α·(100 − P) + β·S` (default α≈0.6, β≈0.4).
- Output:
  - `qers_weights.md`, `qers_fusion_scores.{csv,md}` per `out-dir`.

### tools/derive_weights.py
- Tujuan: Menurunkan bobot AHP via hybrid Entropy × Prior dan menyarankan split makro P vs S berbasis dispersi data. Artefaknya dipakai untuk justifikasi akademik dan verifikasi.
- Input:
  - `--kpi` (default: `data/analysis/s1_kpi_table.csv`)
  - `--quality` (default: `data/analysis/s1_quality_summary.csv`)
- Opsi penting:
  - `--mode {session|reconn|all}`
  - `--alpha` 0..1: blending Entropy (objektif) vs prior (subjektif), default 0.5
  - `--k-orientation {cost|benefit}`, `--co-orientation {cost|benefit}` (eksploratif untuk perhitungan objektif; QERS utama tetap mengikuti implementasi di `compute_qers.py`)
  - `--out` (default: `data/analysis/weights`)
- Output:
  - `data/analysis/weights/<mode>/weights_summary.json` dan `.md` berisi bobot objektif, prior, final (hybrid), CR, serta saran bobot makro.

### tools/qers_basic_tuned/compute_qers_bt.py
- Tujuan: Hitung QERS Basic dan QERS Tuned sesuai formula paper, dengan pembobotan ala `derive_weights.py` (Entropy Weight Method + Prior + blending α), disesuaikan untuk set metrik masing-masing formula dan memastikan jumlah bobot = 1.
- Input: `--kpi` (contoh: `data/analysis/s1_kpi_table.csv`)
- Opsi CLI utama:
  - `--formula {basic|tuned}` untuk memilih rumus
  - `--mode {session|reconn|all}` dan `--normalize-within-mode` (default per-mode)
  - `--ms` (default 100) skala normalisasi
  - `--alpha-basic`, `--prior-basic "L=...,O=...,Ploss=..."` (jumlah = 1) untuk Basic
  - `--alpha-tuned`, `--prior-tuned "L=...,O=...,Ploss=...,C=...,E=...,K=...,R=..."` (jumlah = 1) untuk Tuned
  - `--k-orientation {cost|benefit}`, `--o-orientation {cost|benefit}` (default: cost)
- Proses inti (ringkas):
  - Normalisasi min–maks 0..1 (benefit-oriented) untuk perhitungan bobot objektif (EWM), hanya pada metrik yang relevan di formula:
    - Basic keys = [L, O(=Co), Ploss]
    - Tuned keys = [L, O(=Co), Ploss, C, E, K, R]
  - Hitung bobot objektif (EWM), blend dengan prior via α → `w_final` (renorm agar jumlah = 1).
  - Hitung skor 0..100: untuk cost gunakan 100 − X_norm (guard rentang nol → 0); untuk benefit gunakan X_norm.
  - Rumus skor:
    - Basic: `Q_basic = MS − (wL·L + wO·O + wP·Ploss)`
    - Tuned: `Q_tuned = MS − (wL·L + wO·O + wP·Ploss + wC·C + wE·E + wK·K) + wR·R`
- Output (di `--out-dir`):
  - `qers_basic_scores.{csv,md}` atau `qers_tuned_scores.{csv,md}`
  - Ringkasan bobot: `qers_basic_weights.md` atau `qers_tuned_weights.md` (memuat w_obj, prior, α, w_final)
- Contoh pemakaian:
```bash
python tools/qers_basic_tuned/compute_qers_bt.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_basic_tuned \
  --formula tuned \
  --mode reconn \
  --normalize-within-mode \
  --ms 100 \
  --alpha-tuned 0.5 \
  --prior-tuned "L=0.25,O=0.15,Ploss=0.15,C=0.15,E=0.10,K=0.10,R=0.10" \
  --k-orientation cost --o-orientation cost
```

## Catatan Reproduksibilitas
- Simpan perintah yang dijalankan beserta versi kode/commit.
- Pertahankan struktur direktori `data/analysis/...` agar jalur output konsisten dengan dokumen ini.
- Gunakan kembali opsi CLI yang sama saat mengulang analisis untuk menjaga hasil tetap konsisten.
