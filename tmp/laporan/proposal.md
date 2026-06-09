**PROPOSAL TUGAS AKHIR**

Rabu, 22 April 2026

Jam 13.00

Ruangan B406

**Analisis Komparatif Performa dan Skor Resiliensi Kuantum (QERS) pada Implementasi Standar ML-KEM di Protokol MQTT Menggunakan Instruksi Vektor ESP32-S3**

![](data:image/png;base64...)

**Disusun Oleh :**

**Akhyar Amin**

**L0122010**

**PROGRAM STUDI INFORMATIKA**

**FAKULTAS TEKNOLOGI INFORMASI DAN SAINS DATA**

**UNIVERSITAS SEBELAS MARET**

**SURAKARTA**

**2026**

|  |  |
| --- | --- |
| ![](data:image/png;base64...) | UNIVERSITAS SEBELAS MARET  PROGRAM STUDI INFORMATIKA |

# PROPOSAL TUGAS AKHIR

Nama : Akhyar Amin

NIM : L0122010

PERSETUJUAN PEMBIMBING

Proposal Tugas Akhir ini telah disetujui oleh:

|  |
| --- |
| Pembimbing I |
|  |
| Fajar Muslim S.T., M.T.  NIP 199907242024061001 |

# BAB I PENDAHULUAN

## Latar Belakang

Integrasi teknologi *Internet of Things* (IoT) kini telah menjadi pilar utama dalam transformasi digital di berbagai sektor, mulai dari *smart city* hingga layanan kesehatan berbasis pemantauan jarak jauh (Asif, 2021). Pertumbuhan ini didorong oleh kemudahan konektivitas dan pertukaran data secara *real-time* yang ditawarkan oleh perangkat-perangkat bertenaga rendah. Namun, pesatnya adopsi teknologi ini tidak selalu diimbangi dengan protokol keamanan yang memadai, sehingga perangkat IoT seringkali menjadi titik lemah dalam sebuah ekosistem jaringan (Almutairi & Sheldon, 2025).

Dalam lanskap perangkat keras IoT saat ini, mikrokontroler ESP32-S3 telah menjadi pilihan utama bagi pengembang *edge computing* generasi berikutnya karena integrasi unit pemrosesan vektor yang mendukung beban kerja AI lokal (Baraskar et al., 2025). Kapabilitas arsitektural ini memungkinkan ESP32-S3 diimplementasikan dalam skenario kritis, mulai dari gateway industri (IIoT) yang mengelola transmisi data operasional mesin melalui protokol MQTT (Boonmeeruk et al., 2024), hingga aplikasi interaksi manusia berbasis suara (*Voice AI Assistant*) yang mengirimkan metadata audiens ke layanan *cloud* (Ali et al., 2026). Tingginya volume data telemetri, biometrik kesehatan (Chanal & Kakkasageri, 2020), serta koordinat navigasi kendaraan (Arregui Almeida et al., 2025) yang dikelola oleh perangkat ini menuntut perlindungan integritas dan kerahasiaan yang absolut. Tanpa skema keamanan yang tahan lama, data sensitif tersebut menjadi subjek utama ancaman penyadapan dan manipulasi, yang di masa depan akan semakin rentan seiring dengan meningkatnya kemampuan dekripsi berbasis kuantum.

Di masa depan, ekosistem IoT ini akan menghadapi ancaman dengan munculnya era komputasi kuantum. Skema kriptografi kunci publik seperti *Elliptic Curve Cryptography* (ECC) yang saat ini menjadi standar keamanan utama terancam lumpuh oleh kemampuan algoritma Shor (Almutairi & Sheldon, 2025). Menanggapi risiko tersebut, Google melalui pernyataan Walker dan Neven (2026) menegaskan bahwa transisi menuju protokol yang tahan terhadap serangan kuantum atau *Post-Quantum Cryptography* (PQC) merupakan kebutuhan yang tidak dapat ditunda demi menjaga integritas data jangka panjang.

Sebagai langkah standardisasi, *National Institute of Standards and Technology* (NIST) telah menetapkan ML-KEM yang merupakan mekanisme enkapsulasi kunci berbasis kisi (*lattice-based*) sebagai standar utama melalui dokumen FIPS 203 (NIST, 2024). ML-KEM menawarkan tingkat resiliensi yang tinggi terhadap serangan kuantum dibandingkan dengan kandidat PQC lainnya. Namun, protokol ini memiliki karakteristik yang berbeda dengan kriptografi klasik, terutama dalam hal ukuran kunci dan beban komputasi yang lebih berat untuk diimplementasikan pada perangkat dengan sumber daya terbatas (Malina et al., 2024).

Permasalahan utama muncul ketika ML-KEM diterapkan pada perangkat IoT yang memiliki keterbatasan memori dan daya komputasi. Ukuran kunci dan beban kalkulasi ML-KEM yang besar seringkali menyebabkan peningkatan latensi dan konsumsi energi (Fitzgibbon & Ottaviani, 2024). Riset oleh Lopez et al. (2025) menunjukkan bahwa tanpa optimasi yang tepat, penerapan algoritma PQC pada perangkat dengan sumber daya terbatas dapat menurunkan efisiensi operasional sistem secara keseluruhan.

Upaya untuk mengimplementasikan ML-KEM pada mikrokontroler telah dilakukan, salah satunya oleh Segatz dan Hafiz (2025) yang memanfaatkan pembagian tugas *dual-core* dan *hardware coprocessor* pada ESP32 standar. Pendekatan tersebut berhasil mencapai percepatan sebesar 1,72 kali pada fase *Key Generation* dan 1,84 kali pada fase *Encapsulation* melalui pemanfaatan instruksi SHA dan AES. Meskipun demikian, penelitian tersebut terbatas pada penggunaan *coprocessor* fungsional tanpa menyentuh optimasi pada jalur data arsitektur inti. Padahal, mikrokontroler generasi terbaru seperti ESP32-S3 memiliki fitur *Processor Instruction Extensions* (PIE) yang secara arsitektural berbeda dari pendahulunya, karena mendukung register vektor 128-bit yang secara spesifik dirancang untuk menangani beban kerja komputasi paralel yang lebih intensif (Espressif Systems, 2025).

Hambatan utama dalam performa ML-KEM terletak pada operasi *Number Theoretic Transform* (NTT) yang mengonsumsi sebagian besar siklus CPU dalam perkalian polinomial. Sonbul et al. (2025) mencoba memitigasi hal ini melalui desain akselerator NTT pada FPGA yang mengoptimalkan jalur data dan *pipelining* untuk mencapai latensi rendah. Di sisi lain, Ji et al. (2024) melalui proyek ECO-CRYSTALS berhasil mencatatkan rekor kecepatan pada arsitektur RISC-V 64-bit dengan memanfaatkan strategi penjadwalan register dan optimasi assembly khusus. Namun, terdapat perbedaan fundamental antara optimasi pada RISC-V atau akselerator FPGA dengan arsitektur Xtensa LX7 pada ESP32-S3. Sejauh ini, efektivitas instruksi vektor PIE dalam mempercepat unit *butterfly* NTT pada ML-KEM-512 belum diuji secara komparatif terhadap implementasi perangkat lunak standar. Analisis mendalam mengenai bagaimana register vektor 128-bit mengelola elemen polinomial secara simultan menjadi sangat penting untuk memahami batas performa perangkat IoT bertenaga rendah dalam menjalankan standar FIPS 203 (NIST, 2024).

Tak hanya aspek kecepatan komputasi, evaluasi terhadap integrasi PQC dalam protokol komunikasi ringan juga menjadi perhatian utama. Malina et al. (2024) melakukan benchmarking terhadap berbagai algoritma PQC pada protokol MQTT dan menemukan adanya peningkatan latensi yang signifikan serta risiko fragmentasi paket akibat ukuran kunci yang besar. Akan tetapi, analisis *overhead* jaringan tersebut seringkali dilakukan secara terpisah dari metrik konsumsi energi dan utilisasi hardware yang mendalam. Rassekhnia (2026) kemudian memperkenalkan metrik *Quantum Encryption Resilience Score* (QERS) sebagai instrumen evaluasi multi-kriteria yang mampu mengagregasi variabel performa heterogen ke dalam satu skor terpadu. Implementasi metrik ini memungkinkan penilaian yang lebih holistik terhadap kelayakan sebuah sistem, terutama saat membandingkan dua varian implementasi yang memiliki karakteristik penggunaan sumber daya yang berbeda (Rassekhnia, 2026).

Berdasarkan tinjauan tersebut, penelitian ini difokuskan pada sebuah analisis komparatif performa ML-KEM-512 pada mikrokontroler ESP32-S3. Fokus utama penelitian adalah membandingkan secara empiris antara implementasi operasi NTT berbasis perangkat lunak murni dengan implementasi yang memanfaatkan instruksi vektor PIE. Dengan mengintegrasikan kedua varian tersebut ke dalam protokol MQTT v5.0, penelitian ini bertujuan untuk mengukur dampak nyata dari akselerasi *hardware* terhadap efisiensi *handshake* kriptografi, serta mengukur skor resiliensi QERS untuk memastikan kelayakan operasional pada perangkat IoT dengan sumber daya terbatas.

## Rumusan Masalah

Berdasarkan latar belakang tersebut, rumusan masalah dalam penelitian ini adalah sebagai berikut:

1. Bagaimana perbandingan performa operasi *Number Theoretic Transform* (NTT) antara penggunaan instruksi vektor PIE dan implementasi perangkat lunak murni, serta sejauh mana perbedaan tersebut memengaruhi efisiensi waktu *round-trip* handshake ML-KEM-512 pada protokol MQTT v5.0?
2. Bagaimana perbandingan konsumsi daya listrik yang dihasilkan selama fase handshake ML-KEM-512 pada ESP32-S3 saat menggunakan akselerasi instruksi vektor PIE dan saat menggunakan perangkat lunak murni?
3. Bagaimana perbandingan nilai *Quantum Encryption Resilience Score* (QERS) antara kedua varian implementasi instruksi vektor PIE dan implementasi perangkat lunak murni dalam menentukan tingkat kesiapan (*readiness*) perangkat IoT menghadapi ancaman kuantum?

## Batasan Masalah

Batasan masalah dalam penelitian ini adalah sebagai berikut:

1. Lingkungan jaringan pengujian terbatas pada jaringan lokal (LAN) untuk meminimalkan fluktuasi latensi eksternal.
2. Analisis keamanan dalam penelitian ini difokuskan pada evaluasi resiliensi melalui skor QERS dan performa komputasi, sehingga tidak mencakup pengujian penetrasi atau analisis ketahanan terhadap serangan.

