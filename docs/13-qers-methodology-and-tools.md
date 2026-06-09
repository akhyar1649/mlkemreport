# QERS Methodology and Tools (Basic, Tuned, Fusion)

Dokumen ini merangkum teori QERS (berdasarkan catatan di `data/analysis/qers_paper_notes.md`), serta cara memakai tiga tool analisis QERS di repo ini:
- Fusion (tools/compute_qers.py)
- Weight Derivation untuk Fusion (tools/derive_weights.py)
- Basic/Tuned (tools/qers_basic_tuned/compute_qers_bt.py)

Fokus: normalisasi, pembobotan, rumus skor, serta rekomendasi praktik terbaik.

---

## 1) Pemetaan metrik dan orientasi

- L: total latency (us) → cost
- J: jitter (us) → cost (tidak dipakai di Basic/Tuned menurut paper)
- Ploss: packet loss ratio → cost
- C: CPU utilization (%) → cost
- E: total energy (µJ) → cost
- R: RSSI (dBm) → benefit
- K: payload+ct bytes → cost (default)
- Co (O): overhead bytes → cost (default)
- Pr: proven resistance level (512→1, 768→3, 1024→5) → benefit (Fusion saja)

Catatan:
- Untuk Basic/Tuned, pakai: Basic={L,O,Ploss}, Tuned={L,O,Ploss,C,E,K} dan +R (benefit). Jitter tidak dipakai.
- Untuk Fusion, P_penalty={L,J,Ploss,E,C} dan S_security={Pr,K,R,Co}.

---

## 2) Normalisasi (min–max → 0..100)

Rumus: `X_norm = MS * (X - X_min) / (X_max - X_min)`, dengan `MS=100` (bisa diubah via CLI untuk Basic/Tuned).
- Metrik cost dibalik: `100 - X_norm` bila rentang > 0, jika rentang nol → 0.
- Metrik benefit dibiarkan apa adanya.
- Opsi normalisasi:
  - Per-mode (recommended untuk fairness session vs reconn): min–max dihitung terpisah per `mode`.
  - Global: min–max dihitung untuk seluruh data.

Guard rentang nol (hi==lo): metrik tersebut dianggap tidak informatif untuk skor (norm=0) agar tidak bias.

---

## 3) Pembobotan: konsep umum

Ada dua lapis pembobotan yang digunakan di repo ini.

- Sub-kriteria (di dalam kelompok):
  - Fusion: `w_perf` untuk {L,J,Ploss,E,C}, `w_sec` untuk {Pr,K,R,Co}.
  - Basic/Tuned: bobot langsung di atas kumpulan metrik formula (jumlah = 1).
- Makro (hanya Fusion): `alpha` (Perf) dan `beta` (Sec) dengan `alpha+beta=1`.

Metodologi yang didukung:
- Entropy Weight Method (EWM) → bobot objektif dari dispersi metrik (data-driven).
- Prior → bobot kebijakan yang Anda tentukan.
- Hybrid (subjective–objective) → `w_final = renorm( alpha * w_obj + (1 - alpha) * w_prior )` (blend faktor di sini adalah parameter `alpha` dari tool derive/basic-tuned; jangan bingung dengan `alpha` makro Fusion).
- AHP (Analytic Hierarchy Process) default (hanya compute_qers.py jika tidak memuat JSON/override) → pairwise matrix fixed di kode.

---

## 4) Tool: derive_weights (Fusion)

File: `tools/derive_weights.py`
- Input: `data/analysis/s1_kpi_table.csv`, `data/analysis/s1_quality_summary.csv`.
- Proses:
  1) Normalisasi 0..1 (benefit-oriented, guard nol rentang).
  2) EWM untuk `perf_obj` dan `sec_obj`.
  3) Blend dengan prior via `--alpha` → `perf_final`, `sec_final` (jumlah masing-masing = 1).
  4) Menghitung saran bobot makro P vs S dari dispersi (macro_dispersion): `P = P_disp / (P_disp+S_disp)`.
- Output: `weights_summary.json`, `weights_summary.md`.

Catatan penting:
- Metrik konstan (mis. `Co`=58) → bobot objektif 0, tetapi bobot final tetap ada melalui prior.
- `--alpha` di derive merupakan blend objektif vs prior (BUKAN alpha makro Fusion).

Contoh:
```
python tools/derive_weights.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --quality data/analysis/s1_quality_summary.csv \
  --mode session --alpha 0.3 \
  --out data/analysis/weights/session
```

