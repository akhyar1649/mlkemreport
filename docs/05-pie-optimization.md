# PIE Optimization — Deep Dive, Microbench, dan Speedup Blueprint

Dokumen ini menjelaskan semua aspek optimasi PIE: detail teknis PIE normal vs aggressive, microbench kernel-level, dan rencana speedup lanjutan.

---

# Bagian 1: PIE Optimization Deep Dive (Normal vs Aggressive)

## 1) Konteks: Di Mana Optimasi Terjadi?

ML-KEM banyak menghabiskan waktu pada operasi polinomial di ring R_q, terutama:

- **NTT** (`ntt`) — transformasi untuk perkalian polinomial cepat
- **invNTT** (`invntt`) — transformasi balik
- **basemul** — perkalian pointwise di domain NTT

Saat `Enable PIE kernels = y`, build system mengganti file `ntt.c` clean dengan `ntt_pie.c` dan memakai helper vector di `pie_vec.h`.

Saat `Enable aggressive PIE optimizations = y`, compile flag `MLKEM_PIE_AGGRESSIVE=1` mengaktifkan jalur batching tambahan di helper yang sama.

## 2) Layer Perbandingan (Before vs After)

```text
[Baseline PQClean clean]
poly_ntt -> ntt.c (scalar butterfly, 1 koefisien per iterasi)

[PIE normal]
poly_ntt -> ntt_pie.c
  - butterfly 8-lane vector (EE.VADDS/VSUBS/VMUL)
  - Montgomery reduce vectorized
  - blok 16 koefisien per inner-loop saat len>=8

[PIE aggressive]
poly_ntt -> ntt_pie.c + pie_vec.h (MLKEM_PIE_AGGRESSIVE)
  - semua optimasi PIE normal
  + batch 2x vector block (8x2) dalam satu blok asm
  + constant vector tables QINV/KYBER_Q (tanpa loop init runtime)
  + fqmul_vec8_zvec_batch2 di loop NTT
```

## 3) PIE Normal: Apa Saja yang Dioptimasi?

### A. Vector arithmetic 8×int16 (SIMD-like PIE instructions)

Helper di `components/mlkem_pie/pie_vec.h`:

- `mlkem_pie_add_8`, `mlkem_pie_sub_8`, `mlkem_pie_addsub_8`
- Memuat 16 byte (8 coefficient int16) sekaligus: `EE.VLD.128.IP`
- Operasi lane paralel: `EE.VADDS.S16`, `EE.VSUBS.S16`

**Sebelum (clean scalar):**

```c
// clean/ntt.c - satu koefisien per iterasi butterfly
t = fqmul(zeta, r[j + len]);
r[j + len] = r[j] - t;
r[j] = r[j] + t;
```

**Sesudah (PIE vector block):**

```c
// ntt_pie.c - 8 koefisien sekaligus
fqmul_vec8_zvec(tvec0, zvec, &r[j + len]);
mlkem_pie_addsub_8(&r[j], tvec0, &r[j], &r[j + len]);
```

Artinya: operasi yang tadinya 8 kali loop scalar, menjadi 1 blok vector + reduce vector.

### B. Vector multiply + Montgomery reduce

- `mlkem_pie_mul_lohi_8`: kalikan 8 lane, ambil low/high 16-bit hasil 32-bit via register `SAR`
- `mlkem_pie_montgomery_reduce_vec8`: reduksi Montgomery untuk 8 lane

Di mode normal, tabel `qinv_vec` dan `q_vec` diisi runtime dengan loop 8 iterasi sebelum reduce.

### C. Struktur loop NTT yang lebih kasar-grained

Di `ntt_pie.c`, saat `len >= 8`:

- inner loop loncat per 16 koefisien (`j += 16`) dengan dua blok vector paralel
- fallback per 8 koefisien untuk sisa blok

Di clean `ntt.c`, inner loop selalu per 1 koefisien.

### D. Alignment data

- `PQCLEAN_*_zetas` dibuat `__attribute__((aligned(16)))` agar load vector efisien.

## 4) PIE Aggressive: Tambahan di Atas PIE Normal

Aggressive **tidak mengganti algoritma NTT**, tetapi mengurangi overhead implementasi.

### A. Batched vector multiply (`8x2`)

Contoh `mlkem_pie_mul_lo_8x2`:

- **Normal PIE**: panggil `mlkem_pie_mul_lo_8` dua kali (dua setup `SAR`, dua blok asm terpisah)
- **Aggressive**: satu blok asm memproses dua vector sekaligus (`a0/b0` dan `a1/b1`)

Dampak: mengurangi instruksi setup/teardown dan meningkatkan throughput pada butterfly ganda.

### B. Constant vector tables untuk Montgomery

