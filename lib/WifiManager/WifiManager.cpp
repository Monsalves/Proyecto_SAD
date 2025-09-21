#include <WiFi.h>
#include "WiFiManager.h"
#include "../../include/Config.h"
#include <Arduino.h>

void conectarWiFi() {
  Serial.print("🔌 Conectando a WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado. IP: " + WiFi.localIP().toString());
}