## Tujuan Penelitian

Tujuan dari penelitian ini adalah sebagai berikut:

1. Menganalisis dampak penggunaan instruksi vektor PIE terhadap percepatan operasi NTT dan efisiensi waktu *round-trip handshake* pada protokol MQTT v5.0 dibandingkan dengan implementasi berbasis perangkat lunak.
2. Mengukur dan membandingkan profil konsumsi daya listrik pada fase *handshake* ML-KEM-512 untuk menilai efisiensi energi dan kelayakan operasional pada perangkat dengan sumber daya terbatas.
3. Mengevaluasi skor QERS secara holistik untuk memberikan penilaian kuantitatif mengenai resiliensi dan kesiapan sistem antara implementasi berbasis hardware (PIE) dan software.

## Manfaat Penelitian

Manfaat dari penelitian ini adalah sebagai berikut:

1. Memberikan kontribusi teknis berupa strategi optimasi operasi NTT menggunakan instruksi vektor PIE pada mikrokontroler ESP32-S3 untuk standar ML-KEM-512.
2. Menjadi referensi bagi praktisi dan pengembang sistem IoT dalam mengadopsi standar keamanan pasca-kuantum yang efisien pada perangkat keras bertenaga rendah.
3. Menghasilkan data empiris mengenai skor resiliensi QERS yang dapat membantu standardisasi evaluasi keamanan pada perangkat keras bertenaga rendah.

## Sistematika Penulisan

Sistematika penulisan dari penelitian ini berisikan:

* Bab I "Pendahuluan" menjelaskan tentang hal-hal yang menjadi latar belakang, rumusan, dan batasan masalah. Bagian ini juga membahas tujuan dan manfaat penelitian serta sistematika penulisan.
* Bab II "Tinjauan Pustaka" berisikan teori-teori dasar yang akan digunakan dalam penelitian ini meliputi ML-KEM, MQTT v5.0, arsitektur ESP32-S3, dan lainnya.
* Bab III "Metodologi Penelitian" menjelaskan tentang alur pengerjaan, skema hardware, dan perancangan optimasi instruksi vektor.

# BAB II TINJAUAN PUSTAKA

## Dasar Teori

### *Internet of Things* (IoT) dan Protokol MQTT

*Internet of Things* (IoT) adalah paradigma yang memungkinkan komunikasi antar perangkat elektronik melalui internet, menawarkan solusi inovatif di berbagai sektor. IoT menggabungkan sistem cerdas, sensor pintar, dan teknologi canggih untuk meningkatkan efisiensi dan fungsionalitas (Kumar et al., 2019). Arsitektur IoT umumnya terdiri dari lapisan-lapisan utama yang mencakup persepsi untuk pengumpulan data, jaringan untuk transmisi, hingga aplikasi untuk penyampaian informasi kepada pengguna (Kumar et al., 2019). Perbedaan mendasar jaringan IoT dengan jaringan konvensional terletak pada penggunaan protokol komunikasi yang lebih ringan karena keterbatasan sumber daya pada perangkat sensor (Cherian & Chatterjee, 2019).

![Screenshot From 2026-04-01 10-08-33](data:image/png;base64...)

1. Arsitektur Tradisional Protokol MQTT (Malina et al., 2024)

Dalam mendukung pertukaran data pada perangkat dengan daya rendah, protokol *Message Queuing Telemetry Transport* (MQTT) menjadi standar yang paling banyak digunakan. MQTT adalah protokol pesan berbasis publish/subscribe yang dirancang untuk kondisi jaringan dengan bandwidth terbatas dan latensi tinggi (Malina et al., 2024). Protokol ini meminimalkan overhead jaringan dengan menggunakan struktur pesan yang ringkas. Mekanisme kerja protokol ini melibatkan broker sebagai pusat distribusi pesan antara *publisher* (pengirim) dan *subscriber* (penerima), yang secara visual dapat dijelaskan melalui Gambar 2.1.

### Kriptografi Pasca-Kuantum (*Post-Quantum Cryptography*)

Kriptografi Pasca-Kuantum (*Post-Quantum Cryptography* atau PQC) merujuk pada algoritma kriptografi yang dirancang agar tetap aman meskipun menghadapi ancaman dari komputer kuantum masa depan. Ancaman utama terhadap sistem keamanan saat ini berasal dari algoritma Shor, yang mampu menyelesaikan masalah faktorisasi integer dan logaritma diskrit dalam waktu singkat, sehingga melumpuhkan algoritma kunci publik klasik seperti RSA dan *Elliptic Curve Cryptography* (ECC) (Almutairi & Sheldon, 2025). Oleh karena itu, diperlukan transisi menuju algoritma baru yang berbasis pada masalah matematika yang sulit diselesaikan oleh komputer kuantum (Walker & Neven, 2026).

Algoritma PQC umumnya diklasifikasikan ke dalam beberapa kategori berdasarkan landasan matematikanya, seperti berbasis kisi (*lattice-based*), berbasis kode (*code-based*), dan berbasis isogeni (*isogeny-based*) (Asif, 2021). Di antara kategori tersebut, kriptografi berbasis kisi dianggap paling menjanjikan untuk implementasi pada perangkat IoT karena efisiensi komputasi dan ukuran kunci yang relatif seimbang (Asif, 2021). Tantangan utama dalam adopsi PQC pada perangkat terbatas adalah manajemen konsumsi daya dan memori, mengingat algoritma ini memiliki beban operasional yang lebih besar dibandingkan kriptografi konvensional (Fitzgibbon & Ottaviani, 2024).

### Standar ML-KEM (FIPS 203)

*Module-Lattice-Based Key-Encapsulation Mechanism* (ML-KEM), yang sebelumnya dikenal sebagai CRYSTALS-Kyber, merupakan standar kriptografi pasca-kuantum yang ditetapkan oleh NIST untuk mekanisme enkapsulasi kunci (NIST, 2024). Keamanan ML-KEM didasarkan pada kekerasan masalah *Module Learning with Errors* (MLWE) pada kisi terstruktur. Standar ini dipilih karena memberikan performa tinggi dan latensi yang rendah, menjadikannya kandidat ideal untuk pengamanan komunikasi pada perangkat dengan sumber daya terbatas (NIST, 2024).

Proses utama dalam ML-KEM mencakup tiga algoritma, yaitu *Key Generation* untuk menghasilkan pasangan kunci, *Encapsulation* untuk menghasilkan kunci rahasia bersama dan *ciphertext*, serta *Decapsulation* untuk memulihkan kunci tersebut. Langkah-langkah dalam proses enkapsulasi, di mana kunci publik (ek) digunakan untuk menghasilkan kunci rahasia (K) dan *ciphertext* (c), secara formal didefinisikan dalam Persamaan 2.1.

|  |  |  |
| --- | --- | --- |
|  |  | **(2.1)** |

Standardisasi ML-KEM dalam dokumen FIPS 203 tidak hanya menetapkan alur operasional algoritma, tetapi juga mendefinisikan set parameter matematis yang sangat spesifik. Parameter ini dirancang untuk menjamin keamanan berbasis kisi sekaligus mengoptimalkan efisiensi komputasi pada arsitektur perangkat keras modern (NIST, 2024). Penentuan nilai-nilai tersebut bersifat krusial dan tidak disarankan untuk diubah guna menjaga integritas bukti keamanan (*security proofs*) serta kompatibilitas operasional dengan protokol komunikasi global (Bos et al., 2018). Rincian parameter teknis ML-KEM beserta justifikasi penetapannya dirangkum dalam Tabel 2.2.

**Tabel 2.2.** Parameter Konfigurasi Standar ML-KEM (FIPS 203)

|  |  |  |  |
| --- | --- | --- | --- |
| **Parameter** | **Simbol** | **Nilai** | **Justifikasi Teknis dan Akademik** |
| Polynomial Degree | n | 256 | Menyeimbangkan antara margin keamanan kisi dan efisiensi, nilai ini dipilih agar setiap koefisien dapat merepresentasikan satu bit informasi dalam struktur polinomial(Bos et al., 2018). |
| Modulus | q | 3329 | Dipilih secara spesifik karena memenuhi syarat yang memungkinkan penggunaan algoritma Number Theoretic Transform (NTT) secara optimal pada arsitektur 32-bit tanpa penalti kinerja yang besar (Sonbul et al., 2025). |
| Module Rank | k | 2, 3, 4 | Parameter yang menentukan tingkat keamanan (NIST Level 1, 3, 5). Penambahan nilai k secara linear meningkatkan dimensi kisi, yang secara eksponensial mempersulit serangan reduksi basis kisi seperti BKZ (NIST, 2024). |
| Noise Distribution |  | 2, 3 | Digunakan untuk sampling distribusi binomial terpusat. Nilai kecil dipilih secara presisi untuk menjaga probabilitas kegagalan de-enkapsulasi tetap di bawah sesuai standar keamanan kriptografi (NIST, 2024). |
| Compression |  | 10, 4 | Parameter pembulatan bit untuk mengurangi ukuran ciphertext dan kunci publik, dioptimasi agar ukuran transmisi data pada protokol seperti MQTT tetap efisien tanpa merusak tingkat keberhasilan dekripsi (Bos et al., 2018). |

Melalui konfigurasi pada Tabel 2.2, terlihat bahwa pemilihan nilai q = 3329 menjadi faktor penentu utama bagi efektivitas optimasi instruksi vektor. Jika nilai ini diubah, maka arsitektur unit *butterfly* pada prosesor tidak lagi dapat mengeksekusi reduksi modular secara paralel dengan latensi rendah (Sonbul et al., 2025). Hal ini menegaskan bahwa penggunaan parameter standar bukan hanya masalah kompatibilitas, melainkan syarat mutlak bagi performa operasional sistem pada perangkat terbatas terutama saat membandingkan dua varian implementasi yang memiliki karakteristik penggunaan sumber daya yang berbeda (Rassekhnia, 2026).

### *Number Theoretic Transform* (NTT)

Operasi perkalian polinomial merupakan komponen yang paling intensif secara komputasi dalam algoritma kriptografi berbasis kisi (*lattice-based*), termasuk pada standar ML-KEM. Algoritma *Number Theoretic Transform* (NTT) digunakan untuk mengoptimalkan efisiensi proses tersebut dengan mengubah kompleksitas waktu dari perkalian konvensional menjadi (Sonbul et al., 2025). Dalam implementasi ML-KEM, koefisien polinomial ditransformasikan ke domain NTT untuk melakukan perkalian titik demi titik (*point-wise multiplication*). Transformasi maju NTT (FNTT) untuk setiap koefisien didefinisikan secara formal melalui Persamaan 2.2.

