#include "WebDashboard.h"

#include <WiFi.h>

#include "WebAssets.h"

WebDashboard::WebDashboard(const char* accessPointName)
    : _accessPointName(accessPointName), _server(80) {
}

void WebDashboard::setup() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(_accessPointName)) {
        Serial.println("Failed to start Wi-Fi access point.");
        return;
    }

    _server.on("/", HTTP_GET, [this]() { sendIndex(); });
    _server.on("/styles.css", HTTP_GET, [this]() { sendStyles(); });
    _server.on("/app.js", HTTP_GET, [this]() { sendScript(); });
    _server.on("/api/status", HTTP_GET, [this]() { sendStatus(); });
    _server.onNotFound([this]() { _server.send(404, "text/plain", "Not found"); });
    _server.begin();

    Serial.printf("Wi-Fi AP '%s' started at http://%s\n", _accessPointName, WiFi.softAPIP().toString().c_str());
}

void WebDashboard::loop() {
    _server.handleClient();
}

void WebDashboard::updateMetrics(float speedMetersPerSecond, float distanceMeters) {
    _speedMetersPerSecond = speedMetersPerSecond;
    _distanceMeters = distanceMeters;
}

void WebDashboard::sendIndex() {
    _server.sendHeader("Content-Encoding", "gzip");
    _server.sendHeader("Cache-Control", "public, max-age=86400");
    _server.send_P(
        200,
        "text/html; charset=utf-8",
        reinterpret_cast<PGM_P>(WebAssets::INDEX_HTML_GZ),
        WebAssets::INDEX_HTML_GZ_LEN
    );
}

void WebDashboard::sendStyles() {
    _server.sendHeader("Content-Encoding", "gzip");
    _server.sendHeader("Cache-Control", "public, max-age=86400");
    _server.send_P(
        200,
        "text/css; charset=utf-8",
        reinterpret_cast<PGM_P>(WebAssets::STYLES_CSS_GZ),
        WebAssets::STYLES_CSS_GZ_LEN
    );
}

void WebDashboard::sendScript() {
    _server.sendHeader("Content-Encoding", "gzip");
    _server.sendHeader("Cache-Control", "public, max-age=86400");
    _server.send_P(
        200,
        "application/javascript; charset=utf-8",
        reinterpret_cast<PGM_P>(WebAssets::APP_JS_GZ),
        WebAssets::APP_JS_GZ_LEN
    );
}

void WebDashboard::sendStatus() {
    const float speedKph = _speedMetersPerSecond * 3.6f;
    const float paceSecondsPerKm = _speedMetersPerSecond > 0.0f
        ? 1000.0f / _speedMetersPerSecond
        : 0.0f;
    char response[128];
    snprintf(
        response,
        sizeof(response),
        "{\"distanceMeters\":%.2f,\"speedKph\":%.2f,\"paceSecondsPerKm\":%.1f}",
        _distanceMeters,
        speedKph,
        paceSecondsPerKm
    );

    _server.sendHeader("Cache-Control", "no-store");
    _server.send(200, "application/json", response);
}