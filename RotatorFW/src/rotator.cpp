#include "rotator.h"
#include "Arduino.h"
#include "globals.h"

Pot::Pot(uint8_t potPin) {
    pin = potPin;

    pinMode(pin, INPUT);
    
    //0V - 2.9V -> 0 - 4096
    position = map(analogRead(pin), 0, 4096, 0, 360 + 90);
}

void Pot::handlePot() {
    position = map(lowPassFilter(analogRead(pin)), 0, 4096, 0, 360 + 90);
}

float Pot::lowPassFilter(uint16_t inputValue) {
    const float alpha = 0.01;  // Filter coefficient (adjust as needed)
    static float filteredValue = 0;

    filteredValue = alpha * inputValue + (1 - alpha) * filteredValue;

    return filteredValue;
}


float Pot::getPosition() {
    return position;
}



Motor::Motor() {
    
}

void Motor::left(uint8_t speed) {
    
}

void Motor::right(uint8_t speed) {
    
}

void Motor::stop(uint8_t speed) {
    
}

Rotator::Rotator(uint8_t potPin) : pot(potPin), motor() {
    globalData.targetAzimuth = pot.getPosition();
}

void Rotator::handleRotator(){
    pot.handlePot();

    static unsigned long nextUpdate = 0;
    if (millis() > nextUpdate) {   
        nextUpdate = millis() + 50;
        globalData.currentAzimuth = pot.getPosition();

        #ifdef DEBUG
            //Serial.println(globalData.currentAzimuth);
        #endif
    }
}

void Rotator::setTargetPosition(int target) {
    
}
