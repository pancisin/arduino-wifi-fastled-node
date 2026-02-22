#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <avr/wdt.h>
#include "connector.hpp"
#include "logger.hpp"

Connector *Connector::instancePtr = nullptr;

bool Connector::attemptWiFiConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        dbg("WiFi already connected. IP: %s", WiFi.localIP());
        return true;
    }

    unsigned long currentMillis = millis();

    if (currentMillis - lastReconnectAttempt < 5000) {
        return false;
    }

    lastReconnectAttempt = currentMillis;
    reconnectAttempts++;

    if (reconnectAttempts > 20) {
        dbg("Too many WiFi reconnection attempts. Resetting...");
        while(1);
    }

#ifdef STATIC_IP
    IPAddress ip, gateway, subnet, dns;
    ip.fromString(STATIC_IP);
    gateway.fromString(GATEWAY);
    subnet.fromString(SUBNET);
    dns.fromString(DNS);

    WiFi.config(ip, dns, gateway, subnet);
    dbg("Configuring static IP: %s", STATIC_IP);
#endif

    dbg("Attempting WiFi connection to SSID: %s (attempt %d)", wifiSSID, reconnectAttempts);
    wdt_reset();

    status = WiFi.begin(wifiSSID, wifiPassword);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < 5000) {
        delay(500);
        wdt_reset();
    }

    if (WiFi.status() == WL_CONNECTED) {
        dbg("WiFi connected! IP: %s", WiFi.localIP());
        reconnectAttempts = 0;
        wdt_reset();
        return true;
    }

    wdt_reset();
    return false;
}

bool Connector::attemptMQTTConnection() {
    if (mqttClient.connected()) {
        return true;
    }

    unsigned long currentMillis = millis();

    if (currentMillis - lastReconnectAttempt < 5000) {
        return false;
    }

    lastReconnectAttempt = currentMillis;
    reconnectAttempts++;

    if (reconnectAttempts > 20) {
        dbg("Too many MQTT reconnection attempts. Resetting...");
        while(1);
    }

    dbg("Attempting MQTT connection (attempt %d)", reconnectAttempts);

    if (reconnectAttempts == 1) {
        mqttClient.setUsernamePassword(this->username, this->password);
    }

    wdt_reset();

    if (mqttClient.connect(this->broker, this->port)) {
        dbg("MQTT connected!");
        reconnectAttempts = 0;

        if (!callbacksRegistered) {
            mqttClient.onMessage(handleMessage);
            callbacksRegistered = true;
            dbg("MQTT callbacks registered");

            wdt_enable(WDTO_2S);
            dbg("Watchdog timer enabled (2s timeout) - system is now stable");
        }

        mqttClient.subscribe(this->topic);
        dbg("Subscribed to MQTT topic %s", this->topic);

        return true;
    } else {
        dbg("MQTT connection failed! Error code: %d", mqttClient.connectError());
    }

    wdt_reset();
    return false;
}

void Connector::handleHeartbeat() {
    if (connectionState != CONNECTED) {
        return;
    }

    unsigned long currentMillis = millis();

    if (currentMillis - lastHbMillis >= 5000) {
        lastHbMillis = currentMillis;

        this->hbValue = this->hbValue == 1 ? 0 : 1;
        mqttClient.beginMessage(this->hbTopic, true);
        mqttClient.print(this->hbValue);
        mqttClient.endMessage();

        digitalWrite(LED_BUILTIN, HIGH);
    }

    if (currentMillis - lastHbMillis >= 100 && currentMillis - lastHbMillis < 5000) {
        digitalWrite(LED_BUILTIN, LOW);
    }
}

bool Connector::isAlive() {
    unsigned long currentMillis = millis();
    wdt_reset();

    if (currentMillis - lastWiFiCheck >= 5000) {
        lastWiFiCheck = currentMillis;
        status = WiFi.status();
        wdt_reset();


        if (status == WL_DISCONNECTED || status == WL_CONNECTION_LOST) {
            dbg("WiFi disconnected! Status: %d", status);
            connectionState = DISCONNECTED;
        } else if (!mqttClient.connected() && status == WL_CONNECTED) {
            dbg("MQTT disconnected!");
            connectionState = CONNECTING_MQTT;
        } else if (status == WL_CONNECTED && mqttClient.connected()) {
            connectionState = CONNECTED;
        }
    }


    switch (connectionState) {
        case DISCONNECTED:
            connectionState = CONNECTING_WIFI;
            reconnectAttempts = 0;
            lastReconnectAttempt = 0;
            wdt_reset();
            break;

        case CONNECTING_WIFI:
            if (attemptWiFiConnection()) {
                connectionState = CONNECTING_MQTT;
                reconnectAttempts = 0;
                lastReconnectAttempt = 0;
            }
            wdt_reset();
            break;

        case CONNECTING_MQTT:
            if (attemptMQTTConnection()) {
                connectionState = CONNECTED;
            }
            wdt_reset();
            break;

        case CONNECTED:
            mqttClient.poll();
            handleHeartbeat();
            wdt_reset();
            break;
    }

    return connectionState == CONNECTED;
}

void Connector::initialize() {
    dbg("Initializing connector...");

    if (WiFi.status() == WL_NO_MODULE) {
        dbg("Communication with WiFi module failed!");
        while (true) {
            delay(1000);
        }
    }


    dbg("Watchdog timer will be enabled after first connection");


    connectionState = DISCONNECTED;
    reconnectAttempts = 0;
    lastReconnectAttempt = 0;

    dbg("Connector initialized. Will attempt connections in main loop.");
}

void Connector::handleMessage(int messageSize) {
    dbg("Received MQTT message of size %d.", messageSize);
    Connector *instance = getInstance();
    instance->triggerReceived();
}

void Connector::triggerReceived() {

    if (mqttClient.messageTopic() == this->topic) {
        this->doc.clear();

        const DeserializationError error = deserializeJson(this->doc, mqttClient);

        if (error) {
            dbg("deserializeJson() failed: %s", error.c_str());
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

    strncpy(this->topic, topic, sizeof(this->topic) - 9);
    this->topic[sizeof(this->topic) - 9] = '\0';
    strncat(this->topic, "/status", 7);

    dbg("Setting MQTT topic to %s", this->topic);

    strncpy(this->hbTopic, topic, sizeof(this->hbTopic) - 4);
    this->hbTopic[sizeof(this->hbTopic) - 4] = '\0';
    strncat(this->hbTopic, "/hb", 3);

    this->callback = callback;
}

Connector::Connector() {
    this->mqttClient = MqttClient(this->wifiClient);
}
