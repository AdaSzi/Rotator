#include "globals.h"
#include <ArduinoJson.h>

GlobalData globalData;
StaticJsonDocument<8192> mainConfigDoc;
char sharedConfigJson[8192];

Rotator rotator = Rotator(ROTATOR_POT_PIN, ROTATOR_MOTOR_PWM_PIN, ROTATOR_MOTOR_CW_PIN, ROTATOR_MOTOR_CCW_PIN, &globalData.currentAzimuth, &globalData.currentSpeed, &globalData.targetAzimuth);

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