|  |  |  |
| --- | --- | --- |
|  |  | **(2.2)** |

Berdasarkan Persamaan 2.2, simbol merepresentasikan *primitive 2n-th root of unity* di dalam medan . Operasi utama dalam komputasi NTT melibatkan unit yang disebut *butterfly operation*, yang terdiri dari konfigurasi Cooley-Tukey (CT) untuk transformasi maju dan Gentlemann-Sande (GS) untuk transformasi terbalik (Sonbul et al., 2025). Arsitektur unit butterfly terpadu yang mendukung kedua konfigurasi tersebut digambarkan pada Gambar 2.2.

![Screenshot From 2026-04-01 12-44-21](data:image/png;base64...)

1. Arsitektur Unit Butterfly Terpadu untuk Konfigurasi CT dan GS (Sonbul et al., 2025)

Melalui unit pada Gambar 2.2, logika kontrol menentukan jalur komputasi berdasarkan mode NTT yang dipilih. Pada transformasi maju (FNTT), multiplexer mB mengatur alur pemrosesan, sedangkan pada transformasi terbalik (INTT), digunakan *multiplexer* mA dan mC untuk menjalankan operasi aritmatika yang diperlukan (Sonbul et al., 2025).

### Arsitektur ESP32-S3 dan Instruksi Vektor (PIE)

ESP32-S3 adalah sistem dalam chip (*System on Chip* atau SoC) bertenaga rendah yang ditenagai oleh prosesor Xtensa® Dual-Core 32-bit LX7. Arsitektur LX7 pada ESP32-S3 membawa fitur inovatif berupa unit pemrosesan vektor yang mendukung instruksi khusus untuk akselerasi pemrosesan data paralel (Espressif Systems, 2025). Unit ini dikenal sebagai Processor Instruction Extensions (PIE) yang dirancang untuk meningkatkan performa komputasi pada aplikasi yang membutuhkan throughput tinggi seperti kriptografi. Struktur internal dari unit PIE ini dijelaskan pada Gambar 2.3.

![Screenshot From 2026-04-01 12-46-26](data:image/png;base64...)

1. Struktur Internal Unit *Processor Instruction Extensions* (PIE) (Espressif Systems, 2025)

Unit PIE yang ditunjukkan pada Gambar 2.3 bekerja dengan memanfaatkan register vektor 128-bit yang dilambangkan dengan q0 hingga q7. Dengan lebar data 128-bit, register tersebut mampu memproses delapan elemen data bertipe int16\_t secara simultan dalam satu siklus instruksi melalui mekanisme Single Instruction, Multiple Data (SIMD) (Espressif Systems, 2025). Kemampuan ini sangat relevan untuk mengoptimasi operasi NTT yang dibahas pada sub-bab 2.1.4, di mana koefisien polinomial dapat diproses secara paralel untuk mengurangi jumlah siklus CPU.

Pemanfaatan instruksi vektor melalui register q0–q7 ini memungkinkan percepatan signifikan pada operasi perkalian modular dan penjumlahan yang ada pada unit butterfly. Dengan menggunakan teknik optimasi pada level instruksi mesin, kendala latensi yang biasanya muncul pada implementasi perangkat lunak murni dapat dimitigasi (Ji et al., 2024). Implementasi ini menjadi fokus utama dalam memastikan standar ML-KEM dapat berjalan efisien pada perangkat IoT yang memiliki keterbatasan sumber daya (Segatz & Hafiz, 2025).

### Sensor INA219 dan Pengukuran Daya

Pengukuran konsumsi energi merupakan parameter krusial dalam mengevaluasi kelayakan implementasi kriptografi pasca-kuantum pada perangkat IoT yang beroperasi dengan sumber daya terbatas. Salah satu instrumen yang umum digunakan untuk melakukan pemantauan daya secara real-time adalah sensor INA219. INA219 merupakan sebuah monitor arus dan tegangan dua arah dengan antarmuka yang mampu mengukur aliran arus pada bus tegangan tinggi (Lopez et al., 2025). Sensor ini bekerja dengan prinsip mengukur penurunan tegangan pada resistor shunt eksternal untuk menghitung arus yang mengalir melalui beban.

Dalam pengujian perangkat bertenaga rendah, akurasi pengukuran pada level miliampere (mA) sangat diperlukan untuk menangkap lonjakan arus saat proses komputasi kriptografi yang intensif berlangsung. Data tegangan dan arus yang diperoleh dari sensor kemudian diolah untuk menghitung daya total (P) dalam satuan miliWatt (mW) dan total energi (E) dalam satuan miliJoule (mJ). Perhitungan konsumsi daya sesaat (P) dilakukan berdasarkan hukum Ohm yang didefinisikan dalam Persamaan 2.3.

|  |  |  |
| --- | --- | --- |
|  |  | **(2.3)** |

Berdasarkan Persamaan 2.3, nilai daya diperoleh dari perkalian antara tegangan bus dan arus yang terdeteksi. Integrasi sensor ini pada jalur catu daya mikrokontroler memungkinkan peneliti untuk memetakan profil energi selama fase handshake kriptografi berlangsung, sehingga implikasi penggunaan algoritma ML-KEM terhadap masa pakai baterai perangkat dapat dianalisis secara kuantitatif (Lopez et al., 2025).

### *Quantum Encryption Resilience Score* (QERS)

*Quantum Encryption Resilience Score* (QERS) merupakan sebuah kerangka kerja evaluasi multi-kriteria yang dirancang untuk mengukur dampak operasional dan tingkat kesiapan (*readiness*) dari implementasi kriptografi pasca-kuantum (PQC) pada sistem yang memiliki keterbatasan sumber daya. Berbeda dengan metode *benchmarking* tradisional yang mengevaluasi metrik performa secara terisolasi, QERS menyatukan variabel-variabel heterogen baik dari sisi kinerja komputasi maupun ketahanan keamanan ke dalam satu nilai komposit yang terukur (Rassekhnia, 2026).

![Screenshot From 2026-04-01 13-51-18](data:image/png;base64...)

1. Daftar Metrik dan Simbol dalam Kerangka Kerja QERS (Rassekhnia, 2026)

Metrik utama yang digunakan dalam perhitungan QERS dibagi menjadi kriteria biaya (*cost criteria*) yang diharapkan bernilai rendah, dan kriteria manfaat (*benefit criteria*) yang diharapkan bernilai tinggi. Daftar metrik tersebut beserta simbol identifikasinya dirangkum dalam Tabel 2.1.

Sebelum dilakukan agregasi ke dalam rumus QERS, setiap nilai metrik mentah hasil pengukuran harus melalui tahap normalisasi menggunakan teknik min-max *scaling*. Proses ini bertujuan untuk menyamakan skala pengukuran yang berbeda (misalnya milidetik, persen, atau miliJoule) ke dalam rentang skor standar 0 hingga 100 (Rassekhnia, 2026). Rumus normalisasi didefinisikan dalam Persamaan 2.4.

|  |  |  |
| --- | --- | --- |
|  |  | **(2.4)** |

Berdasarkan Persamaan 2.4, nilai adalah skor hasil normalisasi, X merupakan nilai observasi, MS adalah skor maksimum (100), sementara dan adalah nilai minimum dan maksimum yang ditemukan dalam dataset eksperimen (Rassekhnia, 2026). Setelah tahap normalisasi, skor QERS dihitung melalui tiga lapisan evaluasi (*Basic, Tuned, dan Fusion*) sebagai berikut:

1. *Basic* QERS

Fokus pada metrik operasional dasar untuk menilai dampak langsung PQC terhadap efisiensi komunikasi.

|  |  |  |
| --- | --- | --- |
|  |  | **(2.5)** |

1. *Tuned* QERS

Memperluas evaluasi dengan mengintegrasikan faktor lingkungan perangkat dan karakteristik fisik kunci kriptografi.

|  |  |  |
| --- | --- | --- |
|  |  | **(2.6)** |

1. *Fusion* QERS

Merupakan skor akhir yang mengintegrasikan subskor performa (P) dan subskor keamanan (S) menggunakan prinsip *Multi-Criteria Decision Analysis* (MCDA).

|  |  |  |
| --- | --- | --- |
|  |  | **(2.7)** |
|  |  | **(2.8)** |
|  |  | **(2.9)** |

Pada Persamaan 2.5 hingga 2.9, simbol mewakili koefisien bobot untuk masing-masing metrik, sedangkan dan adalah bobot penyeimbang antara performa dan keamanan. Alur kerja komprehensif dari proses pengambilan data hingga dihasilkannya skor akhir disajikan pada Gambar 2.4.

![Screenshot From 2026-04-01 14-01-30](data:image/png;base64...)

1. Alur Kerja Perhitungan Skor QERS (Rassekhnia, 2026)

Implementasi evaluasi menggunakan skor QERS pada penelitian ini menjadi salah satu metode untuk membandingkan varian optimasi hardware pada ESP32-S3. Dengan mengacu pada alur kerja Gambar 2.4, hasil pengukuran performa dari PIE NTT dan Software NTT akan diolah untuk mendapatkan nilai kuantitatif yang menunjukkan kesiapan protokol MQTT dalam menghadapi era pasca-kuantum (Rassekhnia, 2026).

## Penelitian Terkait

