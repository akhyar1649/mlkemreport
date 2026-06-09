# BAB III METODOLOGI PENELITIAN

Pelaksanaan penelitian ini mengadopsi struktur eksperimental kuantitatif berbasis uji laboratorium empiris untuk menganalisis performa optimasi tingkat instruksi perangkat keras (PIE) dibandingkan perangkat lunak standar (SW) dalam mengeksekusi standar kriptografi ML-KEM-512, ML-KEM-768, dan ML-KEM-1024. Tahapan riset diklasifikasikan ke dalam alur terstruktur yang melibatkan manipulasi variabel komunikasi dan komputasi mikro.

## 3.1 Alur Penelitian

Secara garis besar, alur kerja pengujian diilustrasikan melalui diagram blok sistem. Proses diawali dengan tahap penyiapan integrasi pustaka kriptografi FIPS 203 (*Software* standar) dan pengaktifan *flag compiler* untuk merekonfigurasi rutinitas NTT menggunakan unit ekstensi vektor 128-bit (PIE) bawaan mikrokontroler. Setelah arsitektur *firmware* terangkai, sistem dilanjutkan pada simulasi pengujian mikrokontroler internal (Skenario 2) serta simulasi *handshake* sesi *Internet of Things* (Skenario 1) melalui broker perpesanan MQTT.

*[LABEL GAMBAR 3.1: Diagram Alir Penelitian Umum. Harap masukkan gambar flowchart (dari file arsitektur_tesis.drawio) yang menunjukkan blok Start -> Persiapan ML-KEM -> Kompilasi 2 Varian Firmware (SW & PIE) -> Pengujian Skenario 2 (Local Compute) -> Pengujian Skenario 1 (MQTT Handshake) -> Preprocessing Data (Drop 5, p50) -> Kalkulasi QERS -> Analisis Hasil -> End.]*

Secara arsitektural, letak esensi optimasi PIE ditekankan pada modifikasi jalur pemrosesan koefisien polinomial di dalam blok NTT. Pada varian *software* standar, blok reduksi modular memproses elemen satu demi satu secara linear (skalar). Sebaliknya, pada varian PIE, instruksi operasi secara simultan memuat 8 elemen ke dalam satu register vektor dan menggunakan aritmatika paralel (contoh: komando `EE.VADDS`/`EE.VMUL`) untuk menuntaskan polinomial sekaligus.

*[LABEL GAMBAR 3.2: Diagram Perbandingan Arsitektur NTT SW vs PIE. Harap masukkan gambar diagram (dari file arsitektur_tesis.drawio) yang mengilustrasikan sebuah blok 'poly_ntt'. Di cabang atas (SW): Loop linear memproses 1 array pada satu waktu. Di cabang bawah (PIE): Load vektor 128-bit -> Eksekusi SIMD 8-lane secara bersamaan -> Store vektor.]*

## 3.2 Persiapan Perangkat Keras dan Lingkungan Pengujian

Implementasi lingkungan uji menuntut isolasi variabel untuk menjamin latensi jaringan dan konsumsi daya tidak terdistorsi oleh gangguan eksternal atau kelemahan sensor. Perangkat modul ESP32-S3 disambungkan dengan penganalisis presisi INA219 (I2C) serta dikendalikan melalui komunikasi UART CP2102.

*[LABEL GAMBAR 3.3: Diagram Perakitan Perangkat Keras. Harap masukkan gambar skematik (dari file arsitektur_tesis.drawio) yang menunjukkan ESP32-S3 tersambung dengan modul INA219 secara in-line pada jalur tegangan VCC 5V dari catu daya, di mana jalur I2C INA219 masuk ke pin GPIO ESP32 untuk pembacaan arus. Modul CP2102 terhubung pada jalur TX/RX.]*

Pengujian Skenario 1 membutuhkan konfigurasi infrastruktur topologi Wi-Fi pada *router* OpenWrt yang dimodifikasi secara ketat (Kim & Seo, 2025). Konfigurasi sistem mengisolasi *traffic* menggunakan partisi *Virtual Local Area Network* (VLAN) untuk memisahkan domain perangkat manajemen eksperimen dari domain mikrokontroler. Modul nirkabel dikunci (*channel pinning*) pada kanal frekuensi 11 untuk meredam probabilitas terjadinya *channel hopping* otomatis yang memicu fluktuasi latensi. Filter *firewall* diterapkan sehingga hanya paket *Transmission Control Protocol* (TCP) peladen MQTT pada port 1883 yang direlai. Di sisi peladen, opsi `TCP_NODELAY` dihidupkan untuk menonaktifkan algoritma agregasi paket Nagle, sehingga setiap pecahan kapsul M-LWE PQC dikirim secara langsung tanpa waktu jeda artifisial, dan fitur penghematan energi (*Wi-Fi Power Save*) di mikrokontroler dimatikan untuk mencegah latensi *sleep mode*.

## 3.3 Eksekusi Pengujian dan Sampling Data

