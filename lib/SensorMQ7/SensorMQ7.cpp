#include "SensorMQ7.h"
#include "../../include/Config.h"
#include "MQTTManager.h"
#include "TimeManager.h"
#include <Arduino.h>

unsigned long previousCOMillis = 0;
const long coInterval = 1000;

StaticJsonDocument<200> medirCO() {
  StaticJsonDocument<200> doc;
  int mq7Value = analogRead(PIN_MQ7);
  doc["CO"] = mq7Value;
  return doc;
}

void enviarCO() {
  StaticJsonDocument<200> doc = medirCO();
  doc["device"] = "MQ7";
  doc["timestamp"] = getUnixTimestamp();

  size_t jsonSize = measureJson(doc);
  char jsonBuffer[jsonSize + 1];
  serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

  if (client.publish(TOPIC_MQ7, jsonBuffer))
      Serial.println("📤 Envio CO: " + String(jsonBuffer));
  else
      Serial.println("❌ Fallo al enviar CO");
}

void generarEnvioCO() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousCOMillis >= coInterval) {
    previousCOMillis = currentMillis;
    enviarCO();
  }
}