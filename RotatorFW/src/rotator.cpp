#include "rotator.h"
#include "Arduino.h"


Pot::Pot(uint8_t potPin) {
    this->potPin = potPin;

    //0V - 2.9V -> 0 - 4096
    pinMode(this->potPin, INPUT);
    

    initFilter();
}

void Pot::handlePot() {
    static unsigned long nextUpdate = 0;
    if (millis() > nextUpdate) {   
        nextUpdate = millis() + 50;

        this->position = map(lowPassFilter(analogRead(potPin), 0.1), 0, 4096, 0, 360 + 90);

        #ifdef DEBUG
            //Serial.println(globalData.currentAzimuth);
        #endif
    }
}

void Pot::initFilter() {
    this->position = map(lowPassFilter(analogRead(potPin), 1), 0, 4096, 0, 360 + 90);
}

float Pot::lowPassFilter(uint16_t inputValue, float coefficient) {
    filterValue = coefficient * inputValue + (1 - coefficient) * filterValue;
    return filterValue;
}

float Pot::getPosition() {
    return position;
}


//https://esp32.com/viewtopic.php?t=27518
Motor::Motor(uint8_t pwmPin, uint8_t cwPin, uint8_t ccwPin, uint16_t* speedOutput) {
    this->pwmPin = pwmPin;
    this->cwPin = cwPin;
    this->ccwPin = ccwPin;
    this->speedOutput = speedOutput;

    pinMode(this->cwPin, OUTPUT);
    pinMode(this->ccwPin, OUTPUT);
    pinMode(this->pwmPin, OUTPUT);
    stop();

    #ifdef DEBUG
        //Serial.println("Motor started");
    #endif
}

void Motor::handleMotor(){

}

void Motor::right(uint8_t speed) {
    if(this->status != LEFT && this->nextEnableTime < millis()){
        digitalWrite(this->ccwPin, LOW);

        digitalWrite(this->cwPin, HIGH);
        analogWrite(this->pwmPin, speed);

        this->status = RIGHT;
        *this->speedOutput = speed;

        #ifdef DEBUG
            //Serial.print("Motor R, speed: ");
            //Serial.println(speed);
        #endif
    }
    else stop();
}

void Motor::left(uint8_t speed) {
    if(this->status != RIGHT && this->nextEnableTime < millis()){
        digitalWrite(this->cwPin, LOW);

        digitalWrite(this->ccwPin, HIGH);
        analogWrite(this->pwmPin, speed);

        this->status = LEFT;
        *this->speedOutput = speed;

        #ifdef DEBUG
            //Serial.print("Motor L, speed: ");
            //Serial.println(speed);
        #endif  
    }
    else stop();
}

void Motor::stop() {
    if(this->status != STOP) {
        this->nextEnableTime = millis() + 1000;
    }
    digitalWrite(this->cwPin, LOW);
    digitalWrite(this->ccwPin, LOW);
    digitalWrite(this->pwmPin, LOW);

    this->status = STOP;
    *this->speedOutput = 0;

    #ifdef DEBUG
        //Serial.println("Motor stopped");
    #endif
}



Rotator::Rotator(uint8_t potPin, uint8_t pwmMotPin, uint8_t cwMotPin, uint8_t ccwMotPin, uint16_t* rotatorCurrentPositionInput, uint16_t* Output, uint16_t* rotatorSetpoint): 
    pot(potPin), 
    motor(pwmMotPin, cwMotPin, ccwMotPin, Output),
    pidController(&this->controllerInput, &this->controllerOutput, &this->controllerSetpoint, 50, 20, 0, P_ON_M, DIRECT) {

    this->rotatorCurrentPosition = rotatorCurrentPositionInput;
    this->rotatorSetpoint = rotatorSetpoint;

    this->controllerInput = pot.getPosition();
    *this->rotatorCurrentPosition = pot.getPosition();

    this->controllerSetpoint = pot.getPosition();
    *this->rotatorSetpoint = pot.getPosition();

    this->pidController.SetOutputLimits(-255, 255);
    this->pidController.SetMode(AUTOMATIC);
}

void Rotator::handleRotator(){   
    this->pot.handlePot();
    *this->rotatorCurrentPosition = this->pot.getPosition();
    this->controllerInput = this->pot.getPosition();

    if(this->pidController.Compute()){
        if(abs(this->controllerOutput) < 100) {
            this->motor.stop();
        } 
        else if(abs(*this->rotatorCurrentPosition - *this->rotatorSetpoint) > 1){
            if(0 < this->controllerOutput) {
                this->motor.right(abs(this->controllerOutput));
            }
            else if(this->controllerOutput < 0){
                this->motor.left(abs(this->controllerOutput));
            }
        }

        #ifdef DEBUG
            /*Serial.println("Controller data: ");
            Serial.print(this->controllerInput);
            Serial.print(", ");
            Serial.print(this->controllerOutput);
            Serial.print(", ");
            Serial.println(this->controllerSetpoint);*/
        #endif
    }
}

void Rotator::setTargetPosition(uint16_t target) {
    //if(*rotatorSetpoint == target && controllerSetpoint == target) return;
    
    uint16_t altTarget = (target + 360)%450;    
    if(abs(*this->rotatorCurrentPosition - target) > abs(*this->rotatorCurrentPosition - altTarget)){
    	target = altTarget;
    }

    *this->rotatorSetpoint = target;
    this->controllerSetpoint = target;

    
    #ifdef DEBUG
        Serial.print("Target set to: ");
        Serial.println(*this->rotatorSetpoint);
    #endif

}
