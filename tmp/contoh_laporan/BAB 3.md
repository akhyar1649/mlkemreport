# BAB 3

library.uns.ac.id                                                                             digilib.uns.ac.id




                                                    BAB III
                                       METODOLOGI PENELITIAN

                     Tahapan–tahapan yang akan dilakukan dalam penelitian ini ditunjukkan pada Gambar
             3.1 berikut:




                                    Gambar 3.1. Bagan Metodologi Penelitian


             3.1.   Perencanaan
                    Tahap    awal    penelitian   dimulai   dengan    proses    perancanaan    yaitu
             mengumpulkan dan mengkaji informasi yang relevan dari berbagai sumber.
             Tujuannya untuk memperoleh pendekatan yang telah digunakan dalam penelitian
             sebelumnya, sehingga dapat merancang kerangka penelitian yang sistematis dan
             terarah.
                    Penelitian ini dirancang dalam tiga skenario, yaitu skenario pertama kombinasi
             CoAP dan MQTT menggunakan fog computing. CoAP sebagai komunikasi antara fog
             node dengan mikrokontroller dan MQTT untuk komunikasi antara fog node dengan
             pengguna yang menggunakan cloud broker. Skenario kedua kombinasi CoAP dan
             HTTP menggunakan fog computing. CoAP sebagai komunikasi antara fog node
             dengan mikrokontroler dan HTTP untuk komunikasi antara fog node dengan pengguna



                                                       16
library.uns.ac.id                                                                      digilib.uns.ac.id
                                                                                             17




             yang menggunakan MQTT cloud broker. Skenario ketiga menggunakan cloud
             computing dengan MQTT sebagai komunikasi antara server dengan mikrokontroler
             dan pengguna yang menggunakan MQTT cloud broker.


             3.2.   Perancangan Prototipe
                    Tahap perancangan prototipe bertujuan untuk membangun prototipe IoT untuk
             melakukan pemantauan dan pengendalian dalam pertanian greenhouse. Dalam tahap
             ini meliputi perancangan arsitektur perangkat dan pemilihan spesifikasi perangkat
             keras. Spesifikasi perangkat yang digunakan dalam pengembangan prototipe IoT ini
             ditampilkan pada Tabel 3.1 berikut.


                                           Tabel 3.1. Spesifikasi Peralatan

                                                     Hardware
                      Mikrokontroler       ESP 32
                      Sensor               DHT 22, LDR, Moisture sensor
                      Aktuator             Lampu LED, Pompa Air, kipas
                                                     Fog Node
                      Device               Acer Nitro AN515-58
                      Sistem Operasi       Windows 11 25H2
                      Processor            12th Gen Intel(R) Core(TM) i5-12500H (3.10 GHz)
                      Memori               16 GB


                    Perangkat keras yang dibutuhkan sistem diseleksi untuk memastikan
             fungsionalitas dan integrasi yang optimal. Mikrokontroler ESP32 digunakan sebagai
             pusat kendali dan pemantauan karena kemampuannya untuk terhubung dengan
             berbagai sensor dan aktuator secara efisien. Beberapa sensor yang digunakan dalam
             perangkat ini antara lain:
               1. DHT 22 untuk memantau suhu dan kelembapan udara.
               2. LDR untuk memantau intensitas cahaya dalam greenhouse.
               3. Moisture sensor untuk memantau kelembapan tanah.
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                18




             Aktuator yang digunakan dalam perangkat ini mencakup:
               1. Lampu LED, sebagai sumber pencahayaan tambahan saat intensitas cahaya
                    alami tidak mencukupi.
               2. Pompa air, untuk menyuplai air.
               3. Kipas angin, yang diaktifkan untuk mengatur sirkulasi udara serta menurunkan
                    suhu.
                     Perancangan skenario pertama, CoAP digunakan untuk komunikasi antara
             mikrokontroler dan fog node. Fog node berfungsi sebagai pusat pengolahan data. Fog
             node memiliki server SQL untuk menyimpan data dan server backend untuk
             mendukung aplikasi web. Seluruh komunikasi antara mikrokontroler dan fog node,
             baik pengiriman data sensor maupun instruksi aktuator melalaui protokol CoAP. Data
             sensor diterima fog node diproses terlebih dahulu untuk menentukan aksi aktuator.
             Kemudian dikirimkan kembali ke mikrokontroler. Selain itu, data sensor akan
             diteruskan ke aplikasi web melalui protokol MQTT dengan dukungan MQTT cloud
             broker HiveMQ dan disimpan ke dalam database SQL. Instruksi pengendalian dari
             pengguna pada aplikasi web dikirimkan kembali ke fog node melalui broker MQTT,
             lalu diteruskan ke mikrokontroler menggunakan CoAP. Skenario pertama
             menggunakan arsitektur fog computing yaitu mikrokontroler dan fog node, berada
             dalam satu jaringan lokal. Diagram arsitektur dapat dilihat pada Gambar 3.2.
                     Penelitian ini merancang skenario kedua, sistem menggunakan protokol
             MQTT sebagai pengganti CoAP pada sekenario pertama untuk seluruh komunikasi
             antara mikrokontroler dan server. Data sensor diterima fog node diproses terlebih
             dahulu untuk menentukan aksi aktuator. Kemudian dikirimkan kembali ke
             mikrokontroler. Selain itu, data sensor akan diteruskan ke aplikasi web melalui
             protokol MQTT dengan dukungan cloud broker MQTT HiveMQ dan disimpan ke
             dalam database SQL. Instruksi pengendalian aktuator yang diubah melalui aplikasi
             dikirimkan ke server melalui MQTT cloud broker. Berbeda dengan pendekatan
             sebelumnya, pada rancangan ini menggunakan arsitektur cloud computing yang
             dimana mirkrokontroler, server, dan website berada di jaringan yang berbeda. Diagram
             arsitektur dapat dilihat pada Gambar 3.3.
                     Skenario   ketiga   merancang       alternatif   sistem   komunikasi   dengan
             mengombinasikan protokol CoAP dan HTTP dalam arsitektur fog computing.
