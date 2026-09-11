#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <TAMC_GT911.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "ui.h"

// ==========================================
// PENGATURAN WIFI & MQTT (SESUAIKAN DENGAN DEVKIT)
// ==========================================
const char* ssid = "Adam Hafizh";         
const char* password = "bismillah07"; 
const char* mqtt_server = "broker.emqx.io";  
const int mqtt_port = 1883;
const char* mqtt_topic = "mesin/monitoring/data"; 

WiFiClient espClient;
PubSubClient client(espClient);

// Variabel Timer untuk Reconnect MQTT Non-Blocking
unsigned long lastReconnectAttempt = 0;

// ==========================================
// MAPPING WIDGET SQUARELINE
// ==========================================
#define UI_ARC_SUHU         ui_Arc1
#define UI_LABEL_SUHU       ui_Label2
#define UI_PANEL_SUHU       ui_Panel2
#define UI_CHART_SUHU       ui_Chart3
#define UI_CHART_SUHU_SER   ui_Chart3_series_1 

#define UI_ARC_GETARAN      ui_Arc2
#define UI_LABEL_GETARAN    ui_Label5
#define UI_PANEL_GETARAN    ui_Panel12
#define UI_CHART_GETARAN    ui_Chart1
#define UI_CHART_GETARAN_SER ui_Chart1_series_1

#define UI_ARC_ARUS         ui_Arc3
#define UI_LABEL_ARUS       ui_Label45
#define UI_PANEL_ARUS       ui_Panel14
#define UI_CHART_ARUS       ui_Chart4
#define UI_CHART_ARUS_SER   ui_Chart4_series_1

#define UI_ARC_RPM          ui_Arc5
#define UI_LABEL_RPM        ui_Label51
#define UI_PANEL_RPM        ui_Panel16
#define UI_CHART_RPM        ui_Chart5
#define UI_CHART_RPM_SER    ui_Chart5_series_1

#define UI_ARC_OVERALL      ui_Arc4
#define UI_LABEL_PERCENT    ui_Label28 
#define UI_LABEL_STATUS     ui_Label25 


// ==========================================
// 1. Konfigurasi Layar & Touch
// ==========================================
#define TFT_BL 2
Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    40, 41, 39, 42,
    45, 48, 47, 21, 14,
    5, 6, 7, 15, 16, 4,
    8, 3, 46, 9, 1,
    0, 8, 4, 43, 0, 8, 4, 12, 1, 16000000, false, 0, 0
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(480, 272, bus);

#define TOUCH_SDA     19
#define TOUCH_SCL     20
#define TOUCH_INT     18
#define TOUCH_RST     38
TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, 480, 272);

// ==========================================
// 2. Buffer LVGL v9 & Fungsi Jembatan
// ==========================================
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * 2)
static uint8_t draw_buf[DRAW_BUF_SIZE];

void my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t * px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
  lv_display_flush_ready(display);
}

void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data) {
  tp.read();
  if (tp.isTouched) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = tp.points[0].x;
    data->point.y = tp.points[0].y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

uint32_t my_tick_get_cb(void) { return millis(); }

// ==========================================
// 3. LOGIKA PREDICTIVE MAINTENANCE
// ==========================================
#define MAX_SUHU 100
#define CRIT_SUHU 40      // Sesuai batas kritis AI

#define MAX_ARUS 5.00
#define CRIT_ARUS 2.50    // Baseline normal

#define MAX_GETARAN 100
#define CRIT_GETARAN 15   // Baseline normal
#define MAX_RPM 3000
#define CRIT_RPM 2800

lv_chart_series_t * ser_suhu;
lv_chart_series_t * ser_arus;
lv_chart_series_t * ser_getaran;
lv_chart_series_t * ser_rpm;

// Ubah "int value, int crit_limit" menjadi "float value, float crit_limit"
void set_panel_status(lv_obj_t * panel, float value, float crit_limit) {
  if (value == 0.0) {
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFF00), LV_PART_MAIN); 
  } else if (value >= crit_limit) {
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFF0000), LV_PART_MAIN); 
  } else {
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x00FF00), LV_PART_MAIN); 
  }
}

