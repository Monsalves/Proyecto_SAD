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

// ================== WIFI ==================
#define WIFI_SSID     "WIFI"
#define WIFI_PASSWORD "00000000"

// ================== MQTT ==================
#define MQTT_SERVER   "servidor_mqtt"
#define MQTT_PORT     1883

// Tópico de suscripción
#define TOPIC_SUB     "ufrocasa/si"

// Tópicos de publicación
#define TOPIC_SPS30   "ufrocasa/sps30"
#define TOPIC_DHT22   "ufrocasa/dht22"
#define TOPIC_MQ7     "ufrocasa/mq7"

// ================== NTP (Hora de Chile) ==================
#define NTP_SERVER_1       "pool.ntp.org"
#define NTP_SERVER_2       "time.nist.gov"
#define GMT_OFFSET_SEC     (-4 * 3600)  // -4 horas
#define DAYLIGHT_OFFSET_SEC 0           // Ajustar si aplica horario de verano
