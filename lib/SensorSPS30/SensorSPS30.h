#pragma once
#include <SensirionUartSps30.h>
#include <ArduinoJson.h>
void iniciarSPS30();
StaticJsonDocument<200> medirMaterialParticulado();
void enviarMaterialParticulado();
void generarEnvioMaterialParticulado();