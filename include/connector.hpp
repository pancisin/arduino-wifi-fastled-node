#ifndef Connector_H
#define Connector_H

#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <WiFiNINA.h>

class Connector {
private:
    Connector();

    static Connector *instancePtr;

    uint8_t status = WL_IDLE_STATUS;
    WiFiClient wifiClient;
    MqttClient mqttClient = nullptr;
    unsigned long lastWiFiCheck = 0;
    unsigned long lastHbMillis = 0;
    int hbValue = 0;

    // WiFi
    const char *wifiSSID{};
    const char *wifiPassword{};

    void handleHeartbeat();

    // message handling
    void (*callback)(JsonDocument doc){};

    static void handleMessage(int);

    JsonDocument doc;
    char topic[50]{};
    char hbTopic[50]{};

    // MQTT connection parameters
    const char *broker{};
    uint16_t port{};
    const char *username{};
    const char *password{};

public:
    Connector(const Connector &obj) = delete;

    void initialize();

    bool isAlive();

    void onMessage(const char *topic, void (*callback)(JsonDocument doc));

    void setWifi(const char *ssid, const char *password);

    void setMqtt(const char *broker, uint16_t port, const char *username, const char *password);

    void triggerReceived();

    static Connector *getInstance() {
        if (instancePtr == nullptr) {
            instancePtr = new Connector();
        }

        return instancePtr;
    }
};

#endif