| **No** | **Judul Jurnal** | **Masalah** | **Tujuan** | **Metode** | **Hasil** | **Keterkaitan** |
| --- | --- | --- | --- | --- | --- | --- |
| 1. | Efficient Implementation of CRYSTALS-KYBER Key Encapsulation Mechanism on ESP32. (Segatz & Hafiz, 2025) | Tingginya beban komputasi algoritma Kyber yang sulit dijalankan secara efisien pada mikrokontroler dengan sumber daya terbatas seperti ESP32. | Mencapai implementasi Kyber yang dioptimasi dengan memanfaatkan fitur perangkat keras spesifik pada ESP32. | Pembagian tugas secara manual (hand-partitioning) pada arsitektur dual-core ESP32 dan pemanfaatan koprosesor kriptografi SHA/AES bawaan. | Berhasil meningkatkan kecepatan eksekusi sebesar 1.72x untuk Key Generation dan 1.84x untuk Encapsulation. | Jurnal ini menjadi baseline performa. Penelitian ini meningkatkan subjek ke ESP32-S3 dengan optimasi instruksi vektor PIE. |
| 2. | Quantum Encryption Resilience Score (QERS) for MQTT, HTTP, and HTTPS under Post-Quantum Cryptography in Computer, IoT, and IIoT Systems. (Rassekhnia, 2026) | Kebutuhan metrik tunggal yang komprehensif untuk membandingkan berbagai algoritma PQC di lingkungan IoT yang mempertimbangkan performa sekaligus keamanan. | Mengusulkan dan memvalidasi formula QERS sebagai standar evaluasi resiliensi kriptografi di era kuantum. | Pengukuran eksperimental pada ESP32-C6 dan Raspberry Pi CM4 meliputi utilisasi CPU, RSSI, latensi handshake, dan konsumsi energi. | Terbentuknya formula QERS yang mampu menyatukan metrik heterogen menjadi skor perbandingan yang valid. | Jurnal ini menyediakan llandasan matematis metrik QERS yang akan diterapkan pada varian PIE NTT di penelitian ini. |
| 3. | Accelerating CRYSTALS-Kyber: High-Speed NTT Design with Optimized Pipelining and Modular Reduction. (Sonbul et al., 2025) | Operasi Number Theoretic Transform (NTT) merupakan hambatan utama (bottleneck) dalam performa Kyber karena kompleksitas perkalian polinomialnya. | Mempercepat proses NTT melalui desain hardware yang mendukung pipelining dan reduksi modular yang dioptimasi. | Desain arsitektur akselerator NTT dengan fokus pada efisiensi jalur data (data path) dan pengurangan siklus komputasi. | Penurunan latensi yang signifikan pada proses transformasi domain polinomial. | Jurnal ini memberikan landasan logika struktur NTT yang akan diimplementasikan melalui register q0-q7 pada ESP32-S3. |
| 4. | Quantum-Resistant and Secure MQTT Communication. (Malina et al., 2024) | Ketidakpastian dampak integrasi algoritma PQC yang memiliki ukuran paket besar terhadap protokol ringan seperti MQTT. | Mengevaluasi resiliensi dan performa komunikasi MQTT ketika menggunakan enkripsi pasca-kuantum. | Pengujian benchmarking berbagai algoritma PQC (termasuk Kyber) pada transmisi MQTT untuk mengukur overhead jaringan. | Analisis mendalam mengenai peningkatan latensi dan fragmentasi paket akibat ukuran kunci PQC. | Jurnal ini mendasari rumusan masalah mengenai performa handshake MQTT v5.0 dan menjadi pembanding hasil eksperimen. |
| 5. | ECO-CRYSTALS: Efficient Cryptography CRYSTALS on Standard RISC-V ISA. (Ji et al., 2024) | Operasi Keccak dan perkalian polinomial pada Kyber memakan waktu siklus CPU yang sangat besar pada instruksi standar (General Purpose). | Mengoptimalkan implementasi Kyber dan Dilithium menggunakan instruksi khusus pada arsitektur RISC-V 64-bit. | Penggunaan strategi penjadwalan register dan optimasi assembly khusus untuk memproses elemen polinomial secara efisien. | Mencapai rekor kecepatan implementasi perangkat lunak berbasis assembly untuk Kyber pada platform RISC-V. | Jurnal ini memvalidasi metodologi optimasi level instruksi (ISA) yang serupa dengan penggunaan instruksi PIE dalam penelitian ini. |

Penelitian ini memberikan kontribusi kebaruan melalui optimasi tingkat rendah (*low-level*) pada operasi *Number Theoretic Transform* (NTT) menggunakan instruksi vektor PIE pada mikrokontroler ESP32-S3, sebuah dimensi teknis yang belum dieksplorasi secara mendalam dalam literatur saat ini. Berbeda dengan pendekatan Segatz dan Hafiz (2025) yang berfokus pada pembagian tugas *dual-core* dan koprosesor SHA pada arsitektur ESP32 standar, penelitian ini secara spesifik mengeksploitasi register vektor 128-bit untuk memproses delapan elemen data secara paralel guna memitigasi latensi komputasi ML-KEM-512 sesuai standar final FIPS 203 (NIST, 2024). Lebih lanjut, integrasi hasil optimasi hardware ini ke dalam protokol MQTT v5.0 yang dievaluasi menggunakan metodologi *Quantum Encryption Resilience Score* (Rassekhnia, 2026) menawarkan pandangan yang lebih holistik dibandingkan analisis *overhead* jaringan konvensional yang dilakukan oleh Malina et al. (2024). Dengan demikian, penelitian ini mengisi celah kritis antara desain akselerator NTT teoretis (Sonbul et al., 2025) dengan implementasi praktis pada instruksi set khusus (Ji et al., 2024) untuk menjamin kelayakan keamanan pasca-kuantum pada perangkat IoT bertenaga rendah.

# BAB III METODOLOGI PENELITIAN

Tahapan penelitian dibagi menjadi delapan tahap utama: (1) Studi Literatur, (2) Persiapan Alat dan Bahan, (3) Integrasi ML-KEM FIPS 203, (4) Perancangan Optimasi NTT menggunakan Instruksi Vektor PIE, (5) Konfigurasi Komunikasi MQTT v5.0, (6) Pengujian dan Pengambilan Data Komparatif, (7) Kalkulasi Skor *Quantum Encryption Resilience Score* (QERS), serta (8) Analisis Hasil dan Efisiensi.

## Alur Penelitian

Tahapan penelitian dimulai dengan studi literatur untuk memahami standar ML-KEM (FIPS 203) dan mekanisme instruksi vektor Xtensa LX7. Selanjutnya, dilakukan persiapan perangkat keras ESP32-S3 dan sensor INA219. Tahap ketiga dan keempat melibatkan integrasi pustaka ML-KEM serta implementasi optimasi fungsi NTT menggunakan register vektor 128-bit (q0–q7). Tahap kelima mengatur skema pertukaran data melalui protokol MQTT v5.0.

Pada tahap keenam, dilakukan pengujian komparatif antara varian Software NTT (SW) dan PIE NTT. Pada tahap ini, metrik jaringan seperti Packet Loss, RSSI, dan Jitter direkam secara simultan pada setiap iterasi untuk menangkap dinamika saluran komunikasi yang nyata. Variabel nirkabel ini diintegrasikan sebagai komponen penilaian dalam Rumusan Masalah ke-3 untuk mengukur tingkat kesiapan (*readiness*) sistem secara holistik. Hal ini didasarkan pada prinsip bahwa resiliensi kriptografi pada perangkat IoT sangat dipengaruhi oleh stabilitas transmisi di lingkungan yang tidak menentu (Rassekhnia, 2026). Tahap ketujuh melibatkan kalkulasi skor QERS berbasis *Multi-Criteria Decision Analysis* (MCDA) terhadap data yang telah dipraproses. Tahap akhir adalah analisis rasio percepatan (*speed-up*), efisiensi energi, dan implikasi teknis dari penggunaan optimasi PIE.

## Alat dan Bahan

Tahap persiapan alat dan bahan merupakan kegiatan mengumpulkan seluruh komponen perangkat keras dan perangkat lunak yang diperlukan untuk menjalankan pengujian. Hal ini krusial agar lingkungan pengujian bersifat stabil dan dapat direproduksi.

### Perangkat Keras (*Hardware*)

Perangkat keras yang digunakan dalam penelitian ini dipilih berdasarkan kemampuan pemrosesan vektor dan dukungan terhadap pengukuran daya presisi tinggi. Spesifikasi perangkat keras dirangkum dalam Tabel 3.1.

1. Spesifikasi Perangkat Keras Penelitian

|  |  |  |  |
| --- | --- | --- | --- |
| **No.** | **Perangkat** | **Spesifikasi / Kode Produk** | **Fungsi** |
| 1. | Mikrokontroler | ESP32-S3-DevKitC-1-N16R8 | *End-node* IoT dan unit pemrosesan ML-KEM. |
| 2. | Sensor Daya | Adafruit INA219 High Side DC Current Sensor | Mengukur konsumsi arus dan tegangan secara real-time. |
| 3. | Adapter USB | Micro USB Type B Female to DIP Adapter | Memfasilitasi akses ke jalur daya (VBUS/GND) untuk pengukuran. |
| 4. | Laptop A (Broker) | OS Linux Fedora | Sebagai perantara (transit node) yang mengelola antrean pesan MQTT v5.0. |
| 5. | Laptop B (Subscriber) | OS Windows | Sebagai titik akhir (trusted endpoint) yang melakukan de-enkripsi. |
| 6. | Penghubung | Kabel Jumper & Breadboard | Koneksi fisik antar modul hardware. |

Pemilihan ESP32-S3-DevKitC-1 dengan varian N16R8 didasarkan pada ketersediaan 8MB PSRAM yang diperlukan untuk menangani struktur data kisi pada ML-KEM yang memakan memori cukup besar (Espressif Systems, 2025). Sementara itu, penggunaan sensor INA219 bertujuan untuk mendapatkan akurasi pengukuran daya pada level miliampere sesuai dengan metodologi yang diterapkan oleh Lopez et al. (2025).

### Perangkat Lunak (*Software*)

Perangkat lunak yang digunakan mencakup kerangka pengembangan (*framework*) dan alat analisis jaringan. Spesifikasi perangkat lunak dirangkum dalam Tabel 3.2.

1. Spesifikasi Perangkat Lunak Penelitian

|  |  |  |  |
| --- | --- | --- | --- |
| **No.** | **Perangkat Lunak** | **Versi / Deskripsi** | **Fungsi** |
| 1. | Development Framework | ESP-IDF v5.4 | Kompilasi kode dan dukungan instruksi PIE. |
| 2. | MQTT Broker | Eclipse Mosquitto v2.0.22 | Mengelola antrean pesan MQTT v5.0. |
| 3. | Kriptografi Library | PQClean | Sumber kode standar ML-KEM FIPS 203. |
| 4. | Analisis Data | Python 3.13.12 | Kalkulasi skor QERS dan visualisasi data. |

Penggunaan ESP-IDF versi 5.4 sangat penting karena versi ini menyediakan dukungan kompilator yang optimal untuk instruksi vektor pada chip ESP32-S3 (Espressif Systems, 2025).

## Perancangan dan Implementasi Sistem

Tahap perancangan sistem merupakan inti teknis dari penelitian ini, di mana standar kriptografi ML-KEM diintegrasikan dan dioptimasi pada level instruksi mesin mikrokontroler ESP32-S3. Perancangan ini diawali dengan penyusunan skema perangkat keras untuk memastikan distribusi daya dan akurasi pengukuran selama pengujian berlangsung. Konfigurasi perakitan alat yang mengintegrasikan ESP32-S3, sensor INA219, dan Micro USB to DIP adapter disajikan secara visual pada Gambar 3.1.

