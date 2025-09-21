#include "TimeManager.h"
#include <Config.h>
#include <Arduino.h>
#include "time.h"

void iniciarHora() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);
  Serial.println("Esperando sincronización de hora de Chile...");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  struct tm timeinfo;
  getLocalTime(&timeinfo); 
  char timeBuffer[30];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  Serial.println("\n✅ Hora de Chile sincronizada: " + String(timeBuffer));
}

long getUnixTimestamp() {
  time_t now = time(nullptr);
  if (now < 8 * 3600 * 2) {
    Serial.println("❌ Time not yet synchronized or invalid. Returning 0.");
    return 0;
  }
  return now;
}