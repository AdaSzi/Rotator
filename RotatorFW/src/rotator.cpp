#include "rotator.h"
#include "Arduino.h"
#include "globals.h"

#include "driver/adc.h"

Pot::Pot(uint8_t potPin) {
    pin = potPin;
    

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);

    pinMode(pin, INPUT);
}

uint16_t Pot::getPosition() {
    //return analogRead(pin);
    return adc1_get_raw((adc1_channel_t)3);
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
        //Serial.println(globalData.currentAzimuth);
    }
}

void Rotator::setTargetPosition(int target) {
    
}