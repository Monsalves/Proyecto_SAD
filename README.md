# Proyecto SAD - Monitoreo del Ambiente

Este proyecto implementa un sistema de monitoreo del ambiente utilizando un **ESP32**, con diferentes sensores conectados al microcontrolador y enviando los datos a un **broker MQTT** .

## 📡 Sensores utilizados
- **SPS30** → Medición de material particulado (PM1.0, PM2.5, PM4.0, PM10).  
- **DHT22** → Medición de temperatura y humedad.  
- **MQ-7** → Medición de concentración de monóxido de carbono (lectura analógica).  

## 📁 Estructura del proyecto
El código está modularizado en carpetas dentro de `/lib/`:
- `WiFiManager/` → Manejo de conexión WiFi.  
- `MQTTManager/` → Configuración de cliente MQTT y reconexiones.  
- `TimeManager/` → Sincronización y obtención de hora actual vía NTP.  
- `SensorSPS30/` → Inicialización y lectura del sensor SPS30.  
- `SensorDHT22/` → Inicialización y lectura del sensor DHT22.  
- `SensorMQ7/` → Inicialización y lectura del sensor MQ-7.  

## 🛠️ Configuración previa
Antes de compilar y subir el proyecto al ESP32, **se debe configurar el archivo `Config.h`** con los parámetros específicos de:

- Pines de conexión de los sensores.  
- Credenciales de red WiFi (`WIFI_SSID`, `WIFI_PASSWORD`).  
- Dirección IP/host y puerto del **broker MQTT** (`MQTT_SERVER`, `MQTT_PORT`).  
- Tópicos de publicación y suscripción.  
- Servidores NTP y zona horaria.  

Ejemplo de `Config.h`:
```cpp
#pragma once

// Pines de sensores
#define PIN_DHT 26
#define TIPO_DHT DHT22
#define PIN_MQ7 32     
#define PIN_SPS_RX 16
#define PIN_SPS_TX 17

// WiFi
const char* WIFI_SSID = "WIFI";
const char* WIFI_PASSWORD = "PASSWORD";

// MQTT
const char* MQTT_SERVER = "IP";
const int   MQTT_PORT   = 0000;
const char* TOPIC_SUB   = "topicsub";
const char* TOPIC_SPS30 = "topicpub/sps30";
const char* TOPIC_DHT22 = "topicpub/dht22";
const char* TOPIC_MQ7   = "topicpub/mq7";

// NTP
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.nist.gov";
const long  GMT_OFFSET_SEC     = -4 * 3600;
const int   DAYLIGHT_OFFSET_SEC = 0;