![pqc.drawio](data:image/png;base64...)

1. Skema Perakitan Hardware untuk Pengukuran Daya

Arsitektur perakitan pada Gambar 3.1 mengintegrasikan jalur daya dan jalur komunikasi data dalam satu kesatuan sistem pemantauan. Arsitektur data masuk dan keluar pada skema ini dijelaskan sebagai berikut:

1. Jalur Daya (VBUS/GND): Arus listrik dari Adapter masuk ke sensor INA219 melalui terminal Vin+ untuk diukur intensitasnya, kemudian diteruskan melalui terminal Vin- menuju pin 5V pada ESP32-S3 sebagai sumber energi utama.
2. Jalur Komunikasi Kriptografi (I2C): Data hasil pengukuran daya dikirimkan dari INA219 kembali ke ESP32-S3 melalui jalur serial SDA dan SCL menggunakan protokol I2C.
3. Jalur Telemetri MQTT: ESP32-S3 memproses data daya tersebut dan menggabungkannya dengan metadata handshake ML-KEM, kemudian mengirimkannya secara nirkabel melalui antarmuka Wi-Fi menuju Broker.

Rincian koneksi antar-komponen untuk menjamin stabilitas arus dan akurasi pembacaan sensor dirangkum pada Tabel 3.5.

**Tabel 3.5**. Rincian Perakitan Pin Hardware Pemantauan Daya

|  |  |  |
| --- | --- | --- |
| **Dari Komponen (Pin)** | **Ke Komponen (Pin)** | **Deskripsi Koneksi** |
| Adapter (VBUS) | INA219 (Vin+) | Masukan tegangan utama untuk deteksi arus. |
| INA219 (Vin-) | ESP32-S3 (5V) | Luaran arus yang telah dipantau menuju beban (MCU). |
| INA219 (VCC) | ESP32-S3 (3V3) | Catu daya operasional modul sensor. |
| INA219 (GND) | Adapter (GND) & ESP32 (GND) | Titik referensi tanah bersama (common ground). |
| INA219 (SCL) | ESP32-S3 (IO9) | Jalur jam serial untuk sinkronisasi data I2C. |
| INA219 (SDA) | ESP32-S3 (IO8) | Jalur data serial dua arah untuk pembacaan metrik. |

Setelah perakitan perangkat keras dinyatakan stabil, tahap selanjutnya adalah melakukan integrasi pustaka kriptografi ke dalam lingkungan ESP-IDF.

### Integrasi ML-KEM (FIPS 203)

Proses integrasi dimulai dengan mengadopsi implementasi referensi ML-KEM dari proyek PQClean yang telah disesuaikan dengan standar final FIPS 203 (NIST, 2024). Pustaka tersebut kemudian di-porting ke dalam lingkungan pengembangan ESP-IDF v5.4.

Langkah-langkah integrasi ini meliputi konfigurasi file CMakeLists.txt untuk menyertakan sumber kode kriptografi dan penyusunan fungsi pembungkus (wrapper) dalam bahasa C. Hal ini dilakukan untuk mempermudah pemanggilan algoritma pembangkitan kunci, enkapsulasi, dan dekapsulasi yang secara operasional mengikuti spesifikasi pada dokumen standar NIST (NIST, 2024).

### Desain Optimasi PIE NTT

Optimasi pada bagian ini difokuskan pada percepatan operasi *Number Theoretic Transform* (NTT) yang merupakan komponen paling intensif dalam algoritma ML-KEM. Pada implementasi perangkat lunak standar, operasi butterfly diproses secara berurutan. Penelitian ini mengusulkan desain PIE NTT yang memanfaatkan unit *Processor Instruction Extensions* (PIE) pada arsitektur ESP32-S3 (Espressif Systems, 2025).

Optimasi PIE difokuskan pada percepatan operasi butterfly melalui instruksi SIMD 128-bit pada register vektor. Pada implementasi saat ini, percepatan dominan terjadi pada operasi penjumlahan dan pengurangan vektor di dalam tahap NTT dan Inverse NTT, sedangkan operasi perkalian modular serta reduksi Montgomery masih diproses secara skalar oleh inti CPU. Meskipun optimasi bersifat parsial, penggunaan register vektor ini diproyeksikan memberikan peningkatan efisiensi siklus komputasi yang terukur pada jalur transformasi polinomial. Logika dasar untuk implementasi transformasi ini mengacu pada algoritma NTT iteratif yang didefinisikan dalam Algoritma 3.1.

![Screenshot From 2026-04-02 07-19-52](data:image/png;base64...)

1. Algoritma NTT Iteratif (Aikata et al., 2023)

Berdasarkan Algoritma 3.1, optimasi dilakukan dengan memetakan jalur aritmatika unit butterfly ke dalam instruksi vektor SIMD. Struktur unit butterfly terpadu yang diadaptasi ke dalam desain hardware ini ditunjukkan pada Gambar 2.2.

Melalui implementasi pada Gambar 2.2 tersebut, setiap instruksi PIE pada ESP32-S3 diprogram untuk mengeksekusi beberapa jalur aritmatika secara simultan, sehingga mengurangi jumlah total siklus CPU secara signifikan (Sonbul et al., 2025).

### Konfigurasi Protokol MQTT v5.0

Sistem dikonfigurasi menggunakan protokol MQTT v5.0 untuk memfasilitasi pertukaran material handshake ML-KEM antara perangkat end-node dan aplikasi host. Penelitian ini mengimplementasikan pengiriman public key (pk) dan ciphertext (ct) melalui payload pada topik MQTT khusus dengan format pengkodean Base64.

![IMG_256](data:image/png;base64...)

1. Alur Komunikasi Handshake ML-KEM melalui MQTT

Alur kerja komunikasi aman yang mengintegrasikan fase enkapsulasi dan dekapsulasi kunci ini disajikan secara sistematis pada Gambar 3.2.

Dalam skema yang ditunjukkan pada Gambar 3.2, Broker MQTT berfungsi sebagai perantara publish/subscribe, sementara seluruh proses komputasi kriptografi dilakukan secara lokal pada masing-masing titik komunikasi. Skema ini menunjukkan bahwa fokus utama analisis adalah pada performa komputasi ML-KEM di sisi *end-device* (ESP32-S3) untuk mengukur efektivitas instruksi vektor dalam menangani beban kerja tersebut dalam skenario komunikasi nyata.

Implementasi protokol MQTT v5.0 dalam penelitian ini mengadopsi topologi terdistribusi guna merepresentasikan model arsitektur Zero Trust (ZTA). Pemisahan perangkat menjadi dua entitas laptop yang berbeda bertujuan untuk mengisolasi peran Broker sebagai jaringan yang tidak dipercaya (untrusted network) dan Laptop Subscriber sebagai unit manajemen kredensial. Pengaturan ini memungkinkan pengukuran metrik latency (L) dan jitter (J) yang lebih akurat karena paket data harus melewati saluran fisik nyata antar-hop, bukan melalui antarmuka loopback internal. Alur data dimulai dari ESP32-S3 yang mempublikasikan material kunci ke Laptop A (Broker), yang kemudian diteruskan ke Laptop B (Subscriber) untuk proses kalkulasi resiliensi menggunakan metrik QERS terutama saat membandingkan dua varian implementasi yang memiliki karakteristik penggunaan sumber daya yang berbeda (Rassekhnia, 2026).

### Penentuan Ambang Batas Kelayakan (*Thresholding*)

Penelitian ini menetapkan tiga ambang batas kelayakan (*threshold*) sebagai instrumen determinasi untuk menilai apakah varian ML-KEM-512, 768, dan 1024 layak diimplementasikan pada ESP32-S3. Penggunaan multi-kriteria ini didasarkan pada prinsip pengambilan keputusan atribut jamak untuk menjamin reliabilitas operasional (Vaidya & Kumar, 2006). Ambang batas yang ditetapkan adalah sebagai berikut:

1. Ambang Batas Latensi *Handshake* ()

Merujuk pada standar aplikasi IoT industri dengan batasan waktu nyata (*real-time*), latensi komputasi tidak boleh melampaui 100 ms untuk menghindari kegagalan sinkronisasi atau *timeout* pada broker MQTT (Chhetri, 2026).

1. Ambang Batas Penggunaan Memori ()

Berdasarkan kapasitas SRAM ESP32-S3 sebesar 512 KB (Baraskar et al., 2025), konsumsi memori stack dibatasi maksimal 10% dari total memori agar tidak menginterupsi jalannya program aplikasi utama. Batas ini sangat krusial karena ML-KEM-1024 diketahui memerlukan alokasi stack yang signifikan untuk de-enkapsulasi (Chhetri, 2026).

1. Ambang Batas Konsumsi Energi ()

Mengacu pada model efisiensi energi perangkat bertenaga baterai, satu siklus pertukaran kunci diharapkan menghabiskan energi di bawah 10 mJ (Chhetri, 2026). Sebagai perbandingan, ML-KEM-512 pada perangkat terbatas hanya mengonsumsi sekitar 2,83 mJ, sementara ML-KEM-1024 bisa mencapai 6,79 mJ (Chhetri, 2026).

Jika suatu varian ML-KEM melampaui salah satu dari ketiga kriteria di atas, maka varian tersebut diklasifikasikan sebagai 'Beresiko' untuk operasional *bare-metal* pada perangkat terbatas (Fitzgibbon & Ottaviani, 2024).

## Skenario Pengujian

Tahap pengujian dirancang untuk mengevaluasi efektivitas instruksi vektor PIE secara bertingkat melalui dua fase sistematis guna memastikan validitas internal dan eksternal data. Rincian fase pengujian dijelaskan sebagai berikut:

1. Fase 1: Standalone Performance Benchmarking (Internal)

Fase ini berfokus pada pengukuran performa komputasi murni dari algoritma ML-KEM-512 di dalam mikrokontroler ESP32-S3 secara terisolasi dari beban jaringan (Chhetri, 2026). Ketiga fungsi utama standar FIPS 203, yaitu Key Generation (KeyGen), Encapsulation (Encaps), dan Decapsulation (Decaps) dijalankan secara lokal. Metrik yang dicatat adalah jumlah siklus CPU dan waktu eksekusi () untuk membandingkan rasio percepatan (speed-up) antara varian Software NTT dan PIE NTT secara absolut.

1. Fase 2: Integrated MQTT Handshake Evaluation (Network)

