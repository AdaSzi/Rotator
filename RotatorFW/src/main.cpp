#include <Arduino.h>
#include <ArduinoJson.h>
#include "main.h"
#include "globals.h"
#include "rotator.h"
#include "storage.h"
#include "wifiHandler.h"
#include "webserverHandler.h"


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(250);
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(250);

  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Starting Omega rotator controller");
  #endif
  
  if(!initStorage()){
    setupMode();
  }
  initWifi();

  initWebServer();

}

void loop() {
  rotator.handleRotator();
  handleWifi();
  handleWebServer();
  checkButton();
  
  ledBlink(500);
}

void checkButton() {
  static unsigned long buttonStartTime = 0;
  static bool buttonHeld = false;

  if (!digitalRead(BUTTON_PIN)) {
    if (!buttonHeld) {
      buttonStartTime = millis();
      buttonHeld = true;
    }
    else {
      unsigned long currentTime = millis();
      if (currentTime - buttonStartTime >= BUTTON_HOLD_RESET_TIME) {
        deleteConfig("/config.json");
        deleteConfig("/WiFiConfig.json");
        restart();
        buttonHeld = false;
      }
    }
  }
  else buttonHeld = false;
}