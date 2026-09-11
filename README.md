# 🖥️ MANDOR - Smart HMI Dashboard (Display Node)

![Status](https://img.shields.io/badge/Status-Completed-success)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue)
![Framework](https://img.shields.io/badge/UI_Framework-LVGL_v9.5-orange)
![Protocol](https://img.shields.io/badge/Protocol-MQTT-yellow)

This repository contains the source code for the **Display Node (HMI Dashboard)** of the **MANDOR (Predictive Maintenance)** project. This device acts as a wireless monitoring center that visualizes real-time telemetry data from an industrial conveyor machine using the **MQTT communication protocol**.

> **Note:** For the **Sensor Node** (data acquisition) and **Edge AI** source code, please visit the main MANDOR repository.

---

## ✨ Main Features

* **Smart UI/UX:** An interactive interface consisting of 9 integrated screens, designed using SquareLine Studio and smoothly rendered using the LVGL v9.5 graphics engine.
* **Real-Time Data Visualization:** Displays interactive gauges and dynamic line charts for 4 machine parameters: Temperature (°C), Current (A), Vibration (mm/s), and Speed (RPM).
* **AI-Driven Health Score:** Implements a local Decision Support System algorithm that converts raw sensor data (mainly temperature dominance with vibration/current penalties) into a machine health percentage indicator (Normal, Warning, Critical).
* **Float-to-Int Scaling:** A precision data manipulation technique to render decimal values (such as 0.02 A current readings) into the integer-based LVGL graphics engine without losing visual accuracy.
* **Watchdog & Non-Blocking MQTT:** Includes a connection loss detection system (*Lost Contact*) that resets the display when the sensor node becomes unavailable, along with non-blocking JSON parsing to maintain smooth UI animations.

---

## 🛠️ Hardware Specifications (Display)

* **Main Board:** ESP32-S3 RGB Display 4.3 Inch (Model: 4827S043)
* **Resolution:** 480x272 Pixels
* **Touch Screen:** GT911 (Capacitive Touch)
* **Memory Configuration:** 16MB Flash, 8MB OPI PSRAM
* **Power Supply:** 5V via USB Type-C (Independent/Portable)

---

## 💻 Software Dependencies

Make sure the following libraries are installed in Arduino IDE before compiling the project:

* [LVGL v9.5](https://github.com/lvgl/lvgl) - HMI graphics engine
* [Arduino_GFX](https://github.com/moononournation/Arduino_GFX) - ESP32 RGB panel driver
* [TAMC_GT911](https://github.com/TAMC/TAMC_GT911) - Touch screen driver
* [PubSubClient](https://github.com/knolleary/pubsubclient) - MQTT client library
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - JSON payload processor from MQTT broker

---

## 🚀 Installation & Upload Guide

1. Clone this repository to your computer.
2. Open the `.ino` file using Arduino IDE.
3. Configure the WiFi and MQTT Broker settings (`broker.emqx.io` or another broker) in the variables section at the top of the code.
4. **IMPORTANT - Arduino IDE Configuration:**

   * **Board:** ESP32S3 Dev Module
   * **Flash Size:** 16MB (128Mb)
   * **Partition Scheme:** 16M Flash (8MB APP/8MB SPIFFS) or equivalent
   * **PSRAM:** OPI PSRAM
   * **USB CDC On Boot:** Enabled / Disabled (Adjust according to your USB connection)

5. Click **Upload**. Make sure the Serial Monitor is closed during the upload process.

---

## 👥 Development Team

Developed by **Industrial Informatics students** from:

**Politeknik Manufaktur Bandung (Polman Bandung)**

* Muhammad Daffi Izzuddin
* Bintang Shobri Al Chakim
* Ilham Nur Fiqri
