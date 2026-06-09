# [ARSIP] Data Analysis Plan — Preprocessing dan Analisis (Future Work)

> Status: Diarsipkan. Alur analisis terbaru (end-to-end commands, QERS, dan what‑if) telah dipusatkan di `docs/08-analysis-tools-and-workflows.md`. Dokumen ini dipertahankan karena memuat contoh preprocessing, model usia baterai, dan uji signifikansi yang masih berguna sebagai referensi. Catatan: orientasi QERS saat ini menempatkan K dan Co sebagai COST secara default.

Dokumen ini adalah rencana preprocessing dan analisis data yang akan dilakukan **setelah data Scenario 1 selesai dikumpulkan**. Saat ini (Mei 2026) data Scenario 2 sudah tersedia; dokumen ini disiapkan agar alur analisis bisa langsung dijalankan begitu data S1 tersedia.

---

## 1) Overview Alur Analisis

```
data/csv/          (raw S1+S2)
data/csv_subscriber/ (raw subscriber S1)
        |
        v
[Step 1] Join subscriber CSV ke ESP CSV (S1 only)
        |
        v
data/csv_joined/   (S1 joined)
        |
        v
[Step 2] Preprocessing: load, merge, filter, clean outlier
        |
        v
[Step 3] Descriptive statistics (mean, σ, CV, CI 95%)
        |
        v
[Step 4] Analisis per topik (PIE, reconnect, multi-varian)
        |
        v
[Step 5] Visualisasi (bar chart, box plot, line chart)
        |
        v
[Step 6] QERS Fusion (Scenario 1)
        |
        v
data/analysis/     (tabel markdown, plot, ringkasan)
```

---

## 2) Step 1 — Join CSV Subscriber (Scenario 1)

Sebelum analisis S1, join subscriber CSV ke ESP CSV:

```bash
python3 tools/join_subscriber_csv.py \
  --esp-dir data/csv \
  --sub-dir data/csv_subscriber \
  --out-dir data/csv_joined \
  --scenario s1
```

Output: file `s1_<variant>_<pie>_<reconn|session>_*_joined.csv` di `data/csv_joined/`.
Kolom tambahan: `encaps_us`, `ss_enc_hex`, dan `ss_valid`.

- `ss_enc_hex`: shared secret hasil encaps (hex) dari subscriber.
- `ss_valid`: 1 jika `ss_dec_hex` (ESP) cocok dengan `ss_enc_hex` (subscriber), 0 jika tidak/kurang data.

---

## 3) Step 2 — Preprocessing

### 3.1 Load dan Merge

```python
import pandas as pd
import glob, os

def load_all_csv(pattern, label_cols):
    """Load semua file CSV yang cocok pola, tambah kolom metadata dari nama file."""
    frames = []
    for path in sorted(glob.glob(pattern)):
        name = os.path.basename(path)
        parts = name.split('_')  # s2_512_pieoff_YYYYMMDD_HHMMSS.csv
        df = pd.read_csv(path)
        # Tambah kolom dari nama file jika tidak ada di CSV
        for col, val in zip(label_cols, parts):
            if col not in df.columns:
                df[col] = val
        df['source_file'] = name
        frames.append(df)
    return pd.concat(frames, ignore_index=True)

# Scenario 2
df_s2 = load_all_csv('data/csv/s2_*_pi*.csv', ['s_tag','variant_tag','pie_tag','date_tag','time_tag'])

# Scenario 1 (setelah join)
df_s1 = load_all_csv('data/csv_joined/s1_*_joined.csv', ['s_tag','variant_tag','pie_tag','reconn_tag','date_tag','time_tag'])
```

### 3.2 Warmup Drop

Buang iterasi awal (5 iter pertama) untuk menghindari cold-cache effect:

```python
df_s2 = df_s2[df_s2['iter'] >= 5].copy()
df_s1 = df_s1[df_s1['iter'] >= 5].copy()
```

