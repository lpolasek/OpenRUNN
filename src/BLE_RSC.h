#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

class BLE_RSC {
public:
    BLE_RSC(const char* deviceName = "OpenRUNN");
    void setup();
    bool isConnected() const;
    void notifyRscMeasurement(float speedMetersPerSecond, uint8_t cadence, float totalDistanceMeters);

private:
    class ServerCallbacks : public BLEServerCallbacks {
    public:
        ServerCallbacks(BLE_RSC& parent) : _bleRsc(parent) {}
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    private:
        BLE_RSC& _bleRsc;
    };

    const char* _deviceName;
    BLEServer* _pServer = nullptr;
    ServerCallbacks _serverCallbacks;
    BLECharacteristic* _pRscMeasurement = nullptr;
    volatile bool _deviceConnected = false;
    volatile unsigned long _bleConnectionStartMs = 0;

};
