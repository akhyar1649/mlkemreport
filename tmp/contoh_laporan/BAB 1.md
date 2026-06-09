# BAB 1

library.uns.ac.id                                                                          digilib.uns.ac.id




                                                      BAB I
                                               PENDAHULUAN


             1.1.   Latar Belakang
                    Pemanfaatan Internet of Things (IoT) memungkinkan perangkat fisik seperti
             sensor dan aktuator dapat saling terhubung melalui jaringan internet. Dalam konteks
             pertanian greenhouse, teknologi IoT telah diterapkan dalam berbagai aplikasi, antara
             lain pemantauan kelembapan tanah, suhu, intensitas cahaya, serta pengendalian sistem
             irigasi dan pemberian nutrisi tanaman secara otomatis (Banjardana et al., 2024).
             Namun penerapan sistem IoT pada komputasi cloud sering mengalami berbagai
             kendala, seperti latensi yang tinggi, keterbatasan bandwidth karena bergantung pada
             kestabilan jaringan internet (Andriulo et al., 2024).
                    Penelitian sebelumnya mengenai sistem pertanian berbasis IoT untuk
             pemantauan dan pengendalian, terdapat beberapa pendekatan dalam pengiriman dan
             pengolahan data sensor. Pada penelitian terdapat pendekatan yang menghubungkan
             perangkat mikrokontroler secara langsung dengan aplikasi Blynk tanpa melalui
             pemrosesan data, sehingga data sensor yang dikirim dapat diterima Blynk secara real-
             time (Rahim et al., 2023). Penelitian lainnya menggunakan protokol Message Queuing
             Telemetry Transport (MQTT) dalam proses transmisi data dari mikrokontroler ke
             pengguna (Choosumrong et al., 2023; Dwi Wardihani et al., 2024; Eka Apriyani et al.,
             2025; Eso et al., 2024). Penelitian lainnya melakukan pengiriman data sensor dari
             mikrokontroler ke server untuk diproses sebelum pengambilan keputusan, namun
             karena perangkat tidak dalam satu jaringan, pendekatan ini menyebabkan latensi tinggi
             dan kehilangan data (Dwi Wardihani et al., 2024). Untuk mengatasi latensi tinggi,
             beberapa penelitian mengkaji penerapan fog computing sebagai lapisan pemrosesan
             menengah antara perangkat IoT dan cloud (Suharsono and Nurwarsito, 2022; Zainudin
             et al., 2021). Terdapat penelitian lain yang mengimplementasikan protokol
             Constrained Application Protocol (CoAP) sebagai alternatif komunikasi yang lebih
             ringan dan cepat dari protokol MQTT (Safitri and Priambodo, 2023). Selain itu,
             terdapat penelitian yang menggunakan fuzzy logic untuk melakukan pengambilan
             keputusan aktuator secara otomatis (Eka Apriyani et al., 2025). Kelebihan penggunaan
             fuzzy logic pada penerepan IoT yaitu mampu meningkatkan efisiensi energi yang



                                                         1
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                 2




             digunakan, menyesuaikan tindakan secara otomatis berdasarkan kondisi lingkungan
             karena pengambilan keputusan fleksibel dan menyerupai logika manusia (Eka
             Apriyani et al., 2025). Salah satu algoritma fuzzy yang digunakan yaitu fuzzy mamdani
             yang memiliki kelebihan dalam menangani data yang tidak pasti dari sensor
             lingkungan (Andrianto et al., 2024)
                    Penelitian terkait telah dilakukan penelitian dalam sistem IoT untuk
             greenhouse, masih didominasi oleh penggunaan arsitektur cloud dengan protokol
             MQTT. Implementasi ini masih mengalami masalah dalam hal latensi yang tinggi dan
             kehilangan data saat proses pengiriman data ke server (Dwi Wardihani et al., 2024).
             Mengatasi hal tersebut, implementasi fog computing dapat mempercepat proses
             pengambilan keputusan dengan melakukan pemrosesan awal di node lokal sebelum
             data dikirim ke cloud (Zainudin et al., 2021). Penggunaan protokol CoAP memiliki
             keuntungan dalam kecilnya kehilangan data, komunikasi menjadi lebih ringan
             dibandingkan dengan MQTT, dan lebih sedikit menggunakan sumber daya
             dibandingkan MQTT (Seoane et al., 2021). Oleh karena itu, terdapat celah penelitian
             yang belum dieksplorasi, yaitu pemanfaatan arsitektur fog computing yang
             diintegrasikan dengan protokol CoAP untuk mengurangi latensi serta meminimalkan
             kehilangan data pada sistem IoT.
                    Penelitian sebelumnya menggunakan cloud computing dan protokol MQTT
             menghasilkan latensi tinggi dan kehilangan paket data (Dwi Wardihani et al., 2024).
             Oleh karena itu, diusulkan pengembangan sistem pertanian berbasis IoT dalam
             pemantauan dan pengendalian dengan mengimplementasikan arsitektur fog computing
             serta menggunakan CoAP sebagai transmisi data. Sistem ini dirancang untuk
             melakukan pemantauan lingkungan dengan memanfaatkan sensor seperti suhu udara,
             kelembapan udara, intensitas cahaya, dan kelembapan tanah. Data sensor ini akan
             dilakukan pengolahan juga untuk menentukan aksi dari aktuator seperti penyiraman
             tanaman, lampu, dan kipas angin. Aksi aktuator menggunakan algoritma fuzzy logic
             setiap aksi yang dihasilkan sesuai dengan kondisi lingkungan. Kontribusi dalam
             penelitian ini terletak dalam penerapan fog computing dengan CoAP dalam transmisi
             data dan penggunaan algoritma fuzzy logic mamdani dalam menentukan aksi aktuator,
             serta mengevaluasi protokol CoAP berdasarkan tiga parameter yaitu latensi, jitter, dan
             packet loss, serta dibandingkan dengan penerapan cloud computing yang
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                  3




             menggunakan protokol MQTT.
             1.2. Rumusan Masalah
             Rumusan masalah dari penelitian ini yaitu:
                    a.   Bagaimana mengimplementasikan arsitektur fog computing dan protokol
                         CoAP untuk optimalisasi sistem pemantauan dan pengendalian IoT pada
                         pertanian?
                    b.   Bagaimana menerapakan fuzzy logic untuk mengontrol aksi aktuator?
                    c.   Bagaimana kinerja CoAP dibandingkan MQTT dalam hal latensi, packet
                         loss, dan jitter pada sistem IoT pertanian?
             1.3. Batasan Masalah
             Batasan masalah yang digunakan dalam penelitian ini yaitu:
                    a.   Penelitian ini hanya menggunakan sensor dan aktuator yang tersedia secara
                         komersil tanpa dilakukan modifikasi atau kalibrasi lebih lanjut.
                    b.   Penelitian ini terbatas pada pengembangan prototipe yang diuji dalam skala
                         sederhana dengan mensimulasikan kondisi lingkungan melalui pengujian
                         pada media tanah, suhu udara, dan cahaya untuk melihat respons sistem
                         terhadap perubahan parameter tersebut.
             1.4. Tujuan Penelitian
             Penelitian ini memiliki tujuan berikut:
                    a.   Mengimplementasikan arsitektur fog computing dan protokol komunikasi
                         CoAP dalam transmisi data pada sistem pemantauan dan pengendalian
                         lingkungan pertanian untuk mempercepat pengambilan keputusan dan
                         mengurangi ketergantungan pada cloud.
                    b.   Menerapkan fuzzy logic untuk mengontrol aksi aktuator secara otomatis.
                    c.   Menguji performa sistem menggunakan metrik QoS seperti latensi, packet
                         loss, dan jitter, serta membandingkan performa antara protokol CoAP yang
                         menggunakan arsitektur fog computing dan MQTT yang menggunakan
                         arsitektur cloud computing.
             1.5. Manfaat Penelitian
             Manfaat dari penelitian ini yaitu:
                    a.   Menawarkan solusi alternatif terhadap keterbatasan sistem berbasis cloud
                         dengan memanfaatkan pemrosesan lokal.
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                  4




                    b.     Mendukung otomatisasi dan optimalisasi pengelolaan lingkungan dalam
                           greenhouse, yang berdampak pada peningkatan produktivitas pertanian.
             1.6. Sistematika Penulisan
                         Bab 1 pendahuluan membahas tentang latar belakang, rumusan masalah,
             batasan masalah, manfaat, dan sistematika penulisan. Bab 2 terdapat tinjauan pustaka
             yang mengulas dasar teori yang digunakan sebagai landasan dasar dalam penelitian.
             Bab 3 metodologi penelitian membahas metode yang digunakan dalam penelitian
             meliputi: perencaan, perancangan prototipe, pengujian sistem, dan analisa
             perbandingan. Bab 4 berisi hasil dan pembahasan. Bab 5 penutup terdapat kesimpulan
             dan saran penelitian.
