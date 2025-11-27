

# DaFiq-EvilTwin (Educational Research Version)

**⚠️ Penting:**
Proyek ini dibuat **hanya untuk keperluan riset keamanan, pembelajaran, dan pengujian di lingkungan milik sendiri**.
**Jangan pernah menggunakan alat ini pada jaringan atau kredensial milik orang lain tanpa izin eksplisit.**
Penggunaan tanpa izin melanggar hukum di banyak negara.

Proyek ini merupakan modifikasi dari:

* [https://github.com/M1z23R/ESP8266-EvilTwin](https://github.com/M1z23R/ESP8266-EvilTwin)
* Terinspirasi oleh [https://github.com/vk496/linset](https://github.com/vk496/linset) dan [https://github.com/SpacehuhnTech/esp8266_deauther](https://github.com/SpacehuhnTech/esp8266_deauther)

Tujuan modifikasi:
Menyediakan lingkungan *lab penetration-testing* untuk mempelajari konsep **Evil Twin**, **WiFi authentication workflow**, dan **user credential capture** pada sistem captive portal seperti *MyPublicWiFi*, dalam konteks **pengujian keamanan yang sah**.

---

## ✨ Perubahan Utama pada Versi Ini

### 🔧 Modifikasi dari repository original

Beberapa perubahan yang dilakukan pada versi ini:

1. **SSID default diubah**

   * SSID: `DaFiq`
   * Password: `deauther` (sama seperti versi asli)

2. **Penambahan beberapa endpoint untuk lab testing**

   * `/admin` — halaman admin untuk melihat daftar target hotspot
   * `/result` — halaman test koneksi WiFi
   * `/login` — halaman login mahasiswa yang muncul saat mode Evil Twin aktif
   * `/attempt` — endpoint untuk menerima percobaan login user

3. **Perilaku setelah password hotspot diperoleh**

   * Pada repo asli, Evil Twin langsung memutus koneksi setelah mendapatkan password.
   * Pada versi ini:

     * Password diuji menggunakan fungsi koneksi WiFi internal (WiFi.connect).
     * Jika koneksi **berhasil**, pengguna diarahkan ke `/login` untuk mensimulasikan proses login captive portal.
     * Hal ini digunakan untuk **pengujian keamanan kredensial** pada jaringan seperti MyPublicWiFi di lingkungan lab.

4. **Workflow pengujian lebih terstruktur**
   Fokus versi ini adalah mempelajari:

   * bagaimana perangkat mencoba terhubung ulang ke hotspot asli,
   * bagaimana pengguna diarahkan ke halaman login palsu dalam skenario uji,
   * dan bagaimana sistem menangani kredensial yang dicoba.

---

## 📦 Cara Kompilasi

Sebelum kompilasi, pastikan mengikuti langkah konfigurasi dari proyek berikut:
[https://github.com/SpacehuhnTech/esp8266_deauther/tree/v1](https://github.com/SpacehuhnTech/esp8266_deauther/tree/v1)

Bagian penting:
Aktifkan **wifi_send_pkt_freedom**, karena diperlukan untuk fungsi deauth (pada lingkungan pengujian).

Setelah itu:

1. Buka project di **Arduino IDE**
2. Pilih board **ESP8266**
3. Compile & upload

HTML dapat dimodifikasi sesuai kebutuhan, karena saat ini masih menggunakan string.

---

## 🧪 Cara Penggunaan (Lingkungan Pengujian Sah)

1. Hubungkan perangkat Anda ke AP:

   * **SSID:** `DaFiq`
   * **Password:** `deauther`

2. Buka halaman admin:
   `http://192.168.4.1/admin`

3. Pilih target hotspot yang akan diuji (daftar refresh otomatis setiap ±30 detik)

4. Jalankan simulasi *Evil Twin* untuk mempelajari proses koneksi ulang.

5. Setelah pengguna memasukkan password hotspot:

   * Sistem **tidak langsung memutus** jaringan Evil Twin
   * Password diuji dengan mencoba koneksi ke hotspot asli
   * Jika berhasil, pengguna akan diarahkan ke halaman login uji:
     `http://192.168.4.1/login`

6. Endpoint `/attempt` berjalan sebagai penerima kredensial login untuk keperluan simulasi *captive portal testing*.

---

## 📑 Endpoint yang Tersedia

| Endpoint   | Deskripsi                                             |
| ---------- | ----------------------------------------------------- |
| `/admin`   | Melihat daftar target hotspot dalam mode pengujian    |
| `/result`  | Mengecek status koneksi WiFi                          |
| `/login`   | Halaman login mahasiswa untuk simulasi captive portal |
| `/attempt` | Endpoint untuk menerima kredensial (pengujian)        |

---

## ⚖️ Legal & Etika

Proyek ini **tidak dimaksudkan untuk disalahgunakan**.
Selalu pastikan Anda:

* memiliki izin tertulis,
* bekerja di jaringan milik sendiri,
* mematuhi hukum dan regulasi setempat.

---

## 🙏 Kredit

* Deauthing: [https://github.com/SpacehuhnTech/esp8266_deauther](https://github.com/SpacehuhnTech/esp8266_deauther)
* ESP8266 Core: [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)
* Original Evil Twin base: [https://github.com/M1z23R/ESP8266-EvilTwin](https://github.com/M1z23R/ESP8266-EvilTwin)

---

