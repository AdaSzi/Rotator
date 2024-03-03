#ifndef ROTATOR_H
#define ROTATOR_H
#include <Arduino.h>
#include <PID_v1.h>


class Pot {
    public:
        Pot(uint8_t potPin);

        void handlePot();
        float getPosition();
        
    private:
        uint8_t potPin;
        float position, filterValue;

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
        enum Color {STOP, RIGHT, LEFT};
        uint8_t status;
        unsigned long nextEnableTime = 0;
        uint8_t pwmPin, cwPin, ccwPin;
};

class Rotator {
    private:
        Motor motor;
        Pot pot;
        PID pidController;

        //Pid vars
        double controllerInput, controllerOutput, controllerSetpoint;
        
        //master vars
        uint16_t *rotatorCurrentPosition, *rotatorSetpoint;

    public:
        Rotator(uint8_t potPin, uint8_t pwmMotPin, uint8_t cwMotPin, uint8_t ccwMotPin, uint16_t* Input, uint16_t* Output, uint16_t* Setpoint);

        void handleRotator();
        void setTargetPosition(uint16_t target);
        bool isMoving();
};
#endif
