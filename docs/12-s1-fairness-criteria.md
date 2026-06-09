# Scenario 1 Network Stability and Fairness Criteria

Dokumen ini melengkapi panduan jaringan di `docs/11-openwrt-router-setup.md` dengan kriteria objektif untuk menilai stabilitas dan fairness lingkungan selama pengambilan data Scenario 1 (MQTT Handshake). Kriteria diterapkan untuk:
- Antar-run dalam mode yang sama (session atau reconn)
- Perbandingan antar varian dalam mode yang sama (mis. 512 vs 768 vs 1024 pada session)
- Perbandingan lintas mode (session vs reconn) untuk sanity check

Semua metrik merujuk pada kolom CSV hasil firmware dan analisis: `packet_loss_rate, ss_valid_rate_success, rssi_dbm, net_latency_us, jitter_us, handshake_us, reconnect_us, reconnect_energy_uj, total_latency_us, energy_uj, cpu_util_percent, keygen_us, decaps_us, encaps_us`.

## A. Preflight Checklist (sebelum setiap run)
- **Wiring & daya**: INA219 inline; ESP32 pakai adaptor 5V ≥ 1A; CP2102 hanya UART.
- **RF posisi**: Posisi perangkat, orientasi antena, dan jarak ditandai fisik; tidak berubah antar varian.
- **Channel & bandwidth**: Kanal dipatok (non-DFS disarankan), lebar kanal tetap (20 MHz), SSID khusus lab.
- **NIC host**: Governor CPU "performance"; Wi‑Fi power save OFF; TCP_NODELAY aktif di client jika didukung.
- **ESP32 Wi‑Fi**: Power save OFF (WIFI_PS_NONE) selama pengukuran; frekuensi CPU dikunci maksimum; logging minimal.
- **Lalu lintas latar**: Pastikan channel utilization rendah/konstan; tunda run bila tinggi.
- **Ping sanity**: Ke router dan broker
  - Median stabil; p95 rendah; tidak ada lonjakan berulang dalam 30–60 detik preflight.

## B. Acceptance (mode session, intra-run)
- **Kualitas dasar**
  - `packet_loss_rate = 0`
  - `ss_valid_rate_success = 1`
- **RF konsistensi**
  - `rssi_dbm p50 = −20 ± 1 dB` (target) dengan IQR ≤ 1 dB per run.
- **Jalur jaringan (tanpa reconnect)**
  - `net_latency_us`: CV ≤ 10%, dan rasio `p95/p50 ≤ 1.20`
  - `jitter_us`: `p95 < 10_000 µs` ATAU `p95 < 0.4 × net_latency_us p50`
  - `handshake_us`: tidak multimodal; konsisten dengan komponen (`compute + net`)
- **Compute & energi**
  - `keygen_us`: CV ≤ 2%
  - `decaps_us`: CV ≤ 3%
  - `encaps_us` (subscriber): CV ≤ 15% ATAU `p95/p50 ≤ 1.30`
  - `energy_uj`: CV ≤ 5% dan `p95/p50 ≤ 1.10`
  - `cpu_util_percent`: CV ≤ 5%

## C. Acceptance (mode reconn, intra-run)
- **Kualitas dasar**
  - `packet_loss_rate = 0`, `ss_valid_rate_success = 1`
- **RF konsistensi**
  - `rssi_dbm p50 = −20 ± 1 dB` (target) dengan IQR ≤ 1 dB per run.
- **Reconnect timer**
  - `reconnect_us p50 ≈ 16_000 ms ± 0.1%` (± 16 ms), CV ≤ 0.5%, `p95/p50 ≤ 1.003`
  - `reconnect_energy_uj`: CV ≤ 0.5%
- **Komponen lain (sanity)**
  - `handshake_us` (di run reconn) dalam ±10% dari nilai `handshake_us` pada mode session untuk varian/PIE yang sama (pada RSSI yang serupa)

## D. Fairness antar varian (mode session)
- **RF kesetaraan antar varian**
  - Selisih `rssi_dbm p50` antar varian ≤ 1 dB; IQR tiap varian ≤ 1 dB.
- **Kinerja compute monotonic**
  - `keygen_us p50` dan `decaps_us p50`: 512 < 768 < 1024 (selaras hasil Scenario 2)
  - `encaps_us p50`: 512 < 768 < 1024 (wajar karena beban subscriber lebih besar)
- **Jalur jaringan sebanding**
  - Dengan RSSI sepadan, `net_latency_us p50` antarkonfigurasi tidak menyimpang > ±10% tanpa alasan RF yang jelas.

## E. Sanity lintas mode (session vs reconn)
- Pada varian/PIE dan RSSI yang serupa:
  - `handshake_us p50` (reconn) dalam ±10% dari `handshake_us p50` (session)
  - `total_latency_us` (reconn) ≈ `reconnect_us` + `handshake_us` (session) dalam toleransi ±0.3% (dominan oleh reconnect timer)

## F. Desain run untuk meminimalkan bias
- **Blocking/Randomization**: Susun urutan varian per blok (mis. ABBA atau acak) agar variasi waktu/hari tidak bias satu varian.
- **Ukuran sampel**: Pakai blok verifikasi 30–50 iterasi; jika lulus acceptance, lanjut 100 iterasi.
- **Dokumentasi metadata**: Simpan kanal, lebar kanal, power save state, governor, jarak/posisi, commit firmware, versi broker/subscriber.

## G. Trigger untuk re-run atau penyesuaian
- Gagal kriteria di B atau C (terutama `packet_loss_rate > 0`, CV/p95 menyimpang, atau RSSI tidak sepadan).
- Perbedaan `rssi_dbm p50` antar varian > 1 dB.
- `reconnect_us` CV > 0.5% (indikasi backoff/timer tidak seragam).
- Outlier besar berulang pada `net_latency_us`/`jitter_us` (indikasi gangguan RF atau power save).

## H. Output yang dicek otomatis (opsional)
- Buat skrip acceptance yang membaca CSV per run dan melaporkan PASS/FAIL per butir di atas (dapat diintegrasikan ke `tools/` dan mengeluarkan ringkasan MD/CSV).
