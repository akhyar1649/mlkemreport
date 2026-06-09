# OpenWrt Lab Setup — Dual SSID (LAB + MGMT), Channel 11, VLAN Separation, Strict MQTT-Only Policy

Dokumen ini menyiapkan lingkungan jaringan terkontrol untuk Scenario 1 (MQTT handshake) agar hasil latency/jitter adil dan replikabel. Topologi akhir:

- SSID LAB: `mqtt` (untuk ESP32)
- SSID MGMT: `mgmt` (untuk broker/subscriber/laptop manajemen)
- Radio: 2.4 GHz (radio0), channel dipin ke 11
- WPA2-PSK untuk kedua SSID: `A12345678`
- VLAN/logical separation: interface `lab` terpisah dari `lan`
- Firewall: hanya izinkan LAB → broker:1883/TCP; DHCP & DNS diizinkan; selebihnya diblokir
- Static DHCP leases:
  - Broker MAC 8e:01:68:4f:b7:0e → 192.168.60.10
  - Subscriber MAC 9e:a5:6b:14:65:09 → 192.168.60.11

Mengapa begini:
- Dua SSID + pemisahan logical network mengisolasi trafik ESP32 dari trafik manajemen → stabilitas latency.
- Channel pinning ke 11 menghindari channel hopping otomatis yang menambah jitter; pilih 1/6/11 sesuai survey spektrum (di sini: 11).
- Strict firewall menghilangkan variabel trafik lain dari LAB.
- Static IP memudahkan URI broker yang konsisten antar-run.
- Di sisi broker, `TCP_NODELAY` mencegah artefak Nagle/Delayed ACK yang terbukti menyebabkan gap ratusan ms pada payload kecil.

Catatan ESP32 (firmware): Wi‑Fi power save dimatikan (`esp_wifi_set_ps(WIFI_PS_NONE)`) untuk menghindari sleep jitter; TX power diset (`esp_wifi_set_max_tx_power(52)`).

---

## 1) Pre‑checks (opsional)

```
# Lihat interface Wi‑Fi dan radio
iw dev

# Opsional: survey crowding untuk pemilihan channel (1/6/11)
iw dev <iface> scan | egrep 'SSID|signal|DS Parameter set'
```

---

## 2) Wireless: dua SSID pada radio0 (2.4 GHz) — channel 11

```
uci set wireless.radio0.channel='11'
uci set wireless.radio0.country='ID'
uci set wireless.radio0.htmode='HT20'

# SSID LAB (mqtt) → network "lab"
uci add wireless wifi-iface
uci set wireless.@wifi-iface[-1].device='radio0'
uci set wireless.@wifi-iface[-1].mode='ap'
uci set wireless.@wifi-iface[-1].ssid='mqtt'
uci set wireless.@wifi-iface[-1].encryption='psk2'
uci set wireless.@wifi-iface[-1].key='A12345678'
uci set wireless.@wifi-iface[-1].network='lab'

# SSID MGMT (mgmt) → network "lan"
uci add wireless wifi-iface
uci set wireless.@wifi-iface[-1].device='radio0'
uci set wireless.@wifi-iface[-1].mode='ap'
uci set wireless.@wifi-iface[-1].ssid='mgmt'
uci set wireless.@wifi-iface[-1].encryption='psk2'
uci set wireless.@wifi-iface[-1].key='A12345678'
uci set wireless.@wifi-iface[-1].network='lan'

uci commit wireless
wifi reload
```

---

## 3) Network + DHCP: interface baru `lab` (192.168.60.0/24)

```
# Interface L3 terpisah untuk SSID LAB
uci set network.lab='interface'
uci set network.lab.proto='static'
uci set network.lab.ipaddr='192.168.60.1'
uci set network.lab.netmask='255.255.255.0'
uci set network.lab.ip6assign='60'
uci commit network

# DHCP server untuk LAB
uci set dhcp.lab='dhcp'
uci set dhcp.lab.interface='lab'
uci set dhcp.lab.start='100'

uci set dhcp.lab.limit='150'
uci set dhcp.lab.leasetime='12h'

# Static leases
uci add dhcp host
uci set dhcp.@host[-1].name='broker'
uci set dhcp.@host[-1].mac='8e:01:68:4f:b7:0e'
uci set dhcp.@host[-1].ip='192.168.60.10'
uci set dhcp.@host[-1].leasetime='infinite'

uci add dhcp host
uci set dhcp.@host[-1].name='subscriber'
uci set dhcp.@host[-1].mac='9e:a5:6b:14:65:09'
uci set dhcp.@host[-1].ip='192.168.60.11'
uci set dhcp.@host[-1].leasetime='infinite'

uci commit dhcp
/etc/init.d/dnsmasq restart
```

