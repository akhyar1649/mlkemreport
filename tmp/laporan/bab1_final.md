# BAB I
# PENDAHULUAN

## 1.1 Latar Belakang

Internet of Things (IoT) berkembang sebagai paradigma komputasi yang menghubungkan perangkat fisik, sensor, aktuator, dan layanan digital untuk mendukung pertukaran data secara otomatis (Kumar et al., 2019). Perangkat IoT banyak digunakan pada lingkungan industri, rumah pintar, kesehatan, dan pemantauan lingkungan karena mampu bekerja dekat dengan sumber data (Chanal & Kakkasageri, 2020). Peningkatan jumlah perangkat tersebut memperluas permukaan serangan karena setiap node dapat menjadi titik masuk bagi ancaman keamanan jaringan (Cherian & Chatterjee, 2019). Kondisi ini menjadi lebih kompleks karena banyak perangkat IoT memiliki keterbatasan daya, memori, dan kemampuan komputasi (Fitzgibbon & Ottaviani, 2024).

Keamanan komunikasi IoT masih banyak bergantung pada mekanisme kriptografi kunci publik klasik seperti RSA dan Elliptic Curve Cryptography (ECC) (Almutairi & Sheldon, 2025). Mekanisme tersebut menghadapi risiko jangka panjang karena algoritma Shor dapat melemahkan skema berbasis faktorisasi integer dan logaritma diskrit ketika komputer kuantum berskala besar tersedia (Almutairi & Sheldon, 2025). Ancaman ini mendorong kebutuhan transisi menuju Post-Quantum Cryptography (PQC) agar data yang memiliki masa guna panjang tetap terlindungi (Liu et al., 2024). Transisi tersebut perlu dianalisis secara hati-hati pada IoT karena algoritma PQC umumnya membawa ukuran parameter dan beban komputasi yang lebih besar dibandingkan skema klasik (Asif, 2021).

National Institute of Standards and Technology menetapkan Module-Lattice-Based Key-Encapsulation Mechanism (ML-KEM) sebagai standar mekanisme enkapsulasi kunci pasca-kuantum melalui FIPS 203 (NIST, 2024). ML-KEM berasal dari CRYSTALS-Kyber yang menggunakan masalah Module Learning With Errors sebagai dasar keamanan (Bos et al., 2018). Standar ini menyediakan tiga parameter set, yaitu ML-KEM-512, ML-KEM-768, dan ML-KEM-1024, yang merepresentasikan tingkat keamanan berbeda (NIST, 2024). Perbedaan tingkat keamanan tersebut disertai peningkatan ukuran kunci, ukuran ciphertext, dan beban operasi polinomial (Bos et al., 2018).

Implementasi ML-KEM pada perangkat terbatas membutuhkan perhatian khusus karena operasi seperti Number Theoretic Transform (NTT) dan reduksi modular muncul berulang pada KeyGen, Encaps, dan Decaps (Sonbul et al., 2025). NTT menjadi jalur komputasi penting karena mempercepat perkalian polinomial yang digunakan dalam skema lattice-based (Sonbul et al., 2025). Pada mikrokontroler, jalur komputasi tersebut dapat meningkatkan waktu eksekusi dan konsumsi energi jika tidak dioptimalkan (Chhetri, 2026). Oleh karena itu, optimasi pada level instruksi dan arsitektur perangkat menjadi salah satu pendekatan yang relevan untuk meningkatkan kelayakan ML-KEM pada IoT (Ji et al., 2024).

ESP32-S3 merupakan mikrokontroler yang menyediakan prosesor Xtensa LX7, konektivitas Wi-Fi, dan Processor Instruction Extensions (PIE) untuk operasi vektor 128-bit (Espressif Systems, 2025). Fitur PIE memungkinkan beberapa data berukuran kecil diproses secara paralel dalam satu instruksi vektor (Espressif Systems, 2025). Karakteristik tersebut relevan dengan operasi polinomial ML-KEM karena banyak koefisien direpresentasikan dalam bentuk bilangan 16-bit (Bos et al., 2018). Namun, efektivitas PIE pada implementasi standar ML-KEM di ESP32-S3 tetap perlu dibuktikan melalui pengujian empiris karena percepatan instruksi tidak selalu berbanding lurus dengan penurunan energi sistem (Chhetri, 2026).

Selain aspek komputasi lokal, penerapan ML-KEM pada IoT juga harus diuji pada protokol komunikasi yang digunakan secara nyata. MQTT merupakan protokol *publish/subscribe* ringan yang banyak digunakan pada sistem IoT berbasis broker (Banks et al., 2019). Penggunaan PQC pada MQTT dapat menambah ukuran *payload* dan memengaruhi *latency*, fragmentasi, serta konsumsi energi komunikasi (Malina et al., 2024). Evaluasi yang hanya mengukur operasi lokal belum cukup untuk menilai dampak ML-KEM terhadap komunikasi end-to-end pada perangkat IoT (Kim & Seo, 2025).