**Normal PIE:** loop runtime mengisi `qinv_vec[8]` dan `q_vec[8]` setiap panggilan reduce.

**Aggressive PIE:**

```c
static const int16_t qinv_vec[8] __attribute__((aligned(16))) = { QINV, ... };
static const int16_t q_vec[8]   __attribute__((aligned(16))) = { KYBER_Q, ... };
```

Dampak: menghilangkan loop inisialisasi per panggilan reduce; tabel sudah di `.rodata`.

### C. Batch Montgomery reduce 2× (`montgomery_reduce_vec8x2`)

Reduce dua blok produk sekaligus memakai `mul_lo_8x2` + `mul_lohi_8x2`.

## 5) Kenapa Tidak Dijadikan Satu Opsi Saja?

Alasan desain di project ini:

1. **Isolasi eksperimen**: ingin memisahkan gain dari vectorization dasar vs micro-architectural batching.
2. **Fallback/debug**: aggressive lebih kompleks; jika ada issue performa/validasi, bisa matikan aggressive tanpa mematikan seluruh PIE.
3. **Portabilitas risiko**: aggressive mengandalkan pola asm dan asumsi alignment/SAR behavior yang lebih ketat.

## 6) Seberapa Besar Perbedaan Praktisnya?

| Mode | Perubahan yang diharapkan |
|------|---------------------------|
| clean -> PIE normal | Penurunan signifikan pada `cycles/time_us` operasi ber-NTT (keygen/decaps/encaps) |
| PIE normal -> aggressive | Penurunan tambahan lebih kecil (biasanya single-digit hingga low-teens %) |
| Dampak ke Scenario 1 end-to-end | Lebih kecil lagi, karena ada network latency MQTT |
| Dampak ke Scenario 2 end-to-end | Lebih terlihat, karena compute-dominated |

Untuk angka pasti di board Anda, gunakan pasangan run identik + `tools/analyze_pie_metrics.py`.

## 7) Ringkasan Satu Kalimat

- **PIE normal**: mengubah kernel NTT/reduce dari scalar C menjadi vector ESP32-S3 PIE.
- **PIE aggressive**: mengurangi overhead implementasi vector (batch 2×, constant tables, fewer SAR setups) di atas PIE normal.

---

# Bagian 2: Microbench Deep Dive (Kernel-Level vs Iterasi Biasa)

## 1) Apa Itu Microbench di Project Ini?

Microbench adalah mode pengukuran tambahan (opsional) yang menjalankan **tiga operasi polinomial inti** berulang kali:

1. `mb_ntt` — `poly_ntt`
2. `mb_invntt` — `poly_invntt_tomont`
3. `mb_basemul` — `poly_basemul_montgomery`

Implementasi: `main/microbench.c`, dipanggil dari `scenario_local.c` saat `CONFIG_MLKEM_MICROBENCH=y`.

Output CSV tetap format Scenario 2 (`s2`), dengan kolom `op` berisi prefix `mb_`.

## 2) Apa Maksud "Kernel-Level"?

"Kernel" di sini **bukan** kernel OS. Maksudnya adalah **primitive komputasi terdalam** yang dipakai berulang kali oleh ML-KEM.

```text
[Lapisan aplikasi benchmark]
  keygen / encaps / decaps      <-- iterasi biasa Scenario 2

[Lapisan KEM API]
  crypto_kem_keypair / enc / dec

[Lapisan IND-CPA + protocol logic]
  sampling, hash, compress/decompress, verify, serialization

[Lapisan polinomial - HOTSPOT]
  poly_ntt / poly_invntt / poly_basemul   <-- microbench mengukur ini

[Lapisan NTT inti]
  ntt() / invntt() / basemul()  (dioptimasi PIE pada ntt_pie.c)
```

## 3) Beda Microbench vs Iterasi Biasa

| Aspek | Iterasi biasa (`keygen/encaps/decaps`) | Microbench (`mb_ntt/mb_invntt/mb_basemul`) |
|-------|----------------------------------------|--------------------------------------------|
| Unit yang diukur | Operasi KEM lengkap | Primitive polinomial inti |
| Input data | Flow kripto normal (pk/sk/ct) | Polinomial sintetis deterministik (`fill_poly`) |
| Tujuan | Performa produk akhir | Diagnosa bottleneck compute |
| Sensitif PIE? | Ya, tapi tercampur komponen lain | Ya, lebih murni ke NTT path |
| Cocok untuk laporan akhir? | Ya | Tidak (kecuali studi internal optimasi) |

## 4) Kenapa Microbench Perlu Ada?

Tanpa microbench, jika `keygen_us` turun 20% setelah enable PIE, Anda belum tahu pasti apakah turun karena NTT, atau karena hash/compress, atau efek sampling RNG.

Microbench menjawab: **berapa gain langsung pada tiga primitive NTT-domain**.

