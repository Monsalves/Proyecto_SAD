#include "SensorDHT22.h"
#include "../../include/Config.h"
#include "MQTTManager.h"
#include "TimeManager.h"
#include <DHT.h>
#include <Arduino.h>

DHT dht(PIN_DHT, TIPO_DHT);
unsigned long previousHumTempMillis = 0;
const long humTempInterval = 1000;

void iniciarDHT22() { dht.begin(); }

StaticJsonDocument<200> medirHumedadTemperatura() {
  StaticJsonDocument<200> doc;
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  if (!isnan(temp) && !isnan(hum)) {
      doc["Temperatura"] = temp;
      doc["Humedad"] = hum;
  } else doc["error"] = "Error lectura DHT22";
  return doc;
}

void enviarHumedadTemperatura() {
  StaticJsonDocument<200> doc = medirHumedadTemperatura();
  doc["device"] = "DHT22";
  doc["timestamp"] = getUnixTimestamp();

  size_t jsonSize = measureJson(doc);
  char jsonBuffer[jsonSize + 1];
  serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

  if (client.publish(TOPIC_DHT22, jsonBuffer))
      Serial.println("📤 Envio DHT22: " + String(jsonBuffer));
  else
      Serial.println("❌ Fallo al enviar DHT22");
}

void generarEnvioHumedadTemperatura() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousHumTempMillis >= humTempInterval) {
    previousHumTempMillis = currentMillis;
    enviarHumedadTemperatura();
  }
}