// Ubah int arus menjadi float arus
void update_data_sensor(int suhu, float arus, int getaran, int rpm) {
    
    // 1. LIMITASI DATA
    if (suhu > MAX_SUHU) suhu = MAX_SUHU;
    if (arus > MAX_ARUS) arus = MAX_ARUS;
    if (getaran > MAX_GETARAN) getaran = MAX_GETARAN;
    if (rpm > MAX_RPM) rpm = MAX_RPM;

    // --- TEKNIK SCALING KHUSUS ARUS ---
    // Kalikan 100 dan jadikan integer (Contoh: 1.25A menjadi 125)
    int scaled_arus = (int)(arus * 100);

    // 2. UPDATE GAUGE & LABEL 
    lv_arc_set_value(UI_ARC_SUHU, suhu);
    lv_label_set_text_fmt(UI_LABEL_SUHU, "%d°C", suhu);

    // Arc dikirim nilai scaled
    lv_arc_set_value(UI_ARC_ARUS, scaled_arus);
    
    // Format angka desimal menggunakan String bawaan Arduino
    String teks_arus = String(arus, 2) + " A"; 
    lv_label_set_text(UI_LABEL_ARUS, teks_arus.c_str());

    lv_arc_set_value(UI_ARC_GETARAN, getaran);
    lv_label_set_text_fmt(UI_LABEL_GETARAN, "%d mm/s", getaran);

    lv_arc_set_value(UI_ARC_RPM, rpm);
    lv_label_set_text_fmt(UI_LABEL_RPM, "%d RPM", rpm);

    // 3. UPDATE CHART 
    lv_chart_set_next_value(UI_CHART_SUHU, ser_suhu, suhu);
    // Chart Arus dikirim nilai scaled
    lv_chart_set_next_value(UI_CHART_ARUS, ser_arus, scaled_arus);
    lv_chart_set_next_value(UI_CHART_GETARAN, ser_getaran, getaran);
    lv_chart_set_next_value(UI_CHART_RPM, ser_rpm, rpm);

    // 4. UPDATE WARNA PANEL STATUS 
    set_panel_status(UI_PANEL_SUHU, suhu, CRIT_SUHU);
    set_panel_status(UI_PANEL_ARUS, arus, CRIT_ARUS);
    set_panel_status(UI_PANEL_GETARAN, getaran, CRIT_GETARAN);
    set_panel_status(UI_PANEL_RPM, rpm, CRIT_RPM);

    // =========================================================
    // 5. KALKULASI OVERALL STATUS (RULES BERDASARKAN MANDOR AI)
    // =========================================================
    int health = 100;
    
    // A. Aturan Utama: Berdasarkan Suhu (Dominan)
    if (suhu >= 40) {
        // Rentang Kritis: Persentase kesehatan anjlok ke 0% - 59%
        health = map(suhu, 40, 100, 59, 10); 
    } 
    else if (suhu >= 36) {
        // Rentang Warning: Persentase kesehatan di 60% - 79%
        health = map(suhu, 36, 39, 79, 60);  
    } 
    else {
        // Rentang Normal: Persentase kesehatan di 80% - 100%
        health = map(suhu, 0, 35, 100, 80);  
    }

    // B. Aturan Kontekstual: Penalti dari Getaran dan Arus
    // Pada baseline 34°C, getaran > 15 atau arus > 2.5 bisa mengubah status
    if (getaran > 15 || arus > 2.50) {
        health -= 15; // Beri penalti/kurangi kesehatan sebesar 15%
    }

    // C. Pastikan nilai persentase tidak keluar batas (0 - 100)
    if (health > 100) health = 100;
    if (health < 0) health = 0;

    // D. Terapkan data ke Gauge Overall (Screen 9)
    lv_arc_set_value(UI_ARC_OVERALL, health); 
    lv_label_set_text_fmt(UI_LABEL_PERCENT, "%d%%", health); 

    // E. Logika Penentuan Warna dan Label Status Berdasarkan Health Score
    if (suhu == 0 && arus == 0.0 && getaran == 0 && rpm == 0) {
        lv_label_set_text(UI_LABEL_STATUS, "Status : Undetermined");
        lv_obj_set_style_arc_color(UI_ARC_OVERALL, lv_color_hex(0xFFFF00), LV_PART_INDICATOR); // Kuning
    } 
    else if (health >= 80) {
        lv_label_set_text(UI_LABEL_STATUS, "Status : Normal");
        lv_obj_set_style_arc_color(UI_ARC_OVERALL, lv_color_hex(0x00FF00), LV_PART_INDICATOR); // Hijau
    } 
    else if (health >= 60) {
        lv_label_set_text(UI_LABEL_STATUS, "Status : Warning");
        lv_obj_set_style_arc_color(UI_ARC_OVERALL, lv_color_hex(0xFFA500), LV_PART_INDICATOR); // Oranye
    } 
    else {
        lv_label_set_text(UI_LABEL_STATUS, "Status : Critical");
        lv_obj_set_style_arc_color(UI_ARC_OVERALL, lv_color_hex(0xFF0000), LV_PART_INDICATOR); // Merah
    }
}