library.uns.ac.id                                                                      digilib.uns.ac.id
                                                                                            19




             Komunikasi antara mikrokontroler dan server menggunakan protokol CoAP,
             sedangkan aplikasi web berinteraksi dengan server melalui protokol HTTP. Server
             berperan mengolah data sensor dari mikrokontroler untuk menentukan aksi aktuator.
             Kemudian dikirimkan kembali ke mikrokontroler. Selain itu, server menyimpan data
             sensor dalam database SQL, serta menyediakan endpoint HTTP. Instruksi
             pengendalian dari pengguna dikirim melalui HTTP dan kemudian diteruskan server
             ke mikrokontroler menggunakan CoAP. Diagram arsitektur dapat dilihat pada Gambar
             3.4.
library.uns.ac.id                                                      digilib.uns.ac.id
                                                                                                         20




                    Gambar 3.2. Diagram Arsitektur Skenario Pertama (Fog Computing dengan CoAP + MQTT)
library.uns.ac.id                                                  digilib.uns.ac.id
                                                                                                  21




                    Gambar 3.3. Diagram Arsitektur Skenario Kedua (Cloud Computing dengan MQTT)
library.uns.ac.id                                                      digilib.uns.ac.id
                                                                                                         22




                    Gambar 3.4. Diagram arsitektur skenario ketiga ( Fog Computing dengan CoAP + HTTP)
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                23




                    Data yang diperoleh dari sensor dianalisis menggunakan algoritma fuzzy logic
             Mamdani yang dijalankan pada fog node atau server. Pemilihan metode fuzzy
             Mamdani didasarkan pada kemampuannya dalam merepresentasikan pengetahuan
             manusia ke dalam bentuk aturan linguistik IF–THEN serta kesesuaiannya untuk sistem
             pengendalian dengan keluaran diskrit berupa kondisi ON dan OFF pada aktuator. Hasil
             dari analisis digunakan untuk menjalankan aksi otomatis aktuator. Aturan-aturan
             dalam fuzzy logic disusun berdasarkan referensi dari Irwanto et al (2024) dengan
             penyesesuian pada nilai presentase pada referensi diubah sesuai dengan output sensor
             yang digunakan dalam penelitian ini. Aturan dari fuzzy logic yang digunakan dapat
             dilihat pada Tabel 3.2 sampai dengan Tabel 3.6 berikut ini:
               a. Himpunan Fuzzy
                    •   Kelembapan Tanah
                          Tabel 3.2. Himpunan Fuzzy untuk Kelembapan Tanah (Irwanto et al.,
                                                       2024)

                                      Nilai ADC       Kategori Fuzzy
                                      0-1365          Rendah (Basah)
                                      1200 -2800      Sedang (Lembab)
                                      2600-4095       Tinggi (Kering)


                    •   Suhu Udara
                        Tabel 3.3. Himpunan Fuzzy untuk Suhu Udara (Irwanto et al., 2024)

                                     Nilai (℃)       Kategori Fuzzy
                                     0-20            Dingin
                                     15-30           Sedang
                                     25-45           Panas