## 5) Alur Eksekusi Saat Microbench ON

Urutan di firmware Scenario 2:

1. (Opsional) self-test shared secret
2. **Microbench loop** (`CONFIG_MLKEM_MICROBENCH_ITERS`)
   - precompute `a_ntt`, `b_ntt`
   - ulangi ukur `mb_ntt`, `mb_invntt`, `mb_basemul`
3. Loop normal `CONFIG_MLKEM_ITERATIONS` untuk `keygen/encaps/decaps`

CSV akan berisi dua kelompok data dalam satu run.

## 6) Contoh Interpretasi Data

Jika hasil run menunjukkan:
- `mb_ntt` turun 30% saat PIE on
- `keygen` turun 12% saat PIE on

Interpretasi: NTT memang jauh lebih cepat, tetapi keygen masih memuat komponen non-NTT yang tidak dipercepat sama dramatisnya.

## 7) Kapan Harus ON / OFF

| Kondisi | Rekomendasi |
|---------|-------------|
| Eksperimen performa final Scenario 2 | OFF |
| Investigasi dampak PIE pada NTT | ON |
| Validasi regresi setelah ubah `pie_vec.h` / `ntt_pie.c` | ON |
| Demo/laporan ke user akhir | OFF (hindari kebingungan CSV) |

---

# Bagian 3: PIE Speedup Blueprint

## Goals

- Speedup untuk Scenario 2 (kernel + end-to-end) dan Scenario 1 (handshake).
- Full pipeline PIE usage, tidak terbatas ke NTT/basemul.
- Aggressive optimization diizinkan, dengan KAT + microbench + Scenario 2 sebelum Scenario 1.

## Constraints and Assumptions

- Maintain clean PQClean fallback (PIE off).
- Keep correctness guardrails: KAT/self-test dan microbench logs.
- Alignment: 16-byte alignment untuk semua vector data.
- SAR/rsync overhead adalah major cost; prefer batch processing.

## Phases (Ordered)

### Phase 0: Instruction Feasibility

- Confirm which PIE instructions are usable beyond current VMUL/VADD/VSUB.
- Decide whether QACC can be used in inline asm.
- If QACC is not usable, focus on SAR batching and precomputed vector tables.

### Phase 1: Reduce SAR/rsync Overhead

- Add batched VMUL helpers for low/high halves (two vectors per SAR set).
- Use batched Montgomery reduce to amortize SAR changes.
- Update NTT/invNTT to process 16 coefficients per loop when possible.
- Reuse zeta vectors per block and reuse f vectors for scaling.

### Phase 2: Full Pipeline Vectorization

- Vectorize poly_reduce (Barrett).
- Vectorize poly_tomont (multiply by constant).
- Vectorize poly_add / poly_sub.
- Vectorize polyvec_reduce and polyvec_add.
- Vectorize compress/decompress quantization multiplies (packing still scalar).

### Phase 3: Basemul and Polyvec Tuning

- Reduce packing overhead in basemul (aligned vector loads and pre-shuffled lanes).
- Batch two basemul blocks per loop with x2 helpers to amortize SAR switches.
- If QACC is usable, prototype MAC-based basemul.

### Phase 4: Integration and Gating

- Add aggressive PIE build flag to toggle batched SAR and vector tables.
- `MLKEM_PIE_AGGRESSIVE` (Kconfig) enables batched SAR helpers and constant vector tables.
- Keep clean build path intact.

### Phase 5: Validation and Performance

- Run KAT/self-test on-device.
- Run microbench (mb_ntt, mb_invntt, mb_basemul).
- Run Scenario 2 for 512/768/1024.
- Run Scenario 1 for 512/768/1024 and compute QERS.

## Hot Paths (Priority)

1. NTT/invNTT multiply + reduce (SAR overhead).
2. Barrett reduce (poly_reduce / polyvec_reduce).
3. poly_tomont.
4. poly_add/poly_sub.
5. compress/decompress quantization.
6. basemul packing and zeta handling.

## Verification Checklist

- Build PIE on/off; no warnings or assembler errors.
- Self-test passes (shared secret matches).
- Microbench produces stable deltas and no regressions.
- Scenario 2 deltas show improvement for 512/768/1024.
- Scenario 1 deltas improve or stay neutral, and QERS improves.

## Notes

- SAR batching should be the first big speed win.
- Precomputed zeta vectors can reduce inner-loop overhead but cost flash.
- Aggressive PIE currently adds small constant vector tables (qvec/qinv) in PIE helpers; expect a few dozen bytes of extra rodata per TU.

---

Cara menjalankan microbench dan capture ada di `docs/03-operational-runbook.md`.
Konfigurasi PIE di menuconfig ada di `docs/02-menuconfig-guide.md`.