// ==========================================
// 4. KONEKSI & CALLBACK MQTT
// ==========================================
void setup_wifi() {
  Serial.print("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  
  // Timeout agar tidak stuck selamanya jika WiFi mati
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Terhubung!");
  } else {
    Serial.println("\nGagal terhubung WiFi.");
  }
}

boolean reconnect() {
  if (client.connect("ESP32S3-Display-Client")) {
    Serial.println("MQTT Terhubung!");
    client.subscribe(mqtt_topic); // Dengarkan topik dari DEVKIT
    return client.connected();
  }
  return false;
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Ubah payload byte menjadi string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Parse JSON dari DEVKIT
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("Gagal membaca JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Ambil data sesuai dengan key JSON yang dibuat di DEVKIT
  int data_suhu = doc["suhu_objek"];
  float data_arus = doc["arus"]; // Pastikan menggunakan tipe float
  int data_getaran = doc["getaran"];
  int data_rpm = doc["kecepatan"];

  // Perbarui antarmuka layar
  update_data_sensor(data_suhu, data_arus, data_getaran, data_rpm);
}


// ==========================================
// 5. SETUP & LOOP UTAMA
// ==========================================
void setup() {
  Serial.begin(115200);

  gfx->begin();
  gfx->fillScreen(BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  tp.begin();
  tp.setRotation(ROTATION_INVERTED); 

  lv_init();
  lv_tick_set_cb(my_tick_get_cb);

  lv_display_t * disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();

  // Ambil referensi Chart secara dinamis
  ser_suhu = lv_chart_get_series_next(UI_CHART_SUHU, NULL);
  ser_arus = lv_chart_get_series_next(UI_CHART_ARUS, NULL);
  ser_getaran = lv_chart_get_series_next(UI_CHART_GETARAN, NULL);
  ser_rpm = lv_chart_get_series_next(UI_CHART_RPM, NULL);

  // Inisialisasi Jaringan
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback); // Hubungkan fungsi penerima pesan

  Serial.println("LVGL & MQTT Setup Selesai! Menunggu Data...");
}

void loop() {
  // Manajemen Koneksi MQTT (Non-Blocking)
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      unsigned long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      client.loop(); // Wajib dipanggil agar bisa menerima pesan MQTT
    }
  }

  // Handle animasi dan sentuhan UI LVGL
  lv_timer_handler(); 
  delay(5);
}