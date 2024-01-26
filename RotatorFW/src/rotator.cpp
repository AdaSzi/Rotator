#include "rotator.h"
#include "Arduino.h"
#include "globals.h"


Pot::Pot(uint8_t potPin) {
    pin = potPin;

    pinMode(pin, INPUT);
}

uint16_t Pot::getPosition() {
    return 0; //analogRead(pin);
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

}

void Rotator::handleRotator(){
    static unsigned long nextUpdate = 0;
    if (millis() > nextUpdate) {    
        nextUpdate = millis() + 100;

        globalData.currentAzimuth+=5;
        if(globalData.currentAzimuth >= 360) globalData.currentAzimuth = 0;
        //globalData.currentAzimuth = pot.getPosition();
        #ifdef DEBUG
            //Serial.println(globalData.currentAzimuth);
        #endif
    }
}

void Rotator::setTargetPosition(int target) {
    
}