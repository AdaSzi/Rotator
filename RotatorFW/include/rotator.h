#ifndef ROTATOR_H
#define ROTATOR_H
#include <Arduino.h>


class Pot {
public:
    uint8_t pin;
    Pot(uint8_t pin);

    uint16_t getPosition();
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