Fase ini mengevaluasi dampak optimasi pada skenario komunikasi nyata menggunakan protokol MQTT v5.0. ESP32-S3 bertindak sebagai Publisher (KeyGen dan Decaps), sementara Laptop bertindak sebagai Subscriber (Encaps). Pengujian ini bertujuan untuk mengukur performa sistemik dan stabilitas jabat tangan kriptografi di bawah pengaruh kondisi jaringan nirkabel nyata.

Tahap pengujian juga dirancang untuk mengevaluasi efektivitas penggunaan instruksi vektor secara komparatif melalui dua skenario utama yang dijalankan pada modul ESP32-S3. Pembagian skenario ini bertujuan untuk memberikan data empiris mengenai sejauh mana akselerasi tingkat instruksi memengaruhi performa keseluruhan sistem. Rincian perbedaan teknis antar skenario dirangkum dalam Tabel 3.3.

1. Deskripsi Skenario Pengujian Komparatif

|  |  |  |
| --- | --- | --- |
| **Komponen** | **Skenario A**  **(Software NTT)** | **Skenario B**  **(PIE NTT)** |
| **Implementasi NTT** | Kode C referensi (skalar) dari proyek PQClean. | Modifikasi fungsi NTT menggunakan instruksi PIE. |
| **Unit Aritmatika** | Operasi aritmatika standar Xtensa LX7. | Instruksi vektor SIMD 128-bit. |
| **Manajemen Register** | Register umum (*General Purpose Registers*). | Register vektor. |
| **Metode Komputasi** | Pemrosesan elemen polinomial secara berurutan. | Pemrosesan elemen penjumlahan/pengurangan secara paralel. |

Untuk menjamin validitas hasil perbandingan pada kedua skenario di atas, seluruh pengujian dilakukan dalam kondisi lingkungan yang identik. Pengujian dilakukan di dalam jaringan lokal (LAN) untuk meminimalkan fluktuasi latensi eksternal yang dapat mendistorsi data waktu round-trip. Penelitian ini menetapkan jumlah 100 iterasi untuk setiap skenario pengujian guna menjamin validitas data melalui pendekatan statistik yang ketat. Angka ini dipilih berdasarkan integrasi tiga landasan teoretis:

1. Signifikansi Statistik: jumlah n = 100 melampaui batas minimum Teorema Limit Pusat (), sehingga memungkinkan distribusi rata-rata sampel mendekati distribusi normal dan meminimalkan galat estimasi (Georges et al., 2007)
2. Karakteristik Algoritma: ML-KEM berbasis kisi memiliki variansi latensi yang fluktuatif akibat mekanisme rejection sampling, sehingga dibutuhkan sampel besar untuk menangkap fenomena tail latency pada persentil ke-95 dan ke-99 secara akurat (Chhetri, 2026)
3. Standar Komparasi: penggunaan 100 repetisi selaras dengan protokol benchmarking kriptografi pasca-kuantum pada saluran nirkabel guna memastikan hasil perbandingan antara akselerasi hardware dan perangkat lunak murni bersifat konsisten dan bebas dari bias artifact sistem (Jain, 1991; Rios et al., 2025).

## Teknik Pengumpulan Data

Metrik evaluasi utama yang dicatat mencakup aspek komputasi, jaringan, dan energi. Untuk kepentingan analisis resiliensi pada Rumusan Masalah ke-3, Latency (L) didefinisikan secara spesifik sebagai Handshake Round-Trip Time (RTT), yaitu total waktu sejak paket Public Key dikirim oleh ESP32-S3 hingga paket Ciphertext selesai didekapsulasi (Rassekhnia, 2026). Definisi ini dipilih karena mampu menangkap dampak fragmentasi paket ML-KEM pada layer transport yang sering kali melampaui batas Maximum Transmission Unit (MTU) standar (Malina et al., 2024; Rios et al., 2025).

Pengumpulan data jaringan seperti *Packet Loss*, RSSI, dan Jitter dilakukan guna memotret performa sistem secara natural. Meskipun Rumusan Masalah 1 dan 2 berfokus pada efisiensi komputasi dan energi internal pada ESP32-S3, data jaringan tetap menjadi variabel input yang valid bagi analisis resiliensi. Integrasi fluktuasi kondisi jaringan nirkabel ke dalam pengujian memungkinkan evaluasi yang lebih kredibel terhadap perilaku protokol MQTT v5.0 saat menangani beban paket ML-KEM yang besar terutama saat membandingkan dua varian implementasi yang memiliki karakteristik penggunaan sumber daya yang berbeda (Rassekhnia, 2026).

Pengukuran konsumsi energi dilakukan menggunakan sensor INA219 melalui dua jendela pengambilan data (power profiling):

1. Energi Komputasi (): Mengukur daya secara terpisah untuk masing-masing fungsi standar FIPS 203, yaitu , , dan . Hal ini dilakukan dengan mengisolasi pembacaan sensor tepat saat masing-masing rutin algoritma aktif di dalam CPU guna mengukur efisiensi riil dari instruksi vektor PIE pada setiap tahap operasional kunci.
2. Energi Sesi (): Mengukur daya total selama satu siklus jabat tangan MQTT berlangsung guna menilai kelayakan operasional pada perangkat bertenaga baterai (Konduru et al., 2025).

Alur pengumpulan metrik ini secara sistematis mengikuti kerangka kerja yang ditunjukkan pada Gambar 2.4.

## Analisis Data dan Evaluasi (Skor QERS)

Data hasil pengujian komparatif antara varian Software NTT dan PIE NTT dianalisis secara kuantitatif untuk menentukan tingkat resiliensi sistem. Proses analisis dilakukan melalui tahapan sistematis sebagai berikut:

1. Praproses Data

Tahap ini difokuskan pada pembersihan data (data cleaning) untuk memastikan tidak ada pencilan (outliers) yang disebabkan oleh kegagalan sistemik perangkat keras (Jain, 1991). Berbeda dengan evaluasi performa murni, perhitungan skor QERS dalam penelitian ini tetap mempertahankan variansi nilai Packet Loss (Ploss), RSSI (R), dan Jitter (J) yang terekam. Hal ini bertujuan agar skor akhir resiliensi mencerminkan kemampuan adaptasi sistem terhadap interferensi fisik dan beban trafik MQTT, sehingga memberikan gambaran kesiapan operasional yang akurat terutama saat membandingkan dua varian implementasi yang memiliki karakteristik penggunaan sumber daya yang berbeda (Rassekhnia, 2026).

1. Penghitungan Metrik Turunan (Fusion QERS)

Metrik turunan dalam tahap ini dikalkulasi secara khusus untuk memenuhi variabel input dalam model Fusion QERS. Metrik di sini diekstraksi dari fase Integrated MQTT Handshake Evaluation untuk mendapatkan nilai representatif per sesi komunikasi (Rassekhnia, 2026). Mekanisme penyusunan metrik turunan dirincikan sebagai berikut:

* Handshake Latency (L): Akumulasi waktu total sejak inisiasi pengiriman public key oleh ESP32-S3 hingga proses de-enkapsulasi ciphertext selesai sepenuhnya, mencerminkan dampak sistemik fragmentasi paket nirkabel (Rios et al., 2025; Rassekhnia, 2026).
* Energy (E): Integrasi daya terhadap waktu selama satu siklus jabat tangan MQTT berlangsung secara kontinu guna menilai beban energi sesi (Konduru et al., 2025; Rassekhnia, 2026).
* CPU Utilization (C): Rata-rata beban kerja inti CPU ESP32-S3 selama sesi jabat tangan aktif (Rassekhnia, 2026).
* Packet Loss (Ploss) & Jitter (J): Diperoleh melalui ekstraksi log statistik Broker MQTT v5.0 untuk memetakan reliabilitas saluran (Banks et al., 2019; Rassekhnia, 2026).
* Cryptographic Overhead (O): Penjumlahan antara konsumsi memori heap internal dan beban lebar pita transmisi paket (Rassekhnia, 2026).
* Key Size (K): Ukuran data paket material kunci sesuai spesifikasi FIPS 203 (NIST, 2024; Rassekhnia, 2026).
* RSSI (R): Nilai kekuatan sinyal fisik yang terekam pada antarmuka radio Wi-Fi perangkat terutama saat membandingkan dua varian implementasi yang memiliki karakteristik penggunaan sumber daya yang berbeda (Rassekhnia, 2026).
* Proven Resistance (Pr): Nilai bobot resiliensi teoretis terhadap algoritma Shor/Grover di mana ML-KEM-512 bernilai 1, ML-KEM-768 bernilai 3, dan ML-KEM-1024 bernilai 5 sesuai kategori keamanan NIST (NIST, 2024; Rassekhnia, 2026).

1. Normalisasi

Melakukan normalisasi min-max (0-100) pada seluruh metrik untuk menyamakan skala pengukuran menggunakan Persamaan 2.4.

1. Kalkulasi Skor QERS

Penelitian ini mengadopsi model Fusion QERS sebagai instrumen evaluasi akhir untuk menentukan tingkat resiliensi dan kesiapan sistem. Pemilihan model Fusion didasarkan pada kemampuannya untuk mengintegrasikan variabel performa operasional dan karakteristik keamanan melalui pendekatan Multi-Criteria Decision Analysis (MCDA) (Rassekhnia, 2026). Skor akhir dihitung dengan menggabungkan subskor Performa (P) yang dinamis akibat optimasi PIE dan subskor Keamanan (S) yang berbasis pada standar kriptografi.

Penentuan bobot untuk setiap metriks dalam subskor Performa (P) menggunakan metode Analytic Hierarchy Process (AHP) untuk menjamin konsistensi perbandingan antar-variabel (Saaty, 1987). Berdasarkan kebutuhan aplikasi IoT industri yang mengutamakan kecepatan respon dan efisiensi energi (Konduru et al., 2025), distribusi bobot ditetapkan pada Tabel 3.4.

Tabel 3.4. Distribusi Bobot Metriks pada Subskor Performa (P) QERS Fusion

|  |  |  |  |
| --- | --- | --- | --- |
| **Simbol** | **Metrik** | **Bobot** | **Keterangan** |
| L | *Latency* | 0.35 | Prioritas tertinggi karena ML-KEM memiliki variansi latensi yang tinggi, dan kecepatan handshake menentukan kelayakan real-time (Chhetri, 2026). |
| E | *Energy* | 0.25 | Komponen krusial dalam Sustainable IoT guna memaksimalkan masa pakai baterai ESP32-S3 (Konduru et al., 2025). |
| C | *CPU Utilization* | 0.20 | Mengukur efektivitas instruksi vektor PIE dalam mengurangi beban kerja inti prosesor (Baraskar et al., 2025). |
|  | *Packet Loss* | 0.10 | Bobot lebih rendah karena pengujian dilakukan pada LAN, namun tetap dihitung untuk menilai reliabilitas protokol (Jain, 1991). |
| J | *Jitter* | 0.10 | Menjamin stabilitas aliran data telemetri MQTT pada jaringan terdistribusi (Rassekhnia, 2026). |
| **Total** | | 1.0 |  |

