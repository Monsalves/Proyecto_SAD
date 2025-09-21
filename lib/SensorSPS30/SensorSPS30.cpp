#include "SensorSPS30.h"
#include "../../include/Config.h"
#include "MQTTManager.h"
#include "TimeManager.h"
#include <Arduino.h>

HardwareSerial spsSerial(2); 
SensirionUartSps30 sensor;

unsigned long previousMaterialMillis = 0;
const long materialInterval = 1000;

void iniciarSPS30() {
    spsSerial.begin(115200, SERIAL_8N1, PIN_SPS_RX, PIN_SPS_TX);
    sensor.begin(spsSerial);
    sensor.stopMeasurement();
    delay(100);
    if (sensor.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT) != 0) {
        Serial.println("Error iniciando medición SPS30");
    }
}

StaticJsonDocument<200> medirMaterialParticulado(){
  StaticJsonDocument<200> doc;
  
  float mc1p0, mc2p5, mc4p0, mc10p0;
  float nc0p5, nc1p0, nc2p5, nc4p0, nc10p0, typicalSize;

  int16_t error = sensor.readMeasurementValuesFloat(
    mc1p0, mc2p5, mc4p0, mc10p0,
    nc0p5, nc1p0, nc2p5, nc4p0, nc10p0, typicalSize
  );

  if (error == 0) {
      doc["PM1.0"] = mc1p0;
      doc["PM2.5"] = mc2p5;
      doc["PM4.0"] = mc4p0;
      doc["PM10"] = mc10p0;
  } else {
      doc["error"] = error;
  }
  return doc;
}

void enviarMaterialParticulado() {
  StaticJsonDocument<200> doc = medirMaterialParticulado();
  doc["device"] = "SPS30";
  doc["timestamp"] = getUnixTimestamp();

  size_t jsonSize = measureJson(doc);
  char jsonBuffer[jsonSize + 1];
  serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

  if (client.publish(TOPIC_SPS30, jsonBuffer)) {
    Serial.println("📤 Envio SPS30: " + String(jsonBuffer));
  } else {
    Serial.println("❌ Fallo al enviar SPS30");
  }
}

void generarEnvioMaterialParticulado() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMaterialMillis >= materialInterval) {
    previousMaterialMillis = currentMillis;
    enviarMaterialParticulado();
  }
}