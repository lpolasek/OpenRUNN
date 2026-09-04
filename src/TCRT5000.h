#pragma once

#include "Arduino.h"

struct TCRT5000Data {
    uint64_t totalPulses;
    bool hasPulse;
    unsigned long lastPulseMs;
    unsigned long lastPulseIntervalMs;
};

class TCRT5000
{
public:
    TCRT5000(uint8_t pin, const uint64_t bounceTimeMs);
    void setup();
    TCRT5000Data getData();

private:
    uint8_t _pin;
    const uint64_t _bounceTimeMs;
    volatile uint64_t totalPulses = 0;
    volatile bool _hasPulse = false;
    volatile unsigned long lastPulseMs = 0;
    unsigned long lastPulseIntervalMs = 0;

    portMUX_TYPE _spinlock = portMUX_INITIALIZER_UNLOCKED;

    static void ARDUINO_ISR_ATTR handlerWrapper(void* arg);
    void ARDUINO_ISR_ATTR onSensorPulseRising();
};