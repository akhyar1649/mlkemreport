# BAB 2

library.uns.ac.id                                                                           digilib.uns.ac.id




                                                    BAB II
                                           TINJAUAN PUSTAKA


             2.1.   Dasar Teori
             2.1.1. Internet of Things (IoT)
                    IoT merupakan      perangkat dan sistem pintar yang dapat terhubung serta
             menggunakan data yang dikumpulkan oleh sensor dan aktuator yang tertanam dalam
             mesin dan objek fisik lainnya (Riskiyah et al., 2023). Tujuan dari perangkat IoT yaitu
             meningkatkan manfaat dari konektivitas internet yang tersambung secara terus
             menerus (Banjardana et al., 2024). Perangkat IoT terdiri dari sensor, aktuator, dan
             berbagai alat elektronik yang memiliki fungsi untuk melakukan pemantauan dan
             pengendalian. Adanya IoT dapat memungkinkan interaksi antara perangkat dan
             pengguna, serta dengan perangkat lainnya. Agar tercipta sebuah ekosistem yang cerdas
             dan terintegrasi.

             2.1.2. Fog Computing
                    Fog computing adalah model komputasi yang dirancang untuk mendekatkan
             pemrosesan data ke sumbernya (perangkat IoT) dengan tujuan mengurangi latensi dan
             meningkatkan efisiensi (Das and Inuwa, 2023). Arsitektur fog computing terdiri dari
             beberapa lapisan yang saling berinteraksi. Fog computing terdiri dari tiga lapisan
             utama yaitu cloud, fog, dan pengguna, dengan server fog yang ditempatkan di sekitar
             bangunan sebagai bagian dari lapisan fog (Yoga et al., 2024).
                    Koneksi wireless (Wi-Fi, 5G) dan kabel, memegang peranan penting dalam
             menghubungkan end device, fog nodes, dan cloud untuk menjamin transmisi data yang
             cepat, andal, dan aman (misalnya melalui enkripsi end-to-end) (Andriulo et al., 2024).
             Node fog didistribusikan ke fog node untuk menjalankan pemrosesan data, komputasi,
             dan komunikasi, sehingga secara efektif mengurangi beban pada pusat data cloud
             (Yoga et al., 2024). Fog node dapat berupa gateway, router, atau server mini yang
             mengambil alih tugas komputasi, penyimpanan sementara, dan agregasi data dari
             sensor, sehingga mengurangi volume data yang dikirim ke pusat data cloud dan
             menurunkan beban bandwidth serta latensi (Dogea et al., 2023). Penerapan fog
             computing, aplikasi IoT yang membutuhkan pemrosesan data dengan cepat dapat
             diciptakan, seperti pada sistem kendaraan otonom, pengawasan keamanan, dan


                                                       5
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                 6




             otomatisasi industri. Selain itu, fog computing juga meningkatkan aspek keamanan dan
             privasi data dengan memproses informasi sensitif secara lokal, sehingga
             meminimalkan risiko kebocoran data selama proses transmisi.

             2.1.3. Constrained Application Protocol (CoAP)
                    CoAP merupakan protokol pertukuran pesan secara sinkron yang berbasis
             request dan respons. CoAP dijalankan diatas UDP yang tidak menjamin keandalan
             transmisi data (Seoane et al., 2021). CoAP dirancang secara efisien dan ringan agar
             sesuai digunakan dalam perangkat IoT dengan keterbatasan sumber daya seperti
             memori, daya, dan kapasitas pemrosesan. Selain itu CoAP juga mendukung arsitektur
             RESTful yang memungkinkan integrasi dengan sistem berbasis web serta kompatibel
             dengan model komunikasi machine-to-machine (M2M).
                    CoAP memiliki empat jenis pesan dalam mekanisme komunikasi (Seoane et
             al., 2021). Jenis pesan yang pertama, Confirmable (CON) yaitu jenis pesan yang
             memerlukan balasan berupa ACK atau RST dan akan dikirim ulang jika tidak ada
             respons. Jenis pesan kedua,      Non-confirmable (NON) yaitu pesan yang tidak
             memerlukan balasan dan tidak menjamin kehandalan pengiriman. Jenis pesan ketiga,
             Acknowledgement (ACK) yaitu pesan yang digunakan untuk mengkonfirmasi
             penerimaan pesan CON dan dapat membawa respons (piggybacked). Jenis pesan yang
             terakhir, Reset (RST) yaitu pesan yang menunjukkan bahwa pesan diterima tetapi tidak
             bisa diproses, biasanya karena kehilangan status penerima.
             2.1.4. Message Queuing Telemetry Transport (MQTT)
                    Pada protokol ini menggunakan model publish subscribe berbasis pada topik,
             data dikelompokkan berdasarkan topiknya (Seoane et al., 2021). Keunggulan protokol
             MQTT adalah sederhana, terbuka, ringan dan efisien dalam penggunaan bandwidth
             yang tinggi, sehingga MQTT menjadi pilihan yang ideal untuk melakukan komunikasi
             dengan perangkat yang memiliki sumber daya terbatas (Seoane et al., 2021). Selain
             itu, protokol ini mendukung mekanisme Quality of Service (QoS) yang diatur dalam
             spesifikasi OASIS. Terdapat tiga level keandalan QoS 0 (at most once), QoS 1 (at least
             once), dan QoS 2 (exactly once) sehingga pengembang dapat menyesuaikan jaminan
             pengiriman pesan sesuai dengan kebutuhan aplikasi (Mohammad El-Basioni, 2024).