### 3.3 Filter Packet Loss (Scenario 1)

```python
# Untuk metrik latency dan energy: hanya iterasi sukses
df_s1_clean = df_s1[df_s1['packet_loss'] == 0].copy()

# Simpan packet loss rate sebagai statistik terpisah
pkt_loss_rate = df_s1.groupby(['variant','pie_tag','reconn_tag'])['packet_loss'].mean()
```

### 3.4 Outlier Removal

Gunakan Z-score per grup untuk deteksi outlier pada metrik numerik:

```python
import numpy as np
from scipy import stats

def remove_outliers_zscore(df, group_cols, value_col, threshold=3.0):
    mask = df.groupby(group_cols)[value_col].transform(
        lambda x: np.abs(stats.zscore(x)) < threshold
    )
    return df[mask]

# Untuk energy (CV ~9%, outlier bisa terjadi karena INA219 spike)
for col in ['energy_uj', 'cycles', 'time_us']:
    df_s2 = remove_outliers_zscore(df_s2, ['variant','op','pie_tag'], col)
```

Catatan: cycles memiliki CV ~0.3% sehingga outlier sangat jarang. Energy (CV ~9%) lebih rentan terhadap spike INA219.

### 3.5 Unit Normalization

Semua metrik sudah dalam unit yang konsisten dari firmware:
- `cycles`: raw CPU cycle count (tidak perlu normalisasi)
- `time_us`: mikrodetik (µs)
- `energy_uj`: mikrojoule (µJ)

---

## 4) Step 3 — Descriptive Statistics

```python
def compute_stats(df, group_cols, value_col):
    """Hitung mean, sd, CV, CI 95%, min, max, p50, p95 per grup."""
    def agg(x):
        n = len(x)
        m = x.mean()
        s = x.std(ddof=1)
        se = s / np.sqrt(n)
        return pd.Series({
            'n': n,
            'mean': m,
            'sd': s,
            'cv_pct': s/m*100 if m != 0 else 0,
            'ci95_lower': m - 1.96*se,
            'ci95_upper': m + 1.96*se,
            'ci95_width_pct': 1.96*se/m*100 if m != 0 else 0,
            'min': x.min(),
            'p50': x.quantile(0.50),
            'p95': x.quantile(0.95),
            'max': x.max(),
        })
    return df.groupby(group_cols)[value_col].apply(agg).reset_index()

# Scenario 2: stats per (variant, op, pie_tag)
s2_stats = {}
for col in ['cycles', 'time_us', 'energy_uj']:
    s2_stats[col] = compute_stats(df_s2, ['variant','op','pie_tag'], col)

# Scenario 1: stats per (variant, reconn_tag, pie_tag)
s1_stats = {}
for col in ['keygen_us', 'decaps_us', 'handshake_us', 'energy_uj', 'net_latency_us', 'jitter_us']:
    s1_stats[col] = compute_stats(df_s1_clean, ['variant','reconn_tag','pie_tag'], col)
```

Laporan harus mencakup: **mean ± σ, CV (%), 95% CI, n** untuk semua metrik utama.

---

## 5) Step 4 — Analisis per Topik

### 5.1 PIE Speedup Analysis (S2 + S1)

```python
def compute_speedup(stats_df, group_cols, metric_col, baseline='pieoff', optimized='pieon'):
    pivot = stats_df.pivot_table(
        index=[c for c in group_cols if c != 'pie_tag'],
        columns='pie_tag',
        values=metric_col
    )
    pivot['speedup'] = pivot[baseline] / pivot[optimized]
    pivot['delta_pct'] = (pivot[optimized] - pivot[baseline]) / pivot[baseline] * 100
    return pivot.reset_index()

# Contoh: speedup cycles untuk S2
pie_speedup = compute_speedup(s2_stats['cycles'], ['variant','op','pie_tag'], 'mean')
```