Pengujian eksperimental dijalankan sebanyak 100 iterasi valid berturut-turut untuk setiap komputasi varian ML-KEM dan varian PIE. Kuantitas 100 sampel data matematis ini ditentukan berdasarkan metodologi stabilitas performa sistem perangkat lunak untuk menjamin pemenuhan asas *Central Limit Theorem* (Jain, 1991). Asas tersebut memastikan bahwa estimasi simpangan baku dari data performa operasi kriptografi akan terdistribusi mendekati pola kurva normal untuk pengujian validitas (*confidence interval*), serta menghindari *overhead* kelelahan termal dari durasi pengetesan perangkat yang berkepanjangan (Georges et al., 2007).

## 3.4 Preprocessing Data (Warmup Drop dan p50)

Hasil *log* serial eksperimen mentah disaring secara matematis sebelum agregasi dimulai. Sebanyak 5 baris iterasi paling awal pada setiap sesi eksperimen secara mutlak dibuang (*warmup drop*). Proses pembuangan (drop) siklus data awal ini dijustifikasi sebagai eliminasi efek anomali beban dingin atau *cold-cache effect* pada unit memori instruksi CPU mikrokontroler (Georges et al., 2007). Dalam Skenario 1 yang berbasis nirkabel, 5 iterasi ini juga meredam *settling time* seperti latensi pemetaan *Address Resolution Protocol* (ARP) awal dan fase *TCP Slow Start* dari perute OpenWrt yang lazim memicu *spike* artifisial (Malina et al., 2024).

Setelah data awal direduksi, agregasi hasil eksperimental diekstraksi ke dalam satu ukuran pemusatan data berupa kuantil median (persentil ke-50 atau p50). Penggunaan p50 direkomendasikan secara metodologis menggantikan nilai Rata-rata Aritmatika (*Mean*) untuk menganalisis metrik jaringan seperti koneksi MQTT dan durasi komputasi asinkron (Jain, 1991). Median terbukti jauh lebih kuat (*robust*) dalam meredam deviasi *outlier* asimetris positif akibat *packet loss* sporadis atau tabrakan gelombang di frekuensi Wi-Fi 2.4 GHz tanpa memelencengkan kesimpulan waktu penyelesaian komputasi sejati.

## 3.5 Analisis Skenario 2 (Rasio Percepatan PIE)

Metrik performa komputasi murni dari Skenario 2 ditransformasi untuk membuktikan rasio eskalasi kecepatan antara optimasi perangkat keras dan piranti lunak (*Speedup*). Fungsi persamaan kalkulasi komparatif ini diejawantahkan dalam rasio proporsional durasi *Software* terhadap durasi vektor PIE:  
$Speedup = \frac{T_{software}}{T_{pie}}$  
Sedangkan persentase reduksi jeda *overhead* waktu yang dihasilkan diukur untuk memvalidasi seberapa efisien jalur register 128-bit dalam melampaui linearitas operasi aslinya (Georges et al., 2007):  
$\% \Delta = \frac{T_{pie} - T_{software}}{T_{software}} \times 100$  

## 3.6 Perhitungan Skor QERS Skenario 1

Eksperimen protokol MQTT Handshake akan dinilai derajat efektivitas gabungannya memakai instrumen analitik *Quantum Encryption Resilience Score* (QERS). Metodologi perumusan QERS diawali dengan menormalisasi seluruh atribut KPI ke skala absolut 0 hingga 100 dengan rumusan linier rasio *Min-Max Normalization* bertipe proporsi positif atau proporsi negatif (Rassekhnia, 2026). Agar perhitungan tidak terdistorsi opini sepihak, distribusi kuantitas pembobotan antar-metrik (*weights*) diintegrasikan melalui hibrida metode *Entropy Weight Method* (EWM) dan *Analytic Hierarchy Process* (AHP) pakar (Hwang & Yoon, 1981).

Sistem skrip khusus `derive_weights.py` mengaplikasikan EWM untuk mendeteksi dispersi (variansi) dari setiap kolom data secara obyektif; kolom metrik yang nilainya konstan akan otomatis dihiraukan karena dianggap miskin muatan informasi (Vaidya & Kumar, 2006). Nilai bobot murni matematika ini kemudian dikawinkan dengan bobot prioritas teoritis ahli untuk merumuskan koefisien final ($w_{perf}$ dan $w_{sec}$) melalui persentase hibrida (variabel $\alpha$). Seluruh himpunan bobot ini dieksekusi oleh skrip `compute_qers.py` ke dalam agregasi nilai degradasi metrik operasional (*Penalty Performance/P*) serta skor resiliensi arsitektur (*Security/S*). Keluaran formula *QERS_fusion = 0.6 (100 - P) + 0.4 S* mengkonfirmasi peringkat mutlak kapabilitas setiap varian ML-KEM-512, ML-KEM-768, maupun ML-KEM-1024, di mana makro bobot 0.6 diorientasikan bagi prioritas ketat pemrosesan IoT (Hanna et al., 2025).