Untuk subskor Keamanan (S), bobot kriteria ditentukan berdasarkan dampak kriptografi terhadap perangkat terbatas (Table 3.5).

Tabel 3.5. Distribusi Bobot Metriks pada Subskor Keamanan (S) QERS Fusion

|  |  |  |  |
| --- | --- | --- | --- |
| **Simbol** | **Metrik** | **Bobot** | **Keterangan** |
| Pr | *Proven Resistance* | 0.40 | Parameter utama keamanan pasca-kuantum berdasarkan level NIST (NIST, 2024). |
| Co | *Overhead* | 0.30 | Dampak penggunaan memori dan lebar pita data terhadap batasan sumber daya perangkat (Bos et al., 2018). |
| K | *Key Size* | 0.20 | Ukuran kunci ML-KEM mempengaruhi fragmentasi paket pada layer transport MQTT (Malina et al., 2024). |
| R | *RSSI* | 0.10 | Manfaat dari kekuatan sinyal fisik lingkungan pengujian (Rassekhnia, 2026). |
| **Total** | | 1.0 |  |

Penggabungan kedua subskor tersebut ke dalam skor Fusion QERS (Persamaan 2.9) menggunakan koefisien penyeimbang dan . Pembobotan yang jauh lebih tinggi diberikan karena fokus utama penelitian adalah pada optimasi performa operasional melalui akselerasi hardware, sementara aspek keamanan dianggap sebagai prasyarat standar yang tetap terutama saat membandingkan dua varian implementasi yang memiliki karakteristik penggunaan sumber daya yang berbeda (Rassekhnia, 2026). Hasil akhir dari perhitungan ini adalah nilai komposit (0-100) yang menunjukkan perbandingan tingkat kesiapan resiliensi antara implementasi Software NTT dan PIE NTT.

1. Analisis Statistik

Tahap analisis statistik dilakukan untuk mengevaluasi data atomik yang diperoleh guna menjawab analisis komparatif antara varian Software NTT dan PIE NTT.

* Performa Komputasi: Dilakukan komparasi nilai rata-rata (mean) dan deviasi standar dari Time Execution () dan CPU Cycles yang tercatat pada fase Standalone Performance Benchmarking untuk fungsi KeyGen, Encaps, dan Decaps secara terpisah (Chhetri, 2026; Kannwischer et al., 2019).
* Speed-up Ratio: Menghitung rasio percepatan (S) menggunakan Persamaan 3.1:
* Network RTT: Membandingkan nilai Round-Trip Time (RTT) jabat tangan MQTT antarvarian untuk melihat pengaruh akselerasi hardware pada latensi jaringan nyata (Rios et al., 2025).
* Konsumsi Energi Komputasi: Menganalisis profil penggunaan daya listrik (mJ) untuk setiap fungsi KeyGen, Encaps, dan Decaps secara terpisah berdasarkan data sensor INA219 selama fase Standalone (Chhetri, 2026; Konduru et al., 2025).
* Signifikansi Statistik: Seluruh data diuji menggunakan metode Analysis of Variance (ANOVA) guna memastikan perbedaan performa yang terekam memiliki signifikansi ilmiah dan bukan merupakan hasil dari variansi acak sistem (Jain, 1991; Georges et al., 2007).

# BAB IV HASIL DAN PEMBAHASAN

Bab ini menyajikan hasil pengujian implementasi standar ML-KEM pada ESP32-S3. Pembahasan dimulai dari pengumpulan data, praproses data, hasil benchmark komputasi lokal, hasil handshake MQTT, perhitungan QERS, dan interpretasi akhir. Urutan tersebut digunakan agar pembaca dapat memahami kualitas data sebelum melihat analisis performa dan rekomendasi konfigurasi.

## Pengumpulan Data Hasil Pengujian

Pengumpulan data dilakukan melalui dua skenario. Scenario 2 digunakan untuk mengukur komputasi lokal ML-KEM pada ESP32-S3. Scenario 1 digunakan untuk mengukur performa ML-KEM ketika public key dan ciphertext dipertukarkan melalui MQTT v5.0. Pemisahan skenario ini penting karena performa komputasi lokal belum tentu sama dengan performa komunikasi end-to-end.

Scenario 2 menghasilkan enam file CSV. File tersebut berasal dari kombinasi tiga varian ML-KEM dan dua status PIE. Setiap file memuat hasil KeyGen, Encaps, dan Decaps sebanyak 100 iterasi per operasi. Metrik yang dicatat meliputi waktu eksekusi, siklus CPU, energi, heap, dan stack high-water mark.

Tabel 4.1. Matriks dataset Scenario 2.

|  |  |  |  |
| --- | --- | --- | --- |
| **Varian** | **PIE** | **Operasi** | **Iterasi per operasi** |
| ML-KEM-512 | off/on | KeyGen, Encaps, Decaps | 100 |
| ML-KEM-768 | off/on | KeyGen, Encaps, Decaps | 100 |
| ML-KEM-1024 | off/on | KeyGen, Encaps, Decaps | 100 |

Scenario 1 menghasilkan 12 file joined CSV. File tersebut berasal dari kombinasi tiga varian ML-KEM, dua status PIE, dan dua mode koneksi. Pada skenario ini, ESP32-S3 menjalankan KeyGen dan Decaps, sedangkan subscriber Python menjalankan Encaps. Data subscriber kemudian digabung dengan data ESP32-S3 berdasarkan nomor iterasi.

Tabel 4.2. Matriks dataset Scenario 1.

|  |  |  |
| --- | --- | --- |
| **Faktor** | **Nilai** | **Jumlah Kombinasi** |
| Varian ML-KEM | 512, 768, 1024 | 3 |
| Status PIE | pieoff, pieon | 2 |
| Mode koneksi | session, reconnect | 2 |
| Total konfigurasi | 3 × 2 × 2 | 12 |
| Iterasi per konfigurasi | 100 | 1200 baris |

Gambar 4.1. Alur pembentukan dataset Scenario 1 dan Scenario 2.

Keterangan: Gambar 4.1 perlu menampilkan dua cabang. Cabang pertama menunjukkan Scenario 2 dari ESP32-S3 menuju CSV lokal. Cabang kedua menunjukkan Scenario 1 dari ESP32-S3, broker, subscriber, proses join CSV, dan CSV joined. Gambar ini dibuat sendiri berdasarkan alur eksperimen.

## Praproses Data Hasil Pengujian

Praproses data dilakukan untuk memastikan hasil pengujian dapat dibandingkan secara adil. Langkah pertama adalah memeriksa struktur kolom pada setiap CSV. Langkah kedua adalah memeriksa jumlah iterasi. Langkah ketiga adalah memeriksa packet loss pada Scenario 1. Langkah keempat adalah menyiapkan metrik turunan seperti total latency dan total energy untuk mode reconnect.

Hasil validasi menunjukkan bahwa seluruh konfigurasi Scenario 1 memiliki 100 iterasi. Seluruh file joined juga memiliki nilai Encaps dari subscriber. Packet loss bernilai nol pada semua konfigurasi. Kondisi ini menunjukkan bahwa hasil Scenario 1 dapat dianalisis tanpa membuang baris akibat kegagalan handshake.

Tabel 4.3. Validasi data Scenario 1.

|  |  |  |  |  |  |
| --- | --- | --- | --- | --- | --- |
| **Varian** | **PIE** | **Mode** | **Baris** | **Packet loss** | **Status** |
| ML-KEM-512 | pieoff | reconnect | 100 | 0 | Valid |
| ML-KEM-512 | pieoff | session | 100 | 0 | Valid |
| ML-KEM-512 | pieon | reconnect | 100 | 0 | Valid |
| ML-KEM-512 | pieon | session | 100 | 0 | Valid |
| ML-KEM-768 | pieoff | reconnect | 100 | 0 | Valid |
| ML-KEM-768 | pieoff | session | 100 | 0 | Valid |
| ML-KEM-768 | pieon | reconnect | 100 | 0 | Valid |
| ML-KEM-768 | pieon | session | 100 | 0 | Valid |
| ML-KEM-1024 | pieoff | reconnect | 100 | 0 | Valid |
| ML-KEM-1024 | pieoff | session | 100 | 0 | Valid |
| ML-KEM-1024 | pieon | reconnect | 100 | 0 | Valid |
| ML-KEM-1024 | pieon | session | 100 | 0 | Valid |

Rumus 4.1. Speed-up PIE untuk metrik cost.

speed-up = nilai\_pieoff / nilai\_pieon

Rumus 4.2. Total latency mode reconnect.

total\_latency\_us = reconnect\_us + handshake\_us

Rumus 4.3. Total energy mode reconnect.

total\_energy\_uj = reconnect\_energy\_uj + energy\_uj

## Hasil Pengujian Scenario 2

Scenario 2 digunakan untuk mengisolasi efek PIE dari pengaruh jaringan. Hasil pengujian menunjukkan bahwa PIE menurunkan waktu eksekusi dan siklus CPU pada seluruh varian dan seluruh operasi. Penurunan ini menunjukkan bahwa jalur vektor pada ESP32-S3 berhasil mempercepat bagian komputasi ML-KEM.

