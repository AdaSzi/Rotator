#include "rotator.h"
#include "Arduino.h"
#include "globals.h"
#include "storage.h"

Pot::Pot(uint8_t potPin) {
    this->potPin = potPin;
    this->potMin = 0;
    this->potMax = 0;

    //0V - 2.9V -> 0 - 4096
    pinMode(this->potPin, INPUT);
    

}

void Pot::initPot(uint16_t potMin, uint16_t potMax){
    this->potMin = potMin;
    this->potMax = potMax;
    initFilter();
}

void Pot::handlePot() {
    static unsigned long nextUpdate = 0;
    if (millis() > nextUpdate) {
        nextUpdate = millis() + 50;

        this->positionRaw = lowPassFilter(analogRead(potPin), 0.1);

        if(this->potMin == this->potMax) return; //the pot is not calibrated
        this->position = map(this->positionRaw, this->potMin, this->potMax, 0, 360 + 90);

        #ifdef DEBUG
            //Serial.println(globalData.currentAzimuth);
        #endif
    }
}

void Pot::initFilter() {
    this->positionRaw = lowPassFilter(analogRead(potPin), 1);
    this->position = map(this->positionRaw, this->potMin, this->potMax, 0, 360 + 90);
}

float Pot::lowPassFilter(uint16_t inputValue, float coefficient) {
    filterValue = coefficient * inputValue + (1 - coefficient) * filterValue;
    return filterValue;
}

float Pot::getPosition() {
    return this->position;
}

float Pot::getPositionRaw() {
    return this->positionRaw;
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
    
    this->mode = NORMAL;

    this->rotatorCurrentPosition = rotatorCurrentPositionInput;
    this->rotatorSetpoint = rotatorSetpoint;

    this->controllerInput = pot.getPosition();
    *this->rotatorCurrentPosition = pot.getPosition();

    this->controllerSetpoint = pot.getPosition();
    *this->rotatorSetpoint = pot.getPosition();

    this->pidController.SetOutputLimits(-255, 255);
    this->pidController.SetMode(MANUAL);
}

void Rotator::initRotator(uint16_t rotatorAzimuthOffset, uint16_t rotatorAngleRange, uint16_t potMin, uint16_t potMax){
    this->rotatorAzimuthOffset = rotatorAzimuthOffset;
    this->rotatorAngleRange = rotatorAngleRange;
    pot.initPot(potMin, potMax);
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

    if(this->mode == CALIBRATION) calibrationMode();
}

void Rotator::calibrate(){
    this->mode = CALIBRATION;
    this->calibrationStep = 0;
    this->pidController.SetMode(MANUAL);

    #ifdef DEBUG
        Serial.println("Entering calibration mode");
    #endif
}

void Rotator::calibrationMode(){
    static unsigned long nextUpdate = 0;
    static uint16_t minPotPosRaw, maxPotPosRaw;
    if(calibrationStep == 0) this->motor.left(255);
    if(calibrationStep == 1) this->motor.right(255);
    if(calibrationStep == 2) this->motor.stop();

    if (millis() > nextUpdate) {   
        nextUpdate = millis() + 1100;

        if(calibrationStep == 0){
            static uint16_t lastPositionMin = 65535;

            if(lastPositionMin != (uint16_t)pot.getPositionRaw()){
                lastPositionMin = pot.getPositionRaw();
            }
            else {
                #ifdef DEBUG
                    Serial.print("Reached 0 position: ");
                    Serial.println(lastPositionMin);
                #endif                
                minPotPosRaw = lastPositionMin;
                calibrationStep++;
            }
        }

        else if(calibrationStep == 1){
            static uint16_t lastPositionMax = 65535;

            if(lastPositionMax != (uint16_t)pot.getPositionRaw()){
                lastPositionMax = pot.getPositionRaw();
            }
            else {
                #ifdef DEBUG
                    Serial.print("Reached 100% position: ");
                    Serial.println(lastPositionMax);
                #endif                
                maxPotPosRaw = lastPositionMax;
                calibrationStep++;
            }
        }

        else if(calibrationStep == 2){            
            #ifdef DEBUG
                Serial.println("End-stops calibration end: ");
                Serial.println(minPotPosRaw);
                Serial.println(maxPotPosRaw);
            #endif

            mainConfigDoc["settings"]["potMin"] = minPotPosRaw;            
            mainConfigDoc["settings"]["potMax"] = maxPotPosRaw;  
            pot.initPot(minPotPosRaw, maxPotPosRaw); 
            
            saveConfig(mainConfigDoc, "/config.json");
            serializeJson(mainConfigDoc, mainConfigDocString);

            this->setTargetPosition(360);

            this->mode = NORMAL;
            this->pidController.SetMode(AUTOMATIC);
        }
    }
}

inline uint16_t Rotator::rotatorAzimuthApplyOffset(uint16_t input) {
	// sensed 0 - max
	// out 0 - 359
	return (this->rotatorAzimuthOffset + input) % 360;
}

void Rotator::setTargetPosition(uint16_t newTarget) {
	// target 0 - 359
	newTarget += 360;
    
	uint16_t sourceA = rotatorAzimuthApplyOffset(*this->rotatorCurrentPosition);
	int16_t diff = newTarget - sourceA;
	diff = (diff + 180) % 360 - 180;

	int16_t finalPos = *this->rotatorCurrentPosition + diff;

	if (finalPos < 0) {
		diff += 360;
	}

	if (finalPos > this->rotatorAngleRange) {
		diff -= 360;
	}


    *this->rotatorSetpoint = diff;
    this->controllerSetpoint = diff;

    #ifdef DEBUG
        Serial.print("Target set to: ");
        Serial.println(*this->rotatorSetpoint);
    #endif
}