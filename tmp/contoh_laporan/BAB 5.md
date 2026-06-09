# BAB 5

library.uns.ac.id                                                                             digilib.uns.ac.id




                                                     BAB V
                                                  PENUTUP


             5.1.   Kesimpulan
                    Penelitian ini arsitektur fog computing berhasil diimplementasikan dengan
             menempatkan fog node sebagai pemrosesan lokal yang menerima data sensor melalui
             protokol CoAP. Berdasarkan implementasi dan hasil pengujian yang telah dilakukan,
             kedua skenario yang menggunakan protokol CoAP pada arsitektur fog computing
             menunjukkan perbedaan hasil kinerja. Skenario pertama memberikan rata rata latensi
             3419,42 ms dan jitter 2329,26 ms, sedangkan skenario ketiga memberikan rata-rata
             lantensi 1038,88 ms dan jitter 75,09 ms. Perbedaan ini disebabkan proses pengiriman
             data pada skenario pertama menggunakan cloud broker MQTT, sementara skenario
             ketiga data langsung dikirim ke pengguna. Skenario pertama dan ketiga memiliki rata-
             rata latensi dan dan jitter yang rendah jika dibandingkan skenario kedua yang memiliki
             rata-rata 2927,567 ms dan jitter 1121,402 ms. Ketiga skenario memiliki packet loss
             0% artinya tidak ada data yang hilang saat pengiriman. Selain itu, integrasi fuzzy logic
             dalam sistem mampu mengotomasi pengendalian aktuator sesuai kondisi lingkungan.
             Kombinasi fog computing, CoAP, dan fuzzy logic memberikan sistem IoT yang
             responsif dan stabil, serta dapat beroperasi secara lokal tanpa ketergantungan penuh
             dengan jaringan internet.
             5.2.   Saran
                    Penelitian ini masih memiliki keterbatasan pada jumlah node yang digunakan
             masih terbatas dan belum menerapkan keamanan data yang baik. Saran pengembangan
             untuk penelititan selanjutnya, sistem dapat dikembangkan dengan menambahkan fitur
             keamanan data antar node sehingga proses pengiriman data lebih terjaga. Selain itu
             sistem dapat diimplementasikan pada jumlah node yang lebih besar untuk menguji
             performa dan skalabilitas. Pengembangan tersebut diharapkan supaya sistem mampu
             beroperasi lebih optimal, aman, dan adaptif sesuai kebutuhan.




                                                       40
