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

        position = map(lowPassFilter(analogRead(pin), 0.01), 0, 4096, 0, 360 + 90);

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



Motor::Motor(uint8_t pwm, uint8_t cw, uint8_t ccw) {
    pwmPin = pwm;
    cwPin = cw;
    ccwPin = ccw;

    pinMode(cwPin, OUTPUT);
    pinMode(ccwPin, OUTPUT);
    pinMode(pwmPin, OUTPUT);
    digitalWrite(cwPin, LOW);
    digitalWrite(ccwPin, LOW);
    digitalWrite(pwmPin, LOW);

    #ifdef DEBUG
        Serial.println("Motor started: ");
    #endif
}

void Motor::handleMotor(){

}

void Motor::right(uint8_t speed) {
    digitalWrite(ccwPin, LOW);

    digitalWrite(cwPin, HIGH);
    analogWrite(pwmPin, speed);
}

void Motor::left(uint8_t speed) {
    digitalWrite(cwPin, LOW);

    digitalWrite(ccwPin, HIGH);
    analogWrite(pwmPin, speed);    
}

void Motor::stop(uint8_t speed) {
    digitalWrite(cwPin, LOW);
    digitalWrite(ccwPin, LOW);
    digitalWrite(pwmPin, LOW);  
}



Rotator::Rotator(uint8_t potPin, uint8_t pwmMotPin, uint8_t cwMotPin, uint8_t ccwMotPin, uint16_t* Input, uint16_t* Output, uint16_t* Setpoint): 
    pot(potPin), motor(pwmMotPin, cwMotPin, ccwMotPin), pidController(&controllerInput, &controllerOutput, &controllerSetpoint, 2, 5, 1, P_ON_M, DIRECT) {

    rotatorCurrentPosition = Input;
    rotatorMotorSpeed = Output;
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
        *rotatorMotorSpeed = controllerOutput;

        #ifdef DEBUG
            Serial.println("Controller data: ");
            Serial.print(controllerInput);
            Serial.print(", ");
            Serial.print(controllerOutput);
            Serial.print(", ");
            Serial.println(controllerSetpoint);
        #endif
    }

    motor.handleMotor();
}

void Rotator::setTargetPosition(uint16_t target) {
    //TODO select closest target

    /*int16_t positiveDiff = target - currentPosition;
    int16_t negativeDiff = currentPosition - target;

    // Find the closest target
    if (positiveDiff < negativeDiff) {
        // Positive difference is smaller, set target directly
        setpoint = target;
    } else {
        // Negative difference is smaller or equal, set the opposite direction target
        setpoint = currentPosition - negativeDiff;
    }

    // Ensure setpoint is within the valid range (0 to 450)
    setpoint = constrain(setpoint, 0, 450);*/

    *rotatorTargetPosition = target;
    controllerSetpoint = target;

    
    #ifdef DEBUG
        Serial.print("Target set to: ");
        Serial.println(*rotatorTargetPosition);
    #endif

}
