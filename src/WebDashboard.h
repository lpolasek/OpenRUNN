#pragma once

#include <Arduino.h>
#include <WebServer.h>

class WebDashboard {
public:
    explicit WebDashboard(const char* accessPointName = "OpenRUNN");

    void setup();
    void loop();
    void updateMetrics(float speedMetersPerSecond, float distanceMeters);

private:
    void sendIndex();
    void sendStyles();
    void sendScript();
    void sendStatus();

    const char* _accessPointName;
    WebServer _server;
    float _speedMetersPerSecond = 0.0f;
    float _distanceMeters = 0.0f;
};