library.uns.ac.id                                                                         digilib.uns.ac.id
                                                                                               7




             2.1.5. Mikrokontroler ESP32
                    ESP32 merupakan mikrokontroler yang terintegrasi dengan tujuan kebutuhan
             IoT, serta menggabungkan konektivitas Wi-Fi dan Bluetooth (Abdul Kholik et al.,
             2023). ESP32 menawarkan kombinasi prosesor dual core, konektivitas nirkabel,
             banyak GPIO, serta konsumsi daya yang rendah (Hercog et al., 2023). Pengguna
             memungkinkan membangun sistem tertanam yang kompleks tanpa memerlukan
             banyak komponen tambahan. ESP32 dapat difungsikan sebagai alternatif dari
             Arduino, dengan keunggulan utamanya yaitu kemampuan terintegrasi untuk
             terkoneksi langsung ke jaringan Wi-Fi (Nizam et al., 2022).

             2.1.6. Aktuator
                    Perangkat yang memiliki fungsi mengubah energi listrik menjadi aksi yang
             nyata yaitu aktuator (Arshi and Mondal, 2023). Aktuator memiliki peran dalam
             mengendalikan perangkat dan sistem, misalnya membuka atau menutup katup,
             menggerakkan motor, atau menghidupkan dan mematikan perangkat (Choudhary,
             2024). Sinyal yang dihasilkan dari mikrokontroler atau sistem biasanya dihasilkan
             dari data sensor.
             2.1.7. Fuzzy Logic
                    Pengertian fuzzy logic control merupakan sistem kendali yang dikembangkan
             dengan pendekatan lingustik yang memiliki keterkaitan erat pada bidang kecerdasan
             buatan. Sistem ini lebih unggul dibandingkan kontrol tradisional karena mampu
             meniru cara manusia berpikir, tidak memerlukan model matematis yang presisi, serta
             lebih sesuai diterapkan pada proses industri yang kompleks (Khalid and Omatu, 1993).
                    Sistem kontrol yang berbasis fuzzy pada umumnya terdiri dari empat
             komponen utama (Saatchi, 2024). Komponen pertama adalah tahap fuzzifikasi yaitu
             proses mengubah input berupa nilai crisp menjadi derajat keanggotaan pada himpunan
             fuzzy. Komponen kedua yaitu rule base yang berisikan kumpulan aturan dalam bentuk
             IF-THEN yang linguistik, seperti contohnya "IF (temperature IS cold) THEN (heater
             IS high)". Komponen ketiga yaitu inference engine yang memiliki fungsi untuk
             melakukan evaluasi aturan-aturan tersebut, biasanya menggunakan metode max–min,
             untuk menghasilkan himpunan fuzzy output. Komponen terakhir yaitu tahap
             defuzzifikasi yang bertugas mengubah output fuzzy menjadi nilai crisp, umumnya
             dengan metode centroid.
