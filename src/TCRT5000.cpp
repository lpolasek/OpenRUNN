#include "TCRT5000.h"

TCRT5000::TCRT5000(uint8_t pin,const uint64_t bounceTimeMs) : _pin(pin), _bounceTimeMs(bounceTimeMs) {
    totalPulses = 0;
    lastPulseMs = 0;
    lastPulseIntervalMs = 0;
}

void TCRT5000::setup() {
  pinMode(_pin, INPUT_PULLUP);
  attachInterruptArg(digitalPinToInterrupt(_pin), TCRT5000::handlerWrapper, this, FALLING);
}

TCRT5000Data TCRT5000::getData() {
    TCRT5000Data data;
    portENTER_CRITICAL(&_spinlock);
    data.totalPulses = totalPulses;
    data.hasPulse = _hasPulse;
    data.lastPulseMs = lastPulseMs;
    data.lastPulseIntervalMs = lastPulseIntervalMs;
    portEXIT_CRITICAL(&_spinlock);
    return data;
}

void ARDUINO_ISR_ATTR TCRT5000::handlerWrapper(void* arg) {
    TCRT5000* instance = static_cast<TCRT5000*>(arg);
    instance->onSensorPulseRising();
}

void ARDUINO_ISR_ATTR TCRT5000::onSensorPulseRising() {
    unsigned long nowMs = millis();

    portENTER_CRITICAL_ISR(&_spinlock);
    if ((nowMs - lastPulseMs) >= _bounceTimeMs) {
        lastPulseIntervalMs = _hasPulse ? nowMs - lastPulseMs : 0;
        ++totalPulses;
        lastPulseMs = nowMs;
        _hasPulse = true;
    }
    portEXIT_CRITICAL_ISR(&_spinlock);
}