|  |  |  |  |  |  |  |  |  |  |  |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **Varian** | **Operasi** | **Time No PIE (µs)** | **Time PIE (µs)** | **Δ Time** | **Cycles No PIE** | **Cycles PIE** | **Δ Cycles** | **Energy No PIE (µJ)** | **Energy PIE (µJ)** | **Δ Energy** |
| ML-KEM-512 | KeyGen | 7.765,0 | 7.487,0 | -3,58% | 1.242.119 | 1.197.602 | -3,58% | 1.395,5 | 1.606,0 | +15,08% |
| ML-KEM-512 | Encaps | 9.211,5 | 8.840,0 | -4,03% | 1.473.624 | 1.414.228 | -4,03% | 1.456,5 | 1.676,5 | +15,10% |
| ML-KEM-512 | Decaps | 11.205,0 | 10.646,0 | -4,99% | 1.792.732 | 1.703.231 | -4,99% | 1.825,5 | 1.411,5 | -22,68% |
| ML-KEM-768 | KeyGen | 12.344,5 | 11.826,0 | -4,20% | 1.974.784 | 1.891.753 | -4,20% | 1.727,5 | 1.928,5 | +11,64% |
| ML-KEM-768 | Encaps | 14.520,0 | 13.874,5 | -4,45% | 2.322.944 | 2.219.601 | -4,45% | 2.848,0 | 2.584,5 | -9,25% |
| ML-KEM-768 | Decaps | 17.164,5 | 16.251,0 | -5,32% | 2.746.047 | 2.599.957 | -5,32% | 3.859,0 | 3.281,5 | -14,97% |
| ML-KEM-1024 | KeyGen | 18.721,0 | 17.993,0 | -3,89% | 2.995.247 | 2.878.482 | -3,90% | 2.891,0 | 3.808,0 | +31,72% |
| ML-KEM-1024 | Encaps | 21.423,0 | 20.485,0 | -4,38% | 3.427.424 | 3.277.282 | -4,38% | 5.593,5 | 4.606,5 | -17,65% |
| ML-KEM-1024 | Decaps | 24.759,0 | 23.513,0 | -5,03% | 3.960.992 | 3.761.773 | -5,03% | 4.797,5 | 5.422,0 | +13,02% |

Tabel 4.4. Perbandingan PIE dan No PIE pada Scenario 2.

# JADWAL PELAKSANAAN

|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| No. | Aktivitas | Waktu | | | | | | | | | | | | | | | |
| Maret  2026 | | | | April  2026 | | | | Mei  2026 | | | | Juni  2026 | | | |
| 1 | 2 | 3 | 4 | 1 | 2 | 3 | 4 | 1 | 2 | 3 | 4 | 1 | 2 | 3 | 4 |
| 1. | Penyusunan dan Finalisasi Proposal Penelitian |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| 2. | Integrasi Awal dan Setup Lingkungan Pengembangan |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| 3. | Pengembangan Program dan Optimasi PIE NTT |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| 4. | Pengujian Skenario Software NTT (SW) dan Skenario PIE NTT (Hardware Acceleration) |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| 5. | Analisis Data dan Kalkulasi Skor QERS |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| 6. | Penyusunan Laporan Akhir (Skripsi) dan Evaluasi |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |

# Daftar Pustaka

Aikata, A., Mert, A. C., Imran, M., Pagliarini, S., & Roy, S. S. (2023). KaLi: A Crystal for Post-Quantum Security Using Kyber and Dilithium. IEEE Transactions on Circuits and Systems I: Regular Papers, 70(2), 747–758. <https://doi.org/10.1109/TCSI.2022.3219555>

Almutairi, M., & Sheldon, F. T. (2025). Resilience of Post-Quantum Cryptography in Lightweight IoT Protocols: A Systematic Review. Eng, 6(12), 346. <https://doi.org/10.3390/eng6120346>

Asif, R. (2021). Post-Quantum Cryptosystems for Internet-of-Things: A Survey on Lattice-Based Algorithms. IoT, 2(1), 71–91. <https://doi.org/10.3390/iot2010005>

Cherian, M., & Chatterjee, M. (2019). Survey of Security Threats in IoT and Emerging Countermeasures. In S. M. Thampi, S. Madria, G. Wang, D. B. Rawat, & J. M. Alcaraz Calero (Eds.), Security in Computing and Communications (pp. 591–604). Springer. <https://doi.org/10.1007/978-981-13-5826-5_46>

Espressif Systems. (2025). ESP32-S3 Technical Reference Manual Version 1.7. Espressif Systems.

Fitzgibbon, G., & Ottaviani, C. (2024). Constrained Device Performance Benchmarking with the Implementation of Post-Quantum Cryptography. Cryptography, 8(2), 21. <https://doi.org/10.3390/cryptography8020021>

Ji, X., Dong, J., Huang, J., Yuan, Z., Dai, W., Xiao, F., & Lin, J. (2024). ECO-CRYSTALS: Efficient Cryptography CRYSTALS on Standard RISC-V ISA (No. 2024/1198). Cryptology ePrint Archive. <https://eprint.iacr.org/2024/1198>

Kumar, S., Tiwari, P., & Zymbler, M. (2019). Internet of Things is a revolutionary approach for future technology enhancement: A review. Journal of Big Data, 6(1), 111. <https://doi.org/10.1186/s40537-019-0268-2>

Lopez, J., Cadena, V., & Rahman, M. S. (2025). Evaluating Post-Quantum Cryptographic Algorithms on Resource-Constrained Devices. arXiv. <https://doi.org/10.48550/arXiv.2507.08312>

Malina, L., Dobias, P., Dzurenda, P., & Srivastava, G. (2024). Quantum-Resistant and Secure MQTT Communication. Proceedings of the 19th International Conference on Availability, Reliability and Security (ARES ’24), 1–8. <https://doi.org/10.1145/3664476.3670463>

National Institute of Standards and Technology (NIST). (2024). Module-Lattice-Based Key-Encapsulation Mechanism Standard (Federal Information Processing Standard [FIPS] 203). U.S. Department of Commerce. <https://doi.org/10.6028/NIST.FIPS.203>

OASIS Open. (2019). MQTT Version 5.0 Standard Specification. OASIS Standard. <https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html>

Rassekhnia, J. (2026). Quantum Encryption Resilience Score (QERS) for MQTT, HTTP, and HTTPS under Post-Quantum Cryptography in Computer, IoT, and IIoT Systems. arXiv. <https://doi.org/10.48550/arXiv.2601.13423>

Segatz, F., & Hafiz, M. I. A. (2025). Efficient Implementation of CRYSTALS-KYBER Key Encapsulation Mechanism on ESP32. arXiv. <https://doi.org/10.48550/arXiv.2503.10207>

Sonbul, O. S., Rashid, M., & Jaffar, A. Y. (2025). Accelerating CRYSTALS-Kyber: High-Speed NTT Design with Optimized Pipelining and Modular Reduction. Electronics, 14(11), 2122. <https://doi.org/10.3390/electronics14112122>

Walker, K., & Neven, H. (2026, February 6). The quantum era is coming. Are we ready to secure it? Google Security Blog. <https://blog.google/innovation-and-ai/technology/safety-security/the-quantum-era-is-coming-are-we-ready-to-secure-it/>

TAMBAHAN:

Ali, S. A. M., Abuanas, S., Nadeem, S., & Ahmed, S. (2026). AI VOICE ASSISTANT USING ESP32 AND CLOUD AI SERVICES. INTERNATIONAL JOURNAL OF CREATIVE RESEARCH THOUGHTS, 14(4).

Arregui Almeida, D., Chafla Altamirano, J., Román Cañizares, M., Játiva, P. P., Guaña-Moya, J., & Sánchez, I. (2025). Gateway-Free LoRa Mesh on ESP32: Design, Self-Healing Mechanisms, and Empirical Performance. Sensors, 25(19), 6036. <https://doi.org/10.3390/s25196036>

Baraskar, Y. S., Gawande, D. P., & Ambhore, V. (2025). Exploring the Capabilities of ESP32-S3: An Advanced IoT Development Board for Next-Generation Edge Computing. International Journal of Innovative Research in Science, Engineering and Technology, 14(11). <https://doi.org/10.15680/IJIRSET.2025.1411139>

Boonmeeruk, P., Palrat, P., & Wongsopanakul, K. (2024). Cost-Effective IIoT Gateway Development Using ESP32 for Industrial Applications. Engineering Journal, 28(10), 93–108. <https://doi.org/10.4186/ej.2024.28.10.93>

Chanal, P. M., & Kakkasageri, M. S. (2020). Security and Privacy in IoT: A Survey. Wirel. Pers. Commun., 115(2), 1667–1693. <https://doi.org/10.1007/s11277-020-07649-9>

Bos, J., Ducas, L., Kiltz, E., Lepoint, T., Lyubashevsky, V., Schanck, J. M., Schwabe, P., Seiler, G., & Stehle, D. (2018). CRYSTALS - Kyber: A CCA-Secure Module-Lattice-Based KEM. 2018 IEEE European Symposium on Security and Privacy (EuroS&P), 353–367. <https://doi.org/10.1109/EuroSP.2018.00032>

Baraskar, Y. S., Gawande, D. P., & Ambhore, V. (2025). Exploring the Capabilities of ESP32-S3: An Advanced IoT Development Board for Next-Generation Edge Computing. International Journal of Innovative Research in Science, Engineering and Technology, 14(11).

Vaidya, O. S., & Kumar, S. (2006). Analytic hierarchy process: An overview of applications. European Journal of Operational Research, 169(1), 1–29. <https://doi.org/10.1016/j.ejor.2004.04.028>

Chhetri, R. (2026). Benchmarking NIST-Standardised ML-KEM and ML-DSA on ARM Cortex-M0+: Performance, Memory, and Energy on the RP2040. <https://doi.org/10.5281/zenodo.19393777>

Georges, A., Buytaert, D., & Eeckhout, L. (2007). Statistically rigorous java performance evaluation. Proceedings of the 22nd Annual ACM SIGPLAN Conference on Object-Oriented Programming Systems, Languages and Applications, OOPSLA ’07, 57–76. <https://doi.org/10.1145/1297027.1297033>

Jain, R. (1991). The art of computer systems performance analysis: Techniques for experimental design, measurement, simulation, and modeling. Wiley.

Rios, R., Montenegro, J. A., Muñoz, A., & Ferraris, D. (2025). Toward the Quantum-Safe Web: Benchmarking Post-Quantum TLS. IEEE Network, 39(5), 247–253. <https://doi.org/10.1109/MNET.2025.3531116>

Konduru, R. K., et al. (2025). Sustainable IoT Applications: Energy-Efficient Strategies and Green Computing Paradigms. Social Science Research Network. <https://doi.org/10.2139/ssrn.5425115>

Saaty, R. W. (1987). The analytic hierarchy process—What it is and how it is used. Mathematical Modelling, 9(3), 161–176. [https://doi.org/10.1016/0270-0255(87)90473-8](https://doi.org/10.1016/0270-0255%2887%2990473-8)

Banks, A., Briggs, E., Borgendale, K., & Gupta, R. (2019). MQTT Version 5.0 [OASIS Standard]. OASIS Standard. <https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.pdf>

Kannwischer, M. J., Rijneveld, J., Schwabe, P., & Stoffelen, K. (2019). pqm4: Testing and Benchmarking NIST PQC on ARM Cortex-M4 (No. 2019/844). Cryptology ePrint Archive. <https://eprint.iacr.org/2019/844>