library.uns.ac.id                                                                             digilib.uns.ac.id
                                                                                                       8




                     Fuzzifikasi adalah proses perubahan nilai crips input menjadi derajat
             keanggotaan dalam himpunan fuzzy. Kategori linguistik direpresentasikan dengan
             fungsi keanggotaan, menunjukkan hubungan antara nilai input dan tingkat
             keanggotaannya dalam himpunan fuzzy. Jenis dari fungsi keanggotaan sebagai berikut
             (Imran et al., 2021):
               a. Triangular Membership Function
                          Bentuk keanggotaan berbentuk segitiga memiliki tiga parameter utama,
                    yaitu a, b, dan c. Membentuk sisi kiri, puncak dan sisi kanan segitiga. Persamaan
                    dapat dituliskan pada Persamaan 2.1 (Imran et al., 2021):
                                                        0,       𝑥≤𝑎
                                                      𝑥−𝑎
                                                                 𝑎≤𝑥≤𝑏                         (2.1)
                                      𝑓(𝑥; 𝑎, 𝑏, 𝑐) = 𝑏𝑐 −
                                                         −𝑥
                                                           𝑎
                                                                 𝑏≤𝑥≤𝑐
                                                       𝑐−𝑏
                                                     { 0         𝑥≥𝑐


               b. Trapezoidal Membership Function
                          Bentuk keanggotaan berbentuk trapesium, merupakan pengembangan dari
                    fungsi segitiga dengan area datar di bagian tengah. Persamaan dapat dituliskan
                    pada Persamaan 2.2 (Imran et al., 2021):
                                                         0,       𝑥≤𝑎
                                                        𝑥−𝑎
                                                                  𝑎≤𝑥≤𝑏
                                                        𝑏−𝑎                                    (2.2)
                                     𝑓(𝑥; 𝑎, 𝑏, 𝑐, 𝑑) = 1,        𝑏≤𝑥≤𝑐
                                                        𝑑−𝑥
                                                                   𝑐≤𝑥≤𝑑
                                                        𝑑−𝑏
                                                       { 0         𝑥≥𝑑


                       Defuzzification adalah proses mengubah nilai hasil fuzzy menjadi output
             crisp. Hasil dari proses defuzzification digunakan oleh perangkat keras. Terdapat
             beberapa teknik defuzzification diantaranya (Islam and Hossain, 2022):
               a. Centroid Method
                          Cara kerja centroid method mengurangi area menjadi wilayah yang lebih
                    kecil, dan operasi gabungan dilakukan untuk mendapatkan output akhir. Dapat
                    dituliskan pada Persamaan 2.3 (Islam and Hossain, 2022):
library.uns.ac.id                                                                             digilib.uns.ac.id
                                                                                                       9




                                                      ∑𝑛𝑖=1 𝑋𝑖 𝜇 (𝑋𝑖 )                         (2.3)
                                                   𝑋=
                                                       ∑𝑛𝑖=1 𝜇(𝑋𝑖 )
                    n : jumlah elemen dalam sempel
                    𝜇(𝑋𝑖 ) : derajat keanggotaan
                    𝑋𝑖 : nilai crisp dari variabel fuzzy
               b. Bisector Method
                          Cara kerja bisector method membagi luas menjadi dua bagian dengan luas
                    yang sesuai, dapat pertepatan atau tidak dengan garis pusat luas area tersebut.
               c. Mean of Maxima (MOM)
                          Cara kerja MOM yaitu memilih nilai crips yang memiliki derajat
                    keanggotaan paling tinggi, menghitung rata rata dari semua nilai tersebut. Dapat
                    dituliskan pada Persamaan 2.4 (Islam and Hossain, 2022):
                                                           ∑𝑋𝑖∈𝑀 𝑋𝑖                            (2.4)
                                                      𝑋=
                                                             |𝑀|


                    M : himpunan nilai dengan |𝑀| maksimum
                    |𝑀| : kardinalitas dari himpunan fuzzy M
library.uns.ac.id                                                                             digilib.uns.ac.id
                                                                                                                                                  10




           2.2.     Penelitian Terkait
           Penelitian terkait dapat dilihat pada Tabel 2.1.
                                                                    Tabel 2.1. Penelitian Terkait

            No      Artikel              Masalah              Tujuan                 Metode                  Data dan Hasil        Keterkaitan

            1       Sistem Pemantau      Kesulitan dalam      Menerapkan sistem      Menggunakan sensor      Sistem yang           Monitoring
                    Kondisi              menerapkan           pemantauan berbasis    suhu dan kelembapan     dikembangkan          secara real-time
                    Lingkungan           pertanian presisi    IOT, menyediakan       udara DHT22, sensor     mampu memberikan      dengan sensor
                    Pertanian            pada proses          pemantauan secara      suhu tanah DS18B20,     informasi seperti     soil moisture,
                    Tanaman Pangan       konvensional,        real time dari jarak   sensor soil moisture,   suhu udara,           DHT22, dan
                    dengan               yang                 jauh                   dan sensor intensitas   kelembapan, suhu      intensitas
                    NodeMCU              menyebabkan                                 cahaya BH1750.          tanah, kadar air      cahaya.
                    ESP8266 dan          produktivitas                               Serta NodeMCU           pada tanah, dan       Penggunaan
                    Raspberry Pi         hasil pertanian                             ESP8266, Raspberry      intensitas cahaya     gateway untuk
                    Berbasis IoT         tidak optimal.                              Pi 3B+. Menggunakan     secara real-time      memproses data
                    (Ambarwari et al.,                                               gateway memproses       setiap menit. Serta   sensor. Transfer
                    2021)                                                            data untuk diteruskan   keberhasilan 99.64%   data
                                                                                     ke Node-RED dan         dalam menyimpan       menggunakan
                                                                                     publish ke public       data di dalam basis   protokol MQTT.
                                                                                     MQTT broker             data.
            2       Real-time soil       Penggunaan air       Mengembangkan          Menggunakan             Penggunaan sistem     Menjelaskan
                    monitoring and       dalam irigasi        sistem irigasi dan     Nodemcu ESP8266         irigasi dan           monitoring suhu
                    irrigation system    pertanian yang       pemantauan tanah       sebagai controller,     pemantauan tanah      dan kelembapan
                    for taro yam         seringkali           dengan NodeMcu         sensor DHT11 untuk      dapat mengurangi      serta melakukan
                    cultivation          berlebihan yang      Esp8266 dan            mengukur suhu dan       konsumsi air hingga   kontrol water
                    (Rahim et al.,       menyebabkan air      aplikasi Blynk untuk   kelembapan sekitar      32,5% dibandingkan    pump pada
library.uns.ac.id                                                                         digilib.uns.ac.id
                                                                                                                                                11




            No      Artikel             Masalah             Tujuan               Metode                   Data dan Hasil         Keterkaitan

                    2023)               yang digunakan      memantau dan         dan sensor               dengan teknik          pertanian.
                                        dalam pertanian     mendeteksi tanah     kelembapan tanah.        penyiraman tanaman
                                        menjadi limbah.     pertanian secara     Serta menggunakan        konvensional.
                                                            real-time.           water pump.
                                                                                 Menggunakan aplikasi
                                                                                 Bylink untuk
                                                                                 melakukan kontrol
                                                                                 dan pemantauan.
            3       Monitoring and      Keterbatasan        Memantau dan         Menggunakan              Rata-rata data loss    Menjelaskan
                    Controlling of      parameter yang      mengendalikan        mikrokontroler           pemantauan             sistem
                    IoT-Based           dibutuhkan petani   berbagai parameter   Wemos D1, relay,         gateway-server         monitoring yang
                    Greenhouse          dalam               greenhouse, yang     sensor suhu dan          sebesar 10,6%, rata-   menggunakan
                    Parameters With     memonitoring dan    memungkinkan         kelembapan, serta        rata latensi           ESP 32,
                    the MQTT            mengelola           petani memantau      sensor cahaya,           pemantauan             parameter
                    Protocol (Dwi       pertanian           dan                  menggunakan              gateway-server         monitoring yaitu
                    Wardihani et al.,                       mengendalikannya     mikrokontroler ESP32     sebesar 1,9 s, dan     suhu dan
                    2024)                                   kapan saja dan di    SIM800L.                 rata-rata latensi      kelembapan
                                                            mana saja.           Menggunakan              pengendalian server-   serta
                                                                                 protokol MQTT            gateway sebesar        menggunakan
                                                                                 dalam transfer data      7,1s.                  sensor Cahaya.
                                                                                 dari gateway ke server                          Serta pengujian
                                                                                 dan server ke end                               dengan
                                                                                 device.                                         parameter
                                                                                                                                 packet loss dan
                                                                                                                                 latensi.
                                                                                                                                 Menggunakan
                                                                                                                                 protokol MQTT
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                                                                12




            No      Artikel              Masalah             Tujuan                Metode                  Data dan Hasil        Keterkaitan

                                                                                                                                 dalam transfer
                                                                                                                                 data.
            4       Improving            Kurangnya           Mengembangkan         Menggunakan sensor      Terjadi peningkatan   Controlling IoT
                    marigold             efektivitas dalam   sistem irigasi        SHT11,                  pertumbuhan pada      dengan
                    agriculture with     greenhouse          penyiraman otomatis   mikrokontroler          tanaman setelah       menggunakan
                    an IoT-driven        berkaitan dengan    berbasis IOT untuk    ESP32, dan komponen     menggunakan sistem    mikrokontroler
                    greenhouse           ketidakstabilan     mengontrol            relay. Serta            IoT.                  ESP32 dan
                    irrigation           kelembapan dan      kelembapam dan        menggunakan                                   komponen relay.
                    management           suhu.               suhu di greenhouse.   database SQL untuk                            Serta
                    control system                                                 menyimpan data.                               penggunaan
                    (Bunpalwong et                                                                                               SQL untuk
                    al., 2023)                                                                                                   menyimpan data
                                                                                                                                 sensor.
            5       Smart Plant          Pemeliharaan        Mengembangkan         Menggunakan             Terjadi peningkatan   Melakukan
                    Watering and         dalam pertanian     sistem penyiraman     NodeMCU ESP8266,        pertumbuhan           monitoring
                    Lighting System to   untuk               dan pencahayaan       Arduino R3, sensor      tanaman sebesar 3%    secara real-time
                    Enhance Plant        mengoptimalkan      otomatis berbasis     suhu dan kelembapan     saat menggunakan      Mengunakan
                    Growth Using         pertumbuhan dan     IoT untuk             AM2302, Soil            sistem IoT.           Moisture
                    Internet of Things   perkembangan        meningkatkan          Moisture Sensor,                              Sensor, sensor
                    (Setiawan et al.,    tumbuhan yang       pertumbuhan           sensor cahaya LDR                             LDR.
                    2023)                cukup sulit.        tanaman.              photoresistor,                                Melakukan
                                                                                   Ultraviolet LED, LCD                          controlling dari
                                                                                   12C 16x2, dan                                 jarak jauh
                                                                                   komponen relay.                               menggunakan
                                                                                   Menggunakan aplikasi                          komponen relay.
                                                                                   Blynk untuk
                                                                                   melakukan
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                                                                     13




            No      Artikel             Masalah            Tujuan                  Metode                  Data dan Hasil         Keterkaitan

                                                                                   pemantauan dan
                                                                                   pengendalian jarak
                                                                                   jauh.
            6       Sistem              Perubahan cuaca    Mengembangkan           Menggunakan ESP32,      Sistem mampu           Melakukan
                    Monitoring          saat ini tidak     sistem hijau pintar     protokol MQTT,          memantau secara        monitoring
                    Budidaya Melon      dapat diprediksi   untuk memantau dan      node-red, sensor LDR,   real-time berbagai     dengan
                    Melalui             yang               mengendalikan           sensor soil moisture,   parameter penting      menggunakan
                    Greenhouse          menyebabkan        perawatan tanaman       Serta menggunakan       seperti suhu,          ESP 32.
                    Berbasis Internet   petani sulit       melon secara efisien.   Fuzzy Sugeno dalam      kelembapan,            Menggunakan
                    Of Things (Eka      menentukan                                 pengolahan data.        intensitas cahaya,     algoritma fuzzy
                    Apriyani et al.,    masa tanam.                                                        dan kelembapan         dalam
                    2025).                                                                                 tanah, yang            pengolahan data.
                                                                                                           semuanya krusial       Transfer data
                                                                                                           bagi pertumbuhan       menggunakan
                                                                                                           tanaman melon.         protokol MQTT

            7       Prototipe Sistem    Bagaimana          Menerapkan sistem       Menggunakan fog         Sistem dapat bekerja   Penggunaan fog
                    Pemantauan Suhu     penempatan         pemantauan suhu         computing, komponen     normal seperti         computing
                    dan Kelembapan      perangkat IoT      dan kelembapanyang      hardware untuk          menggunakan cloud.     sebagai
                    berbasis Fog        agar saat tidak    mengacu pada            sensor node seperti     Selain itu, 25-50      arsitektur dalam
                    Computingpada       terdapat koneksi   konsep fog              DHT11, arduino nano,    meter adalah jarak     sistemnya.
                    Kumbung Jamur       internet dapat     computing.              relay, NRF24L01,        yang baik dalam hal
                    (Suharsono and      berjalan secara                            sprayer, dan MB-102     pengiriman data
                    Nurwarsito, 2022)   normal.                                    power modul,            menggunakan modul
                                                                                   perangkat Raspberry     NRF24L01.
                                                                                   diisi dengan Flask,
                                                                                   SQLite, kode python,
library.uns.ac.id                                                                        digilib.uns.ac.id
                                                                                                                                              14




            No      Artikel             Masalah            Tujuan               Metode                  Data dan Hasil        Keterkaitan

                                                                                HTML, CSS, dan
                                                                                javascript
            8       Purwarupa           Kebutuhan sistem   Menerapkan           Menggunakan             Fog computing dapat Penggunaan fog
                    Perangkat           yang memerlukan    teknologi IoT,       ESP8266,                mengambil alih        computing dan
                    Pelindung           internet stabil.   MQTT, dan fog        CD74HC4067 16-          fungsi kontrol ketika MQTT
                    Jemuran pada                           computing secara     Channel Analog          koneksi terputus.
                    Rumah Pintar                           bersamaan untuk      Digital Multiplexer     MQTT dapat
                    Berbasis IoT                           membuat perangkat    74HC4067, raindrop      berkomunikasi
                    Memanfaatkan                           pelindung jemuran    module sensor,          antara perangkat
                    MQTT dan Fog                           otomatis.            voltage comparator      dengan baik.
                    Computing.                                                  LM393 module, DHT
                    (Widianto et al.,                                           11, motor dc.
                    2022)                                                       Menggunakan
                                                                                protokol MQTT
                                                                                dalam transfer data
                                                                                dan arsitekur fog
                                                                                computing.
            9       Fog Computing       Kebutuhan sistem   Menerapkan fog       Menggunakan sensor      Hasil letensi rata-   Penggunaan fog
                    Service in the      yang real time     computing pada       suhu, ECG, dan          rata untuk simple     computing untuk
                    Healthcare          dan memberikan     sistem monitoring    tekanan darah.          report 22.499428 ms   melakukan
                    Monitoring          keamanan dan       kesehatan untuk      Menerapkan fog          dan 107.994685 ms     pengolahan data
                    System for          privasi.           mengurangi latensi   computing untuk         untuk completed       di lingkungan
                    Managing the                           dan meningkatkan     pemrosesan lokal dan    report.               lokal.
                    Real-Time                              keamanan.            cloud computing
                    Notification                                                untuk menyimpan
                    (Elhadad et al.,                                            data.
                    2022).
library.uns.ac.id                                                                           digilib.uns.ac.id
                                                                                                                                                     15




            No      Artikel              Masalah             Tujuan                Metode                  Data dan Hasil         Keterkaitan

            10      Development of       Keterbatasan        Menerapkan CoAP       Dibangun                CoAP memiliki          Penggunaan
                    CoAP protocol for    efisiensi dan       pada sistem robotik   menggunakan             waktu pemrosesan       CoAP untuk
                    communication in     keamanan            serta melakukan       ESP8266 sebagai         paling cepat, yaitu    melakukan
                    mobile robotic       komunikasi pada     evalusi.              pengendali dan server   antara 9 hingga 99     proses transmisi
                    systems using IoT    protokol IoT yang                         CoAP untuk              detik, diikuti oleh    data.
                    technique (Sarkar    ada seperti HTTP                          dikirimkan ke           MQTT dengan
                    et al., 2025)        dan MQTT ketika                           Thingspeak cloud.       waktu pemrosesan
                                         digunakan pada                                                    12 hingga 145 detik,
                                         perangkat dengan                                                  sedangkan HTTP
                                         sumber daya                                                       memiliki waktu
                                         terbatas                                                          paling lama yaitu 18
                                                                                                           hingga 217 detik.

                    Berdasarkan penelitian terkait tersebut menunjukkan penelitian sebelumnya protokol yang digunakan adalah MQTT dengan sensor
           untuk mengukur kelembapan tanah, intensitas cahaya, suhu dan kelembapan udara untuk pemantauan. Aktuator yang digunakan seperti
           water pump, LED, relay untuk pengendalian. Analisis data dilakukan di server cloud yang berada di luar jaringan lokal mikrokontroler.
           Penggunaan aplikasi Blynk untuk memantau parameter sensor dan pengendalian aktuator dari jarak jauh. Penelitian ini memanfaatkan
           konsep-konsep yang telah diterapkan pada penelitian-penelitian sebelumnya, namun memberikan pendekatan baru pada transfer data.
           Penelitian ini berfokus pada penerapan fog computing dan protokol CoAP untuk diterapkan dalam sistem IoT, serta pengujian Quality of
           Service dari sistem yang diterapkan. Alasan pendekatan ini untuk meningkatkan Quality of Service pada sistem IoT ditinjau dari parameter
           latensi, packet loss, dan jittte.
