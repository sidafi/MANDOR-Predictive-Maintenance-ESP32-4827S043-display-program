# 🖥️ MANDOR - Smart HMI Dashboard (Display Node)

![Status](https://img.shields.io/badge/Status-Completed-success)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue)
![Framework](https://img.shields.io/badge/UI_Framework-LVGL_v9.5-orange)
![Protocol](https://img.shields.io/badge/Protocol-MQTT-yellow)

Repositori ini berisi kode sumber untuk **Display Node (HMI Dashboard)** dari proyek **MANDOR (Predictive Maintenance)**. Perangkat ini berfungsi sebagai pusat pemantauan nirkabel yang memvisualisasikan data telemetri dari mesin konveyor industri secara *real-time* menggunakan protokol MQTT.

> **Catatan:** Untuk kode *Sensor Node* (pengambil data) dan *Edge AI*, silakan kunjungi repositori utama MANDOR.

---

## ✨ Fitur Utama
* **Smart UI/UX:** Antarmuka interaktif yang terdiri dari 9 layar terintegrasi, dirancang menggunakan SquareLine Studio dan dirender dengan mulus oleh *engine* grafis LVGL v9.5.
* **Real-Time Data Visualization:** Menampilkan *Gauge* interaktif dan *Line Chart* dinamis untuk 4 parameter mesin: Suhu (°C), Arus (A), Getaran (mm/s), dan Kecepatan (RPM).
* **AI-Driven Health Score:** Memiliki algoritma *Decision Support System* lokal yang mengubah data sensor mentah (terutama dominansi suhu dan penalti getaran/arus) menjadi persentase kesehatan mesin (Normal, Warning, Critical).
* **Float-to-Int Scaling:** Teknik manipulasi presisi data untuk merender grafik nilai desimal (seperti arus 0.02 A) ke dalam *engine* integer LVGL tanpa kehilangan akurasi visual.
* **Watchdog & Non-Blocking MQTT:** Dilengkapi sistem pendeteksi putus koneksi (*Lost Contact*) yang akan mereset tampilan jika sensor node mati, serta sistem parsing JSON yang tidak memblokir animasi layar.

## 🛠️ Spesifikasi Perangkat Keras (Display)
* **Board Utama:** ESP32-S3 RGB Display 4.3 Inch (Model: 4827S043)
* **Resolusi:** 480x272 Pixels
* **Layar Sentuh:** GT911 (Capacitive Touch)
* **Konfigurasi Memori:** 16MB Flash, 8MB OPI PSRAM
* **Catu Daya:** 5V via USB Type-C (Independen/Portable)

## 💻 Pustaka Perangkat Lunak (Dependencies)
Pastikan pustaka berikut telah diinstal pada Arduino IDE Anda sebelum melakukan kompilasi:
* [LVGL v9.5](https://github.com/lvgl/lvgl) - Engine grafis HMI
* [Arduino_GFX](https://github.com/moononournation/Arduino_GFX) - Driver panel RGB ESP32
* [TAMC_GT911](https://github.com/TAMC/TAMC_GT911) - Driver layar sentuh
* [PubSubClient](https://github.com/knolleary/pubsubclient) - Klien MQTT
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - Pemroses *payload* JSON dari broker

## 🚀 Cara Instalasi & Upload
1. *Clone* repositori ini ke komputer Anda.
2. Buka file `.ino` menggunakan Arduino IDE.
3. Konfigurasikan pengaturan WiFi dan MQTT Broker (`broker.emqx.io` atau yang lain) pada variabel di bagian atas kode.
4. **PENTING - Pengaturan Arduino IDE:**
   * **Board:** ESP32S3 Dev Module
   * **Flash Size:** 16MB (128Mb)
   * **Partition Scheme:** 16M Flash (8MB APP/8MB SPIFFS) *atau yang setara*
   * **PSRAM:** OPI PSRAM
   * **USB CDC On Boot:** Enabled / Disabled (Sesuaikan dengan port colokan USB Anda)
5. Klik **Upload**. Pastikan Serial Monitor ditutup selama proses *upload*.

## 👥 Tim Pengembang
Dikembangkan oleh mahasiswa **Informatika Industri**, **Politeknik Manufaktur Bandung (Polman Bandung)**:
* Muhammad Daffi Izzuddin
* Bintang Shobri Al Chakim
* Ilham Nur Fiqri
