#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "BLE_RSC.h"

// Standard BLE UUIDs for Running Speed and Cadence
#define SERVICE_UUID_RSC           "1814"
#define CHARACTERISTIC_UUID_RSC_M  "2A53" // Measurement (Notify)
#define CHARACTERISTIC_UUID_RSC_F  "2A54" // Feature (Read)

#define FEATURE_TOTAL_DISTANCE_MEASUREMENT_SUPPORTED 0x02
#define FLAG_TOTAL_DISTANCE_PRESENT                 0x02

BLE_RSC::BLE_RSC(const char* deviceName)
    : _deviceName(deviceName), _serverCallbacks(*this) {
}

void BLE_RSC::ServerCallbacks::onConnect(BLEServer* pServer) {
    _bleRsc._deviceConnected = true;
    _bleRsc._bleConnectionStartMs = millis();
    Serial.printf("BLE RSC connected at %lu ms\n", _bleRsc._bleConnectionStartMs);
}

void BLE_RSC::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    unsigned long nowMs = millis();
    unsigned long connectedForMs = nowMs - _bleRsc._bleConnectionStartMs;
    _bleRsc._deviceConnected = false;
    pServer->startAdvertising(); // restart advertising
    Serial.printf(
        "BLE RSC disconnected at %lu ms after %lu ms; advertising restarted\n",
        nowMs,
        connectedForMs
    );
}

void BLE_RSC::setup() {
    Serial.println("Initializing BLE...");
    BLEDevice::init(_deviceName);
    Serial.println("BLE initialized.");

    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(&_serverCallbacks);

    BLEService *pRscService = _pServer->createService(SERVICE_UUID_RSC);

    _pRscMeasurement = pRscService->createCharacteristic(
        CHARACTERISTIC_UUID_RSC_M,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    _pRscMeasurement->addDescriptor(new BLE2902());

    BLECharacteristic *pRscFeature = pRscService->createCharacteristic(
        CHARACTERISTIC_UUID_RSC_F,
        BLECharacteristic::PROPERTY_READ
    );

    uint16_t featureFlags = FEATURE_TOTAL_DISTANCE_MEASUREMENT_SUPPORTED;
    pRscFeature->setValue((uint8_t*)&featureFlags, sizeof(featureFlags));

    pRscService->start();

    Serial.println("Starting BLE advertising...");
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID_RSC);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // helps with iPhone connections
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("BLE RSC Service is now advertising...");
}

bool BLE_RSC::isConnected() const {
    return _deviceConnected;
}

void BLE_RSC::notifyRscMeasurement(float speedMetersPerSecond, uint8_t cadence, float totalDistanceMeters) {
    if (!_deviceConnected || !_pRscMeasurement) {
        return;
    }

    uint32_t distanceMeters = (uint32_t)totalDistanceMeters;
    uint16_t speed = (uint16_t)((speedMetersPerSecond * 256.0f) + 0.5f);

    uint8_t rscBuffer[8] = {
        FLAG_TOTAL_DISTANCE_PRESENT,
        (uint8_t)(speed & 0xFF),
        (uint8_t)((speed >> 8) & 0xFF),
        cadence
    };
    for (int index = 0; index < 4; index++) {
        rscBuffer[4 + index] = (uint8_t)((distanceMeters >> (8 * index)) & 0xFF);
    }

    _pRscMeasurement->setValue(rscBuffer, sizeof(rscBuffer));
    _pRscMeasurement->notify();
    Serial.printf(
        "RSC tx: time=%.2fs speed=%u cadence=%u distance=%lu m\n",
        millis() / 1000.0f,
        speed,
        cadence,
        distanceMeters
    );
}