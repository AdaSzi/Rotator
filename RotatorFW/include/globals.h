#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include "rotator.h"

#define LED_PIN 8
#define BUTTON_PIN 9

#define ROTATOR_POT_PIN A2

#define BUTTON_HOLD_RESET_TIME 10000

#define WEBSOCKET_UPDATE_INTERVAL 1000

struct GlobalData {
    uint16_t currentAzimuth = 0;
    uint16_t targetAzimuth = 0;
    uint8_t currentSpeed = 0;
    bool isMoving = false;

    bool operator == (const GlobalData& other) const {
        return currentAzimuth == other.currentAzimuth &&
               targetAzimuth == other.targetAzimuth &&
               currentSpeed == other.currentSpeed &&
               isMoving == other.isMoving;
    }

    bool operator != (const GlobalData& other) const {
        return !(*this == other);
    }
};

extern GlobalData globalData;
extern StaticJsonDocument<8192> mainConfigDoc;
extern char sharedConfigJson[8192];

extern Rotator rotator;

void ledBlink(uint16_t period);
void restart();

#endif