Catatan: Konfigurasi ini memisahkan L3; pada perangkat OpenWrt modern berbasis DSA, VLAN pada port LAN dapat ditambahkan bila perlu, namun untuk skenario Wi‑Fi‑only lab ini tidak wajib.

---

## 4) Firewall: zona baru `lab` dan kebijakan MQTT‑only

```
# Zona firewall untuk LAB
uci add firewall zone
uci set firewall.@zone[-1].name='lab'
uci set firewall.@zone[-1].input='ACCEPT'
uci set firewall.@zone[-1].output='ACCEPT'
uci set firewall.@zone[-1].forward='REJECT'
uci set firewall.@zone[-1].network='lab'

# Izinkan DHCP (67/68) + DNS (53) dari LAB ke router
uci add firewall rule
uci set firewall.@rule[-1].name='Allow-DHCP-DNS-from-LAB'
uci set firewall.@rule[-1].src='lab'
uci set firewall.@rule[-1].proto='tcpudp'
uci set firewall.@rule[-1].dest_port='53 67-68'
uci set firewall.@rule[-1].target='ACCEPT'

# Izinkan LAB → LAN hanya ke broker:1883/TCP
uci add firewall rule
uci set firewall.@rule[-1].name='LAB-to-Broker-1883'
uci set firewall.@rule[-1].src='lab'
uci set firewall.@rule[-1].dest='lan'
uci set firewall.@rule[-1].dest_ip='192.168.60.10'
uci set firewall.@rule[-1].proto='tcp'
uci set firewall.@rule[-1].dest_port='1883'
uci set firewall.@rule[-1].target='ACCEPT'

# Tidak menambahkan forwarding default dari LAB → LAN/WAN (forward=REJECT sudah menolak lainnya)

uci commit firewall
/etc/init.d/firewall restart
```

Opsional debugging (boleh diaktifkan sementara):

```
# Izinkan ICMP ping dari LAB ke broker (non-wajib)
uci add firewall rule
uci set firewall.@rule[-1].name='LAB-allow-ICMP-to-broker'
uci set firewall.@rule[-1].src='lab'
uci set firewall.@rule[-1].dest='lan'
uci set firewall.@rule[-1].dest_ip='192.168.60.10'
uci set firewall.@rule[-1].proto='icmp'
uci set firewall.@rule[-1].target='ACCEPT'
uci commit firewall
/etc/init.d/firewall restart
```

---

## 5) Validasi lingkungan

```
# Wi‑Fi up?
iw dev
logread -e hostapd

# Alamat interface lab
ip addr show br-lab 2>/dev/null || ip addr | grep -A2 '192.168.60.1'

# DHCP/leases
cat /tmp/dhcp.leases | egrep '192.168.60.1|192.168.60.10|192.168.60.11'

# Dari host broker/subscriber (MGMT/mgmt SSID):
ping -c 2 192.168.60.10

# Uji MQTT dari subscriber host:
mosquitto_pub -h 192.168.60.10 -p 1883 -u mlkem -P 12345678 -t test/topic -m ok
mosquitto_sub -h 192.168.60.10 -p 1883 -u mlkem -P 12345678 -t test/# -v
```

Pastikan broker/subscriber berada di SSID `mgmt`, sedangkan ESP32 pada SSID `mqtt`.

---

## 6) Broker: Fedora Mosquitto (TCP_NODELAY aktif)

Gunakan file contoh proyek `tools/fedora_mosquitto.conf` dan jalankan broker sebagai berikut:

```
sudo dnf install -y mosquitto mosquitto-clients
sudo mosquitto_passwd -c /etc/mosquitto/pwfile mlkem
sudo mosquitto -c tools/fedora_mosquitto.conf
```

Konfig sudah menyetel:

- `listener 1883 0.0.0.0`
- `allow_anonymous false`
- `password_file /etc/mosquitto/pwfile`
- `set_tcp_nodelay true`  ← penting untuk mencegah kombinasi Nagle + delayed ACK pada payload kecil yang dapat menambah ratusan ms latency.

---

## 7) Ringkasan keputusan metodologis jaringan

- Dua SSID + pemisahan logical network dan firewall ketat mengurangi noise jaringan dan memastikan fairness antar varian ML‑KEM.
- `TCP_NODELAY` di broker menghilangkan bias protokol TCP terhadap ukuran payload kecil (terkonfirmasi oleh data baru: semua varian kini di rentang ~22–40 ms untuk net latency pada PIE off).
- ESP32 diminta non‑power‑save selama pengukuran handshake untuk mencegah jitter radio.

Lampiran: Untuk pemilihan channel alternatif (1 atau 6), ulangi Step 1 (survey) dan ganti `wireless.radio0.channel` menjadi `1`/`6` sesuai hasil survey; sisanya tetap sama.