Penelitian ini menganalisis secara komparatif performa dan Skor Resiliensi Kuantum (QERS) pada implementasi standar ML-KEM di protokol MQTT menggunakan instruksi vektor ESP32-S3. Pengujian dilakukan pada ML-KEM-512, ML-KEM-768, dan ML-KEM-1024 dengan konfigurasi tanpa PIE dan dengan PIE. Evaluasi dilakukan melalui dua skenario, yaitu benchmark komputasi lokal dan handshake MQTT v5.0. Hasil pengujian digunakan untuk menilai pengaruh PIE terhadap waktu eksekusi, siklus CPU, konsumsi energi, performa komunikasi, serta skor QERS.

## 1.2 Rumusan Masalah

Berdasarkan latar belakang tersebut, rumusan masalah penelitian ini adalah sebagai berikut:

1. Bagaimana pengaruh instruksi vektor PIE terhadap performa operasi ML-KEM pada ESP32-S3 dibandingkan implementasi tanpa PIE?
2. Bagaimana perbandingan konsumsi energi implementasi standar ML-KEM pada ESP32-S3 untuk varian ML-KEM-512, ML-KEM-768, dan ML-KEM-1024?
3. Bagaimana dampak integrasi ML-KEM pada protokol MQTT v5.0 terhadap *latency*, *jitter*, *packet loss*, dan overhead komunikasi?
4. Konfigurasi mana yang menghasilkan nilai QERS terbaik sebagai trade-off antara performa, energi, sumber daya, dan tingkat resiliensi pasca-kuantum?

## 1.3 Batasan Masalah

Batasan masalah dalam penelitian ini adalah sebagai berikut:

1. Pengujian dilakukan pada jaringan lokal untuk mengurangi pengaruh *latency* internet eksternal.
2. Perangkat yang diuji adalah ESP32-S3 dengan pengukuran energi menggunakan sensor INA219.
3. Algoritma yang diuji adalah ML-KEM-512, ML-KEM-768, dan ML-KEM-1024 sesuai standar FIPS 203.
4. Protokol komunikasi yang digunakan adalah MQTT v5.0 dengan QoS 1.
5. Optimasi instruksi difokuskan pada pemanfaatan PIE untuk jalur operasi polinomial dan NTT.
6. Analisis keamanan difokuskan pada resiliensi pasca-kuantum dan QERS, bukan pada *side-channel attack* atau kriptanalisis baru.

## 1.4 Tujuan Penelitian

Tujuan penelitian ini adalah sebagai berikut:

1. Menganalisis pengaruh instruksi vektor PIE terhadap waktu eksekusi dan siklus CPU operasi KeyGen, Encaps, dan Decaps pada ML-KEM.
2. Mengukur konsumsi energi implementasi ML-KEM pada ESP32-S3 untuk mengetahui trade-off antara optimasi komputasi dan efisiensi daya.
3. Mengevaluasi performa handshake ML-KEM melalui MQTT v5.0 pada mode koneksi yang berbeda.
4. Menentukan konfigurasi terbaik berdasarkan QERS yang menggabungkan performa, energi, reliabilitas jaringan, sumber daya, dan tingkat keamanan.

## 1.5 Manfaat Penelitian

Manfaat penelitian ini adalah sebagai berikut:

1. Memberikan data empiris mengenai kelayakan implementasi standar ML-KEM pada ESP32-S3.
2. Menunjukkan dampak penggunaan instruksi vektor PIE terhadap performa ML-KEM pada mikrokontroler.
3. Menyediakan evaluasi end-to-end ML-KEM pada protokol MQTT v5.0.
4. Menjadi referensi bagi pengembang IoT yang ingin mengadopsi PQC dengan mempertimbangkan performa, energi, dan resiliensi sistem.

## 1.6 Sistematika Penulisan

Sistematika penulisan penelitian ini adalah sebagai berikut:

1. Bab I Pendahuluan menjelaskan latar belakang, rumusan masalah, batasan masalah, tujuan, manfaat, dan sistematika penulisan.
2. Bab II Tinjauan Pustaka menjelaskan dasar teori dan penelitian terkait yang mendukung penelitian.
3. Bab III Metodologi Penelitian menjelaskan rancangan sistem, skenario pengujian, teknik pengumpulan data, praproses, dan metode analisis.
4. Bab IV Hasil dan Pembahasan menyajikan hasil pengujian, perhitungan QERS, dan pembahasan konfigurasi terbaik.