---

## 5) Tool: compute_qers (Fusion)

File: `tools/compute_qers.py`
- Input: `data/analysis/s1_kpi_table.csv`.
- Pilihan bobot:
  - Default AHP internal (statis) atau
  - Memuat hasil derive: `--weights-json path/to/weights_summary.json` → pakai `perf_final`, `sec_final`, dan `macro_weights_suggested` sebagai alpha/beta; atau
  - Override manual CLI: `--w-perf`, `--w-sec`, `--alpha`, `--beta`.
- Rumus Fusion:
  - `P = Σ w_perf[i] * norm_i`, `S = Σ w_sec[j] * norm_j`.
  - `QERS_fusion = alpha * (MS - P) + beta * S` (dengan `MS=100`).

Rekomendasi:
- Gunakan `--weights-json` dari derive agar sub-bobot sensitif terhadap data, lalu tetapkan `--alpha 0.6 --beta 0.4` (atau preferensi Anda) untuk makro Perf/Sec stabil.
- Aktifkan `--normalize-within-mode` untuk fairness.

Contoh:
```
python tools/compute_qers.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_session \
  --normalize-within-mode --mode session \
  --weights-json data/analysis/weights/session/weights_summary.json \
  --alpha 0.6 --beta 0.4
```

---

## 6) Tool: compute_qers_bt (Basic/Tuned)

File: `tools/qers_basic_tuned/compute_qers_bt.py`
- Input: `data/analysis/s1_kpi_table.csv`.
- Formula:
  - Basic: `Q_basic = MS − (wL·L + wO·O + wP·Ploss)`.
  - Tuned: `Q_tuned = MS − (wL·L + wO·O + wP·Ploss + wC·C + wE·E + wK·K) + wR·R`.
- Pembobotan (ala derive):
  - EWM pada metrik yang relevan (0..1), blend dengan prior via `--alpha-basic/--alpha-tuned` → `w_final` (jumlah = 1).
- Normalisasi skor (0..100) pakai aturan di Bagian 2 saat menghitung QERS.
- Orientasi `K` dan `O` bisa diubah: default cost.

CLI utama:
```
python tools/qers_basic_tuned/compute_qers_bt.py \
  --kpi data/analysis/s1_kpi_table.csv \
  --out-dir data/analysis/qers_basic_tuned \
  --formula {basic|tuned} \
  --mode {session|reconn|all} \
  --normalize-within-mode \
  --ms 100 \
  --alpha-basic 0.5 --prior-basic "L=0.5,O=0.25,Ploss=0.25" \
  --alpha-tuned 0.5 --prior-tuned "L=0.25,O=0.15,Ploss=0.15,C=0.15,E=0.10,K=0.10,R=0.10" \
  --k-orientation cost --o-orientation cost
```

Output:
- Skor: `qers_basic_scores.(csv|md)`, `qers_tuned_scores.(csv|md)`.
- Ringkasan bobot: `qers_basic_weights.md`, `qers_tuned_weights.md` (memuat w_obj, prior, α, w_final).

---

## 7) Rekomendasi praktik terbaik

- Gunakan `--normalize-within-mode` untuk perbandingan adil session vs reconn.
- Pertahankan `K` dan `O` sebagai cost (default) kecuali ada alasan kuat.
- Atur prior agar sesuai kebijakan domain; gunakan `alpha` kecil (mis. 0.3–0.5) jika ingin prior lebih dominan ketika metrik tertentu konstan.
- Untuk Fusion, tetapkan `alpha/beta` makro eksplisit (mis. 0.6/0.4) agar stabil dan mudah ditafsirkan lintas dataset.
- Periksa metrik konstan (mis. `Co`=58) karena akan mematikan bobot objektif; pastikan prior memberikan porsi yang sesuai jika masih ingin memasukkan metrik tersebut.
- Dokumentasikan setelan yang dipakai (prior, alpha, orientasi, normalisasi) di laporan agar hasil reprodusibel.

---

## 8) Sumber & catatan

- Catatan teori: `data/analysis/qers_paper_notes.md`.
- Sumber PDF (RAG): `/home/me/projects/MCP/RAG/data/pdfs` (lihat file yang disebut di `qers_paper_notes.md`).
- Tool terkait lain: `tools/analyze_campaign.py`, `tools/join_subscriber_csv.py` (mempersiapkan KPI input), dan dokumentasi `07-data-analysis-plan.md`/`08-analysis-tools*.md`.