Hasil yang dicari per varian per op:
- Speedup cycles (>1 = PIE lebih cepat)
- Delta% time_us
- Delta% energy_uj

### 5.2 Reconnect Study (S1)

```python
# Bandingkan reconn vs session per (variant, pie_tag)
def compare_reconn_session(df_clean, metric_col):
    stats = compute_stats(df_clean, ['variant','pie_tag','reconn_tag'], metric_col)
    pivot = stats.pivot_table(
        index=['variant','pie_tag'],
        columns='reconn_tag',
        values='mean'
    )
    pivot['overhead_reconn'] = pivot['reconn'] - pivot['session']
    pivot['overhead_pct'] = pivot['overhead_reconn'] / pivot['session'] * 100
    return pivot.reset_index()

reconn_energy = compare_reconn_session(df_s1_clean, 'energy_uj')
reconn_latency = compare_reconn_session(df_s1_clean, 'handshake_us')
```

Pertanyaan utama: apakah overhead `reconn` vs `session` signifikan? Jika overhead >10%, session resumption sangat direkomendasikan.

### 5.3 Multi-Varian Tradeoff (S2)

```python
# Hitung rasio relatif terhadap varian 512
pivot_variants = df_s2.groupby(['variant','op','pie_tag'])['energy_uj'].mean().reset_index()
base_512 = pivot_variants[pivot_variants['variant']=='ML-KEM-512'].set_index(['op','pie_tag'])['energy_uj']
pivot_variants['ratio_vs_512'] = pivot_variants.apply(
    lambda r: r['energy_uj'] / base_512.get((r['op'], r['pie_tag']), 1), axis=1
)
```

### 5.4 Statistical Significance Test (Opsional)

Untuk konfirmasi PIE speedup signifikan secara statistik:

```python
from scipy.stats import mannwhitneyu

def test_significance(df, group_col, val_col, g1, g2):
    x = df[df[group_col]==g1][val_col].dropna()
    y = df[df[group_col]==g2][val_col].dropna()
    stat, p = mannwhitneyu(x, y, alternative='two-sided')
    return {'stat': stat, 'p_value': p, 'significant': p < 0.05}

# Test: apakah PIE on secara signifikan berbeda dari PIE off?
result = test_significance(
    df_s2[(df_s2['variant']=='ML-KEM-512') & (df_s2['op']=='keygen')],
    'pie_tag', 'cycles', 'pieoff', 'pieon'
)
```

Mann-Whitney U digunakan karena tidak mengasumsikan distribusi normal (lebih robust untuk distribusi non-normal seperti energy dengan ekor kanan).

---

## 6) Step 5 — Model Estimasi Usia Baterai

Dari data `energy_uj` per handshake (S1 reconn mode = full auth per sesi):

```python
def battery_lifetime_hours(energy_per_op_uj, frequency_per_hour, battery_mah, voltage_v=3.3):
    battery_uj = battery_mah * voltage_v * 3600 * 1000  # mAh → µJ
    energy_per_hour = energy_per_op_uj * frequency_per_hour
    return battery_uj / energy_per_hour

# Skenario: smart sensor baterai 1000 mAh, 1 handshake per menit
mean_energy = reconn_energy.loc[0, 'reconn']  # contoh µJ
freq = 60  # per jam
print(f"Estimasi usia baterai: {battery_lifetime_hours(mean_energy, freq, 1000):.1f} jam")
```

Sajikan dalam tabel untuk tiga varian + dua skenario deployment (low frequency 1/menit dan high frequency 1/detik).

---

## 7) Step 6 — QERS Fusion

Detail rumus ada di `docs/06-experiment-plan.md`. Implementasi:

