#include "Config.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "TimeManager.h"
#include "SensorSPS30.h"
#include "SensorDHT22.h"
#include "SensorMQ7.h"

void setup() {
  Serial.begin(115200);

  conectarWiFi();

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  iniciarHora();

  iniciarDHT22();
  iniciarSPS30();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  generarEnvioMaterialParticulado();
  generarEnvioHumedadTemperatura();
  generarEnvioCO();

  delay(2000);
}
