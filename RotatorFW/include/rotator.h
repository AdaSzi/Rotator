#ifndef ROTATOR_H
#define ROTATOR_H
#include <Arduino.h>
#include <PID_v1.h>


class Pot {
    public:
        Pot(uint8_t potPin);

        void initPot(uint16_t potMin, uint16_t potMax);
        void handlePot();
        float getPosition();
        float getPositionRaw();
        
    private:
        uint8_t potPin;
        float position, positionRaw, filterValue, potMin, potMax;

        void initFilter();
        float lowPassFilter(uint16_t inputValue, float coefficient);
};

class Motor {
    public:
        Motor(uint8_t pwmPin, uint8_t cwPin, uint8_t ccwPin, uint16_t* speedOutput);

        void handleMotor();

        void left(uint8_t speed);
        void right(uint8_t speed);
        void stop();

    private:
        uint16_t* speedOutput;
        enum Status {STOP, RIGHT, LEFT};
        uint8_t status;
        unsigned long nextEnableTime = 0;
        uint8_t pwmPin, cwPin, ccwPin;
};

class Rotator {
    public:
        Rotator(uint8_t potPin, uint8_t pwmMotPin, uint8_t cwMotPin, uint8_t ccwMotPin, uint16_t* Input, uint16_t* Output, uint16_t* Setpoint);

        void initRotator(uint16_t potMin, uint16_t potMax);
        void handleRotator();
        void setTargetPosition(uint16_t target);
        void calibrate();

    private:
        Motor motor;
        Pot pot;
        PID pidController;

        //Pid vars
        double controllerInput, controllerOutput, controllerSetpoint;
        
        //master vars
        uint16_t *rotatorCurrentPosition, *rotatorSetpoint;

        
        enum Mode {CALIBRATION, NORMAL};
        uint8_t mode, calibrationStep;
        void calibrationMode();

};
#endif