```python
def minmax_norm(series, is_cost=True):
    mn, mx = series.min(), series.max()
    if mx == mn:
        return pd.Series([50.0]*len(series), index=series.index)
    normalized = (series - mn) / (mx - mn) * 100
    return (100 - normalized) if is_cost else normalized

def compute_qers(df_s1_agg, weights_perf, weights_sec, alpha=0.6, beta=0.4):
    """
    df_s1_agg: DataFrame dengan kolom metrik QERS, satu baris per (variant, pie, reconn).
    """
    # Performance (cost criteria)
    perf_score = (
        weights_perf['L'] * minmax_norm(df_s1_agg['handshake_us'], is_cost=True) +
        weights_perf['J'] * minmax_norm(df_s1_agg['jitter_us'], is_cost=True) +
        weights_perf['P'] * minmax_norm(df_s1_agg['packet_loss_rate'], is_cost=True) +
        weights_perf['E'] * minmax_norm(df_s1_agg['energy_uj'], is_cost=True) +
        weights_perf['C'] * minmax_norm(df_s1_agg['cpu_util_percent'], is_cost=True)
    )
    # Security (mix cost/benefit)
    sec_score = (
        weights_sec['K'] * minmax_norm(df_s1_agg['key_size'], is_cost=False) +
        weights_sec['R'] * minmax_norm(df_s1_agg['rssi_dbm'], is_cost=False) +
        weights_sec['Pr'] * minmax_norm(df_s1_agg['proven_resistance'], is_cost=False) +
        weights_sec['Co'] * minmax_norm(df_s1_agg['crypto_overhead_bytes'], is_cost=True)
    )
    fusion = alpha * (100 - perf_score) + beta * sec_score
    return perf_score, sec_score, fusion
```

---

## 8) Step 7 — Visualisasi

### Plot yang direncanakan

| Plot | Tipe | Data | Tujuan |
|------|------|------|--------|
| PIE speedup per op | Grouped bar + error bars | S2: cycles/time_us/energy_uj PIE off vs on | Perbandingan PIE |
| Multi-varian scaling | Line chart | S2: mean cycles/time_us vs variant (512/768/1024) | Tradeoff keamanan-performa |
| Energy distribution | Box plot | S2: energy_uj per op per varian | Distribusi + outlier visual |
| Handshake breakdown | Stacked bar | S1: keygen_us + net_latency_us + decaps_us | Dekomposisi RTT |
| Reconnect overhead | Grouped bar | S1: energy_uj reconn vs session | Overhead autentikasi per sesi |
| QERS radar chart | Radar/spider | S1: QERS per varian | Trade-off performa-keamanan |
| 95% CI plot | Error bar | S2: mean ± CI cycles per op | Konfirmasi statistical rigour |

### Library yang direkomendasikan

```python
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# Set style konsisten
sns.set_theme(style='whitegrid', palette='colorblind')
plt.rcParams['figure.dpi'] = 150
plt.rcParams['font.size'] = 11
```

---

## 9) Checklist Sebelum Mulai Analisis

- [ ] Semua 12 run Scenario 1 selesai dikumpulkan (6 ESP CSV + 6 subscriber CSV untuk `reconn`, sama untuk `session`)
- [ ] `join_subscriber_csv.py` sudah dijalankan → `data/csv_joined/` terisi
- [ ] FreeRTOS tick 1000 Hz dikonfirmasi saat semua run S1
- [ ] INA219 sample period identik antar run (default 2ms)
- [ ] Tidak ada run yang terpotong (header CSV + 100 iterasi lengkap)
- [ ] Varian, PIE mode, dan reconnect tag di nama file sesuai konfigurasi aktual

---

## 10) Output yang Diharapkan

Setelah analisis selesai:

- `data/analysis/s2_summary_stats.csv` — statistik deskriptif S2 semua varian
- `data/analysis/s1_summary_stats.csv` — statistik deskriptif S1 semua varian
- `data/analysis/pie_speedup_s2.md` — tabel speedup PIE untuk S2
- `data/analysis/pie_speedup_s1.md` — tabel speedup PIE untuk S1
- `data/analysis/reconnect_comparison.md` — tabel overhead reconn vs session
- `data/analysis/variant_scaling.md` — kurva scaling 512/768/1024
- `data/analysis/qers_fusion.md` — skor QERS per konfigurasi
- `data/analysis/battery_model.md` — estimasi usia baterai
- `data/analysis/plots/` — folder gambar (PNG/PDF)

