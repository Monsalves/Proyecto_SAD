#pragma once

// ================== PINES ==================
// --- DHT22 ---
#define PIN_DHT   26
#define TIPO_DHT  DHT22

// --- MQ-7 ---
#define PIN_MQ7   32   // Entrada analógica

// --- SPS30 (UART2 ESP32) ---
#define PIN_SPS_RX 16
#define PIN_SPS_TX 17

// --- LED ---
#define PIN_B 33
#define PIN_G 25
#define PIN_R 27

// ================== WIFI ==================
#define WIFI_SSID     "PerryZone"
#define WIFI_PASSWORD "agenteP123"

// ================== MQTT ==================
#define MQTT_SERVER   "200.13.4.202"
#define MQTT_PORT     1883

// Tópico de suscripción
#define TOPIC_SUB     "SAD/led"

// Tópicos de publicación
#define TOPIC_SPS30   "SAD/sps30"
#define TOPIC_DHT22   "SAD/dht22"
#define TOPIC_MQ7     "SAD/mq7"

// ================== NTP (Hora de Chile) ==================
#define NTP_SERVER_1       "pool.ntp.org"
#define NTP_SERVER_2       "time.nist.gov"
#define GMT_OFFSET_SEC     (-4 * 3600)  // -4 horas
#define DAYLIGHT_OFFSET_SEC 0           // Ajustar si aplica horario de verano
