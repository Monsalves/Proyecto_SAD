#include "MQTTManager.h"
#include <Config.h>
#include <Arduino.h>
#include <WiFi.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("📩 Message received on topic '");
  Serial.print(topic);
  Serial.print("': ");
  Serial.println(msg);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("🔁 Intentando conexión con MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ Conectado a MQTT");
      if (client.subscribe(TOPIC_SUB)) {
        Serial.print("📡 Suscrito a topic: ");
        Serial.println(TOPIC_SUB);
      } else {
        Serial.println("❌ Error al suscribirse al topic.");
      }
    } else {
      Serial.print("❌ MQTT Failed. State: ");
      Serial.println(client.state());
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
