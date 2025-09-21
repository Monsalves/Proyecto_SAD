#pragma once
#include <ArduinoJson.h>
void iniciarDHT22();
StaticJsonDocument<200> medirHumedadTemperatura();
void enviarHumedadTemperatura();
void generarEnvioHumedadTemperatura();