---

## 11) Rencana Analisis Scenario 1 — Detail Lengkap

### 11.1 Struktur CSV Terbaru (setelah perbaikan kode)

Header CSV S1 (ESP, `data/csv/`):

```
scenario,variant,iter,keygen_cycles,decaps_cycles,keygen_us,decaps_us,handshake_us,net_latency_us,jitter_us,energy_uj,reconnect_us,reconnect_energy_uj,rssi_dbm,packet_loss,ss_dec_hex,pub_bytes,ct_bytes,overhead_bytes,heap_free,heap_min,stack_hwm_bytes,cpu_util_percent
```

Header CSV S1 joined (`data/csv_joined/`):

```text
[header ESP di atas] + encaps_us,ss_enc_hex,ss_valid
```

Kolom baru:
- `reconnect_us`: waktu MQTT disconnect+reconnect per iterasi (µs). Selalu 0 untuk mode session.
- `reconnect_energy_uj`: energi fisik INA219 selama window reconnect (µJ). Selalu 0 untuk mode session.
- `ss_dec_hex`: shared secret hasil decaps (hex). Kosong untuk iterasi gagal.

### 11.2 Validitas Tiap Metric — Reconn vs Session

| Metric | Mode | Validitas | Catatan |
|--------|------|-----------|---------|
| `keygen_us` | session | ✅ Bersih | ISR minimal, CV <1% |
| `keygen_us` | reconn | ⚠️ Terpengaruh ISR | TCP/IP ISR dari reconnect mencemari cycle count; CV ~10%. Interpretasi: "keygen latency under reconnect load" |
| `keygen_cycles` | session | ✅ Bersih | Stabil |
| `keygen_cycles` | reconn | ⚠️ Terpengaruh ISR | Cycle counter Xtensa LX7 terus berjalan selama ISR |
| `decaps_us` | kedua | ✅ Valid | Terjadi setelah koneksi stabil + settle delay 50ms; CV <3% |
| `decaps_cycles` | kedua | ✅ Valid | Konsisten antar mode |
| `handshake_us` | kedua | ✅ Valid | Window sama: start setelah reconnect+settle, end setelah decaps |
| `net_latency_us` | kedua | ✅ Valid | Murni network RTT dari publish sampai ciphertext diterima |
| `jitter_us` | kedua | ✅ Valid | \|latency_N - latency_N-1\|; iter=0 selalu 0 |
| `energy_uj` | kedua | ✅ Valid | Window keygen+publish+wait+decaps; reconnect TIDAK termasuk |
| `reconnect_us` | reconn | ✅ Valid (baru) | Waktu satu siklus disconnect+reconnect+SUBACK |
| `reconnect_us` | session | 0 (by design) | Tidak ada reconnect |
| `reconnect_energy_uj` | reconn | ✅ Valid (baru) | Energi satu siklus reconnect diukur INA219 |
| `reconnect_energy_uj` | session | 0 (by design) | Tidak ada reconnect |
| `cpu_util_percent` | session | ✅ Valid | (keygen_us + decaps_us) / handshake_us × 100 |
| `cpu_util_percent` | reconn | ⚠️ Bias | Inflated karena keygen_us tercemar ISR |
| `packet_loss` | kedua | ✅ Valid | Biner: timeout / iter_id mismatch / payload invalid |
| `encaps_us` (subscriber) | kedua | ✅ Valid | Tidak dipengaruhi mode koneksi ESP32 |

### 11.3 Rumus Perhitungan Tiap Metric

```
keygen_us       = esp_timer_get_time() setelah keypair - sebelum keypair
keygen_cycles   = mlkem_cycle_diff(c_before, c_after)  [wrapping-safe]
decaps_us       = esp_timer_get_time() setelah decaps - sebelum decaps
decaps_cycles   = mlkem_cycle_diff(c_before, c_after)
handshake_us    = t_handshake_end - t_handshake_start
                  window: [setelah settle delay] → [setelah decaps selesai]
net_latency_us  = t_ciphertext_received - t_publish
jitter_us       = |net_latency_iter_N - net_latency_iter_(N-1)|
                  (= 0 untuk iter pertama setiap run)
energy_uj       = INA219 integrasi daya × Δt, window handshake saja
reconnect_us    = t_after_mqtt_connected - t_before_disconnect  [reconn mode]
reconnect_energy_uj = INA219 integrasi daya × Δt, window reconnect saja  [reconn mode]
cpu_util_percent = (keygen_us + decaps_us) / handshake_us × 100
total_energy_uj = energy_uj + reconnect_energy_uj  [turunan, hitung offline]
total_latency_us = handshake_us + reconnect_us     [turunan, hitung offline]
```

### 11.4 Yang Dapat Dibandingkan Reconn vs Session

#### A. Perbandingan VALID (metodologi identik)

| Metric | Interpretasi Perbandingan |
|--------|--------------------------|
| `decaps_us` | Pure crypto decaps cost; bersih di kedua mode |
| `encaps_us` | Pure crypto encaps cost di subscriber; identik |
| `handshake_us` | End-to-end RTT per iterasi (minus reconnect) |
| `net_latency_us` | Network RTT murni |
| `jitter_us` | Stabilitas jaringan per mode |
| `packet_loss rate` | Reliabilitas pengiriman |
| `energy_uj` | Energi handshake murni (tanpa reconnect) |
| `reconnect_us` (reconn) | Overhead waktu per koneksi baru |
| `reconnect_energy_uj` (reconn) | Overhead energi per koneksi baru |

#### B. Turunan untuk Reconnect Study

```python
# Total energy cost per "full operation cycle" di mode reconn:
df_s1['total_energy_uj'] = df_s1['energy_uj'] + df_s1['reconnect_energy_uj']

# Overhead reconnect relatif terhadap handshake:
df_s1['reconnect_energy_pct'] = (
    df_s1['reconnect_energy_uj'] / df_s1['total_energy_uj'] * 100
)
df_s1['reconnect_time_pct'] = (
    df_s1['reconnect_us'] / (df_s1['handshake_us'] + df_s1['reconnect_us']) * 100
)
```

#### C. Perbandingan TIDAK LANGSUNG (perlu catatan metodologi)

| Metric | Masalah | Cara Mitigasi |
|--------|---------|---------------|
| `keygen_us` reconn vs session | ISR TCP/IP mencemari reconn | Laporkan sebagai "keygen under load"; bandingkan dengan `keygen_cycles` dari S2 sebagai baseline bersih |
| `cpu_util_percent` reconn vs session | Inflated karena keygen_us tercemar | Gunakan session value sebagai referensi; reconn nilai hanya indikatif |

### 11.5 Analisis Reconnect Study — Step-by-Step

```python
import pandas as pd
import numpy as np

# Load semua S1 joined CSV
df = pd.read_csv(...)  # sudah dipreprocess sesuai Step 2-3

# Pisahkan mode
df_reconn  = df[df['reconn_tag'] == 'reconn'].copy()
df_session = df[df['reconn_tag'] == 'session'].copy()

# Hitung total energy untuk reconn
df_reconn['total_energy_uj'] = (
    df_reconn['energy_uj'] + df_reconn['reconnect_energy_uj']
)

# Statistik per konfigurasi (variant × PIE)
group_cols = ['variant_tag', 'pie_tag']

def agg_metrics(df, extra_cols=[]):
    cols = ['handshake_us', 'energy_uj', 'net_latency_us',
            'jitter_us', 'decaps_us', 'keygen_us'] + extra_cols
    return df.groupby(group_cols)[cols].agg(['mean', 'median', 'std']).round(1)

stats_reconn  = agg_metrics(df_reconn,  extra_cols=['reconnect_us', 'reconnect_energy_uj', 'total_energy_uj'])
stats_session = agg_metrics(df_session)

# Tabel overhead reconnect
overhead = pd.DataFrame()
for g, grp_r in df_reconn.groupby(group_cols):
    grp_s = df_session[
        (df_session['variant_tag'] == g[0]) &
        (df_session['pie_tag']     == g[1])
    ]
    overhead = pd.concat([overhead, pd.DataFrame({
        'variant': [g[0]],
        'pie':     [g[1]],
        'reconn_mean_reconnect_us':     [grp_r['reconnect_us'].mean()],
        'reconn_mean_reconnect_energy': [grp_r['reconnect_energy_uj'].mean()],
        'reconn_mean_total_energy':     [grp_r['total_energy_uj'].mean()],
        'session_mean_energy':          [grp_s['energy_uj'].mean()],
        'overhead_energy_uj':           [grp_r['reconnect_energy_uj'].mean()],
        'overhead_energy_pct':          [grp_r['reconnect_energy_uj'].mean() /
                                         grp_r['total_energy_uj'].mean() * 100],
    })])

overhead.to_markdown('data/analysis/reconnect_comparison.md', index=False)
```

**Pertanyaan riset utama:**
- Berapa overhead energi satu MQTT reconnect vs satu handshake? (angka absolut dan %)
- Apakah overhead >10%? Jika ya, session resumption sangat direkomendasikan untuk perangkat baterai.
- Apakah `energy_uj` (handshake only) berbeda antara reconn dan session? (seharusnya tidak, perbedaan = network variance)

### 11.6 Analisis QERS — Mapping ke CSV Columns

QERS menggunakan **agregat per konfigurasi** (satu baris per variant × PIE × reconnect_mode):

```python
# Agregasi S1 per konfigurasi
df_s1_clean = df[df['packet_loss'] == 0]  # buang packet loss
df_s1_clean = df_s1_clean[df_s1_clean['iter'] >= 5]  # buang warmup

df_s1_agg = df_s1_clean.groupby(['variant_tag', 'pie_tag', 'reconn_tag']).agg(
    handshake_us      = ('handshake_us',       'median'),
    jitter_us         = ('jitter_us',          'median'),
    energy_uj         = ('energy_uj',          'median'),
    cpu_util_percent  = ('cpu_util_percent',   'median'),
    rssi_dbm          = ('rssi_dbm',           'mean'),
    packet_loss_rate  = ('packet_loss',        'mean'),   # dari df asli sebelum filter
).reset_index()

# Tambah kolom konstanta/metadata per varian
KEY_SIZE_MAP       = {'ML-KEM-512': 512,  'ML-KEM-768': 768,  'ML-KEM-1024': 1024}
PROVEN_RES_MAP     = {'ML-KEM-512': 1,    'ML-KEM-768': 3,    'ML-KEM-1024': 5}
df_s1_agg['crypto_overhead_bytes'] = (
    df_s1_clean.groupby(['variant_tag', 'pie_tag', 'reconn_tag'])
    .apply(lambda g: (g['pub_bytes'] + g['ct_bytes'] + g['overhead_bytes']).median())
    .reset_index(drop=True)
)

df_s1_agg['key_size']           = df_s1_agg['variant_tag'].map(KEY_SIZE_MAP)
df_s1_agg['proven_resistance']  = df_s1_agg['variant_tag'].map(PROVEN_RES_MAP)
```

**Bobot QERS default (dapat diubah):**

