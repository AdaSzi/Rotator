#ifndef ROTATOR_H
#define ROTATOR_H
#include <Arduino.h>


class Pot {
    public:
        Pot(uint8_t pin);

        void handlePot();
        float getPosition();

    private:
        uint8_t pin;
        float position;
        float lowPassFilter(uint16_t inputValue);
};

class Motor {
    public:
        Motor();

        void left(uint8_t speed);
        void right(uint8_t speed);
        void stop(uint8_t speed);
};

class Rotator {
    private:
        Motor motor;
        Pot pot;

    public:
        Rotator(uint8_t potPin);

        void handleRotator();
        void setTargetPosition(int target);
        bool isMoving();
};
#endif
