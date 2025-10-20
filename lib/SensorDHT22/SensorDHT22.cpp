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

String medirHumedadTemperatura() {
  String doc = "{";
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  if (!isnan(temp) && !isnan(hum)) {
    doc += "\"Temperatura\": " + String(temp) + ", ";
    doc += "\"Humedad\": " + String(hum);
  } else doc += "\"error\": \"Error lectura DHT22\"";
  doc += "}";

  return doc;
}

void enviarHumedadTemperatura() {
  String payload = medirHumedadTemperatura();

  if (payload.endsWith("}")) {
    payload.remove(payload.length() - 1);
    if (payload.length() > 1) payload += ", ";
    payload += "\"device\": \"DHT22\", \"timestamp\": ";
    payload += String(getUnixTimestamp());
    payload += "}";
  }

  if (client.publish(TOPIC_DHT22, payload.c_str())) {
    Serial.println("📤 Envio DHT22: " + payload);
  } else {
    Serial.println("❌ Fallo al enviar DHT22");
  }
}


void generarEnvioHumedadTemperatura() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousHumTempMillis >= humTempInterval) {
    previousHumTempMillis = currentMillis;
    enviarHumedadTemperatura();
  }
}