#include "rotator.h"
#include "Arduino.h"


Pot::Pot(uint8_t potPin) {
    pin = potPin;

    //0V - 2.9V -> 0 - 4096
    pinMode(pin, INPUT);
    

    initFilter();
}

void Pot::handlePot() {
    static unsigned long nextUpdate = 0;
    if (millis() > nextUpdate) {   
        nextUpdate = millis() + 50;

        position = map(lowPassFilter(analogRead(pin), 0.1), 0, 4096, 0, 360 + 90);

        #ifdef DEBUG
            //Serial.println(globalData.currentAzimuth);
        #endif
    }
}

void Pot::initFilter() {
    position = map(lowPassFilter(analogRead(pin), 1), 0, 4096, 0, 360 + 90);
}

float Pot::lowPassFilter(uint16_t inputValue, float coefficient) {
    filterValue = coefficient * inputValue + (1 - coefficient) * filterValue;
    return filterValue;
}

float Pot::getPosition() {
    return position;
}


//https://esp32.com/viewtopic.php?t=27518
Motor::Motor(uint8_t pwm, uint8_t cw, uint8_t ccw, uint16_t* output) {
    pwmPin = pwm;
    cwPin = cw;
    ccwPin = ccw;
    speedOutput = output;

    pinMode(cwPin, OUTPUT);
    pinMode(ccwPin, OUTPUT);
    pinMode(pwmPin, OUTPUT);
    stop();

    #ifdef DEBUG
        Serial.println("Motor started: ");
    #endif
}

void Motor::handleMotor(){

}

void Motor::right(uint8_t speed) {
    if(status != LEFT && nextEnableTime < millis()){
        digitalWrite(ccwPin, LOW);

        digitalWrite(cwPin, HIGH);
        analogWrite(pwmPin, speed);

        status = RIGHT;
        *speedOutput = speed;

        #ifdef DEBUG
            Serial.print("Motor R, speed: ");
            Serial.println(speed);
        #endif
    }
    else stop();
}

void Motor::left(uint8_t speed) {
    if(status != RIGHT && nextEnableTime < millis()){
        digitalWrite(cwPin, LOW);

        digitalWrite(ccwPin, HIGH);
        analogWrite(pwmPin, speed);

        status = LEFT;
        *speedOutput = speed;

        #ifdef DEBUG
            Serial.print("Motor L, speed: ");
            Serial.println(speed);
        #endif  
    }
    else stop();
}

void Motor::stop() {
    if(status != STOP) {
        nextEnableTime = millis() + 1000;
    }
    digitalWrite(cwPin, LOW);
    digitalWrite(ccwPin, LOW);
    digitalWrite(pwmPin, LOW);

    status = STOP;
    *speedOutput = 0;

    #ifdef DEBUG
        Serial.println("Motor stopped");
    #endif
}



Rotator::Rotator(uint8_t potPin, uint8_t pwmMotPin, uint8_t cwMotPin, uint8_t ccwMotPin, uint16_t* Input, uint16_t* Output, uint16_t* Setpoint): 
    pot(potPin), 
    motor(pwmMotPin, cwMotPin, ccwMotPin, Output),
    pidController(&controllerInput, &controllerOutput, &controllerSetpoint, 50, 20, 0, P_ON_M, DIRECT) {

    rotatorCurrentPosition = Input;
    rotatorTargetPosition = Setpoint;

    controllerInput = pot.getPosition();
    *rotatorCurrentPosition = pot.getPosition();

    controllerSetpoint = pot.getPosition();
    *rotatorTargetPosition = pot.getPosition();

    pidController.SetOutputLimits(-255, 255);
    pidController.SetMode(AUTOMATIC);
}

void Rotator::handleRotator(){   
    pot.handlePot();
    *rotatorCurrentPosition = pot.getPosition();
    controllerInput = pot.getPosition();

    if(pidController.Compute()){
        if(abs(controllerOutput) < 100) {
            motor.stop();
        } 
        else if(abs(*rotatorCurrentPosition - *rotatorTargetPosition) > 1){
            if(0 < controllerOutput) {
                motor.right(abs(controllerOutput));
            }
            else if(controllerOutput < 0){
                motor.left(abs(controllerOutput));
            }
        }

        #ifdef DEBUG
            Serial.println("Controller data: ");
            Serial.print(controllerInput);
            Serial.print(", ");
            Serial.print(controllerOutput);
            Serial.print(", ");
            Serial.println(controllerSetpoint);
        #endif
    }
}

void Rotator::setTargetPosition(uint16_t target) {
    if(*rotatorTargetPosition == target && controllerSetpoint == target) 
        return;
    
    uint16_t altTarget = (target + 360)%450;
    
    if(abs(*rotatorCurrentPosition - target) > abs(*rotatorCurrentPosition - altTarget)){
    	target = altTarget;
    }

    *rotatorTargetPosition = target;
    controllerSetpoint = target;

    
    #ifdef DEBUG
        Serial.print("Target set to: ");
        Serial.println(*rotatorTargetPosition);
    #endif

}
