#include "globals.h"
#include <ArduinoJson.h>

GlobalData globalData;
StaticJsonDocument<8192> mainConfigDoc;

Rotator rotator = Rotator(ROTATOR_POT_PIN);

void ledBlink(uint16_t period){
  static unsigned long nextLEDUpdate = 0;
  if (millis() > nextLEDUpdate) {
    nextLEDUpdate = millis() + period;
    digitalWrite(LED_PIN, !digitalRead(8));
  }
}

void restart(){
  #ifdef DEBUG
    Serial.println("\nRestarting...");
  #endif
  delay(1000);
  ESP.restart();
}