library.uns.ac.id                                                                             digilib.uns.ac.id
                                                                                                    24




                    •   Kelembapan Udara
                           Tabel 3.4. Himpunan Fuzzy untuk Kelembapan Udara (Irwanto et al.,
                                                     2024)

                                       Nilai %            Kategori Fuzzy
                                       0-50               Rendah
                                       40-80              Sedang
                                       70-100             Tinggi


                    •   Intensitas Cahaya
                        Tabel 3.5. Himpunan Fuzzy untuk Intensitas Cahaya (Irwanto et al., 2024)

                                       Nilai ADC          Kategori Fuzzy
                                       0-1365             Rendah (Terang)
                                       1200-2800          Redup
                                       2600-4095          Tinggi (Gelap)


               b. Tabel Aturan Fuzzy
                           Aturan yang digunakan merupakan representasi dari kondisi utama
                    rendah, sedang dan tinggi yang paling memengaruhi respon aktuator. Aturan
                    fuzzy yang telah disusun untuk mengatur kerja penyiram, kipas, dan lampu
                    berdasarkan kombinasi kelembapan tanah, suhu udara, kelembapan udara dan
                    intensitas cahaya. Ketika tanah dalam kering dan lembab penyiram akan aktif,
                    kipas akan menyala jika suhu panas, dan lampu akan menyala saat kondisi gelap
                    atau redup. Aturan ini disusun dengan pendekatan yang juga diterapkan oleh
                    Irwanto et al. (2024). Jumlah aturan fuzzy terdapat 81 aturan contoh aturan dapat
                    dilihat pada Tabel 3.6 dan 81 aturan dapat dilihat pada Lampiran 1.


                                                 Tabel 3.6. Aturan Fuzzy

                     Kelembapan       Suhu       Kelembapan Intensitas
                                                                                   Aktuator
                       Tanah         Udara         Udara            Cahaya
                     Rendah          Dingin      Rendah            Rendah     Penyiram OFF, Kipas
                     (Basah)                                       (Terang)   OFF, Lampu ON
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                25




                    Kelembapan       Suhu    Kelembapan Intensitas
                                                                                 Aktuator
                      Tanah         Udara        Udara        Cahaya
                    Rendah          Sedang Rendah            Rendah       Penyiram OFF, Kipas
                    (Basah)                                  (Terang)     OFF, Lampu ON

                    Rendah          Panas    Rendah          Rendah       Penyiram OFF, Kipas
                    (Basah)                                  (Terang)     ON, Lampu ON

                    Sedang          Dingin   Sedang          Rendah       Penyiram ON, Kipas
                    (Lembab)                                 (Terang)     OFF, Lampu OFF


                    Sedang          Dingin   Tinggi          Redup        Penyiram ON, Kipas
                    (Lembab)                                              OFF, Lampu OFF

                    Tinggi          Dingin   Rendah          Redup        Penyiram ON, Kipas
                    (Kering)                                              OFF, Lampu ON

                    Tinggi          Dingin   Rendah          Tinggi       Penyiram ON, Kipas
                    (Kering)                                 (Gelap)      OFF, Lampu ON



             3.3.   Pengujian Sistem
             3.3.1. Pengujian Sensor dan Aktuator
                    Pengujian sensor dan aktuator bertujuan mengevaluasi kinerja sensor dalam
             mendeteksi berbagai parameter lingkungan, meliputi suhu udara, kelembapan udara,
             intensitas cahaya, dan kelembapan tanah. Pengujian ini difokuskan pada validasi alur
             pemrosesan data oleh fuzzy logic. Data yang dikirimkan dari sensor akan diproses oleh
             fuzzy logic untuk menentukan kondisi lingkungan dan menghasilkan keputusan
             terhadap aktuator, seperti menyalakan lampu, mengaktifkan pompa air, atau
             menyalakan kipas angin. Dengan demikian, pengujian ini memastikan bahwa integrasi
             antara deteksi sensorik, pemrosesan berbasis fuzzy logic, dan reaksi aktuator berjalan
             secara sinkron dan sesuai dengan skenario yang telah dirancang.
library.uns.ac.id                                                                               digilib.uns.ac.id
                                                                                                    26




                                    Tabel 3.7. Tahap Pengujian Sensor dan Aktuator

                    1.   Pompa   Deskripsi        Uji pengendalian secara otomatis terhadap
                         Air     Pengujian        pengaturan kelembapan tanah
                                 Kondisi Awal     Pompa air mati dengan level kelembapan
                                                  tinggi
                                 Langkah          -   Mengaktifkan perangkat
                                 Pengujian        -   Mengubah level kelembapan tanah sesuai
                                                      dengan aturan yang sudah dibuat.
                                 Hasil       yang -   Pompa air dapat menyala jika kelembapan
                                 Diharapkan           tanah berada di level rendah
                                                  -   Pompa air tidak menyala jika kelembapan
                                                      tanah berada di level sedang dan tinggi
                    2.   Lampu   Deskripsi        Uji pengendalian secara otomatis terhadap
                         LED     Pengujian        pengaturan intensitas cahaya yang diatur oleh
                                                  sensor LDR
                                 Kondisi Awal     Lampu LED mati dengan intensitas cahaya di
                                                  level redup atau terang.
                                 Langkah          -   Mengaktifkan perangkat
                                 Pengujian        -   Mengubah level intensitas cahaya sesuai
                                                      dengan aturan yang sudah dibuat.
                                 Hasil       yang -   Lampu LED dapat menyala jika intensitas
                                 Diharapkan           cahaya berada di level gelap
                                                  -   Lampu LED tidak menyala jika intensitas
                                                      cahaya berada di level redup dan terang.
                    3.   Kipas   Deskripsi        -   Uji   pengendalian     secara     otomatis
                         Angin   Pengujian            terhadap     pengaturan        suhu   dan
                                                      kelembapan udara oleh sensor DHT 22
                                 Kondisi Awal     -   Kipas angin dalam kondisi mati dengan
                                                      suhu dan kelembapan udara di level
                                                      sedang
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                  27




                               Langkah           -   Mengaktifkan perangkat
                               Pengujian         -   Mengubah suhu dan kelembapan sesuai
                                                     dengan aturan yang sudah dibuat.
                               Hasil       yang -    Kipas angin dapat menyala jika suhu
                               Diharapkan            dalam   level   panas    apapun    kondisi
                                                     kelembapan udara.
                                                 -   Kipas angin tidak menyala jika suhu
                                                     dalam level sedang dan dingin apapun
                                                     kondisi kelembapan udara.


             3.3.2. Pengujian Performa
                    Pengujian performa dilakukan mengevaluasi kemampuan sistem dalam
             menjalankan fungsi pemantauan dan pengendalian, terutama dalam aspek komunikasi
             antara mikrokontroler, fog node, dan aplikasi web. Evaluasi difokuskan pada tiga
             parameter, yaitu latensi, packet loss, dan jitter, dengan membandingkan kinerja pada
             tiga skenario yaitu kombinasi CoAP dan MQTT pada arsitektur fog computing,
             kombinasi CoAP dan HTTP pada arsitektur fog computing dan MQTT pada arsitektur
             cloud computing. Proses pencatatan dilakukan dengan menambahkan timestamp pada
             setiap paket data sensor yang dikirim dari mikrokontroler. Fog node atau server
             mencatat waktu selisih penerimaan paket menggunakan logger Node.js. Selisih antara
             waktu pengiriman dan penerimaan akan digunakan untuk menghitung latensi (Alamin
             et al., 2025). Packet loss dihitung dari jumlah paket yang hilang dibandingkan total
             paket yang dikirim (Alamin et al., 2025). Jitter dihitung berdasarkan variasi nilai
             latensi antar paket secara berurutan untuk mengetahui kestabilan waktu pengiriman
             paket (Alamin et al., 2025). Perhitungan nilai latensi menggunakan Persamaan 3.1
             (Alamin et al., 2025), packet loss menggunakan Persamaan 3.2 (Alamin et al., 2025),
             dan jitter menggunakan Persamaan 3.3 (Alamin et al., 2025).
                                    𝐿𝑎𝑡𝑒𝑛𝑠𝑖 = 𝑊𝑎𝑘𝑡𝑢 𝑇𝑖𝑏𝑎 − 𝑊𝑎𝑘𝑡𝑢 𝑘𝑖𝑟𝑖𝑚                       (3.1)
                                               𝐽𝑢𝑚𝑙𝑎ℎ 𝑝𝑎𝑘𝑒𝑡 𝑦𝑎𝑛𝑔 ℎ𝑖𝑙𝑎𝑛𝑔
                               𝑃𝑎𝑐𝑘𝑒𝑡 𝑙𝑜𝑠 =                             𝑥100%                (3.2)
                                               𝑇𝑜𝑡𝑎𝑙 𝑝𝑎𝑘𝑒𝑡 𝑦𝑎𝑛𝑔 𝑑𝑖𝑘𝑖𝑟𝑖𝑚
                                           𝐽𝑖𝑡𝑡𝑒𝑟 = 𝑉𝑎𝑟𝑖𝑎𝑛𝑠𝑖 𝐷𝑎𝑟𝑖 𝐷𝑒𝑙𝑎𝑦                      (3.3)
library.uns.ac.id                                                                         digilib.uns.ac.id
                                                                                              28




                    Pada penelitian ini, pengujian dilakukan dengan interval pengiriman data dari
             mikrokontroler ke fog node setiap 15 detik dengan waktu pengujian 15 menit untuk
             setiap skenario pengujian.


             3.4.   Analisa Perbandingan
                    Tahap ini dilakukan analisis terhadap hasil pengujian performa sistem untuk
             menilai kesesuaian sistem pemantauan dan pengendalian berbasis IoT yang
             menggunakan arsitektur fog computing dan CoAP memenuhi tujuan penelitian.
             Analisis mencakup kinerja sistem dari aspek latensi, packet loss, dan jitter yang
             dibandingkan dengan perangkat yang menggunakan arsitektur cloud computing dan
             MQTT. Serta memastikan bahwa sensor mampu membaca parameter lingkungan
             dengan baik dan aktuator merespons perintah sesuai dengan ketentuan yang telah
             ditetapkan.