| Komponen | Metric | Tipe | Bobot |
|----------|--------|------|-------|
| L (Latency) | `handshake_us` | cost | 0.25 |
| J (Jitter) | `jitter_us` | cost | 0.15 |
| P (Packet Loss) | `packet_loss_rate` | cost | 0.20 |
| E (Energy) | `energy_uj` | cost | 0.25 |
| C (CPU Util) | `cpu_util_percent` | cost | 0.15 |
| K (Key Size) | konstanta per varian | benefit | 0.30 |
| R (RSSI) | `rssi_dbm` | benefit | 0.20 |
| Pr (Proven Resistance) | konstanta level | benefit | 0.30 |
| Co (Crypto Overhead) | `pub_bytes+ct_bytes+overhead` | cost | 0.20 |

**Catatan penting untuk QERS:**
- Normalisasi min-max harus dihitung **lintas semua konfigurasi sekaligus** (semua varian 512/768/1024, semua PIE, semua reconn/session dalam satu DataFrame).
- Jika data varian 768 dan 1024 belum tersedia, QERS sementara tidak valid — tunggu semua 12 run S1 selesai.
- Dataset menyediakan `reconnect_us`/`reconnect_energy_uj` sehingga QERS bisa memakai handshake-only atau total (handshake+reconnect). Keputusan final mengikuti definisi di paper QERS primer dan harus dinyatakan eksplisit di Bab 4.

### 11.7 Analisis PIE Study untuk S1

```python
# Bandingkan pieoff vs pieon per (variant, reconn_mode)
pie_group = ['variant_tag', 'reconn_tag']

def pie_comparison_s1(df, metric):
    g = df.groupby(pie_group + ['pie_tag'])[metric].median().unstack('pie_tag')
    g['delta_pct'] = (g['pieon'] - g['pieoff']) / g['pieoff'] * 100
    g['speedup']   = g['pieoff'] / g['pieon']
    return g.round(2)

for m in ['keygen_us', 'decaps_us', 'handshake_us', 'energy_uj']:
    print(f"\n=== PIE effect on {m} ===")
    print(pie_comparison_s1(df_s1_clean, m).to_markdown())
```

**Catatan:** Untuk PIE study di S1, efek PIE pada `keygen_us` lebih noise di reconn mode (ISR contamination). Gunakan session mode sebagai referensi utama untuk PIE effect on keygen.

### 11.8 Checklist Sebelum Jalankan Ulang S1

- [ ] Firmware di-build ulang dengan kolom baru (`reconnect_us`, `reconnect_energy_uj`)
- [ ] Jalankan semua 12 run dalam satu sesi waktu berdekatan (kondisi jaringan seragam)
- [ ] Urutan run: 512-pieoff-reconn → 512-pieoff-session → 512-pieon-reconn → 512-pieon-session → (768) → (1024)
- [ ] Subscriber di-start dengan `--mode reconn` atau `--mode session` sesuai run
- [ ] Verifikasi header CSV pertama setiap run memiliki kolom `reconnect_us`
- [ ] Tidak ada run yang terpotong (100 iterasi lengkap + header)
- [ ] Catat waktu mulai dan selesai tiap run untuk korelasi kondisi jaringan

### 11.9 Preprocessing Tambahan untuk Kolom Baru

```python
# Setelah load df_s1 (dari csv_joined):

# Pastikan kolom baru ada (backward compat dengan data lama jika diperlukan)
if 'reconnect_us' not in df_s1.columns:
    df_s1['reconnect_us'] = 0
if 'reconnect_energy_uj' not in df_s1.columns:
    df_s1['reconnect_energy_uj'] = 0

# Total energy per siklus penuh
df_s1['total_energy_uj'] = df_s1['energy_uj'] + df_s1['reconnect_energy_uj']

# Flag: apakah ini reconn mode?
df_s1['is_reconn'] = (df_s1['reconn_tag'] == 'reconn').astype(int)

# Buang iter 0 saja untuk jitter (iter 0 selalu jitter=0 by design)
df_s1_jitter = df_s1[df_s1['iter'] > 0].copy()
```

