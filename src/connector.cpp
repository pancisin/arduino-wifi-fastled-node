#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include "connector.hpp"
#include "logger.hpp"

Connector *Connector::instancePtr = nullptr;

void Connector::handleHeartbeat() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastHbMillis > 5000) {
        lastHbMillis = currentMillis;

        this->hbValue = this->hbValue == 1 ? 0 : 1;
        mqttClient.beginMessage(this->hbTopic, true);
        mqttClient.print(this->hbValue);
        mqttClient.endMessage();

        digitalWrite(LED_BUILTIN, LOW);
    }

    if (currentMillis - lastHbMillis > 4900) {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

bool Connector::isAlive() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastWiFiCheck > 5000) {
        lastWiFiCheck = currentMillis;
        status = WiFi.status();

        if (status == WL_DISCONNECTED || status == WL_CONNECTION_LOST || !mqttClient.connected()) {
            initialize();
        }
    }

    mqttClient.poll();

    handleHeartbeat();
    return true;
}

void Connector::initialize() {
    dbg("Initializing connector...");

    if (WiFi.status() == WL_NO_MODULE) {
        dbg("Communication with WiFi module failed!");
        while (true) {
        }
    }

    while (status != WL_CONNECTED) {
        dbg("Attempting to connect to SSID: %s.", wifiSSID);
        status = WiFi.begin(wifiSSID, wifiPassword);
        delay(5000);
    }

    dbg("Connected to the WiFi network. IP: %s.", WiFi.localIP());

    mqttClient.stop();
    mqttClient.setUsernamePassword(this->username, this->password);

    while (!mqttClient.connect(this->broker, this->port)) {
        dbg("MQTT connection failed! Error code: %s.", mqttClient.connectError());
        delay(5000);
    }

    mqttClient.onMessage(handleMessage);

    mqttClient.subscribe(this->topic);
    dbg("Subscribed to MQTT topic %s.", this->topic);
}

void Connector::handleMessage(int messageSize) {
    dbg("Received MQTT message of size %d.", messageSize);
    Connector *instance = getInstance();
    instance->triggerReceived();
}

void Connector::triggerReceived() {
    String messageTopic = mqttClient.messageTopic();
    char top[messageTopic.length()];
    strcpy(top, messageTopic.c_str());

    if (strcmp(top, this->topic) == 0) {
        this->doc.clear();

        const DeserializationError error = deserializeJson(this->doc, mqttClient);

        if (error) {
            dbg("deserializeJson() failed: %s.", error.c_str());
            return;
        }

        this->callback(doc);
    }
}

void Connector::setWifi(const char *ssid, const char *password) {
    this->wifiSSID = ssid;
    this->wifiPassword = password;
}

void Connector::setMqtt(const char *broker, const uint16_t port, const char *username, const char *password) {
    this->broker = broker;
    this->port = port;
    this->username = username;
    this->password = password;
}

void Connector::onMessage(const char *topic, void (*callback)(JsonDocument)) {
    strcpy(this->topic, topic);
    strcat(this->topic, "/status");

    dbg("Setting MQTT topic to %s.", this->topic);

    strcpy(this->hbTopic, topic);
    strcat(this->hbTopic, "/hb");

    this->callback = callback;
}

Connector::Connector() {
    this->mqttClient = MqttClient(this->wifiClient);
}
