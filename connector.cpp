#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include "connector.h"

Connector *Connector::instancePtr = nullptr;

void dbg(char *format, ...)
{
    char output[128];
    va_list args;
    va_start(args, format);

    vsprintf(output, format, args);
    Serial.println(output);

    va_end(args);
    return;
}

void Connector::handleHeartbeat()
{
    unsigned long currentMillis = millis();
    if (abs(currentMillis - lastHbMillis) > 5000)
    {
        lastHbMillis = currentMillis;

        this->hbValue = this->hbValue == 1 ? 0 : 1;
        mqttClient.beginMessage(this->hbTopic, true);
        mqttClient.print(this->hbValue);
        mqttClient.endMessage();

        digitalWrite(LED_BUILTIN, LOW);
    }

    if (abs(currentMillis - lastHbMillis) > 4900)
    {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

bool Connector::isAlive()
{
    unsigned long currentMillis = millis();
    if (abs(currentMillis - lastWiFiCheck) > 5000)
    {
        lastWiFiCheck = currentMillis;
        status = WiFi.status();

        if (status == WL_DISCONNECTED || status == WL_CONNECTION_LOST || !mqttClient.connected())
        {
            initialize();
        }
    }

    // ArduinoOTA.handle();

    mqttClient.poll();

    handleHeartbeat();
}

void Connector::initialize()
{
    dbg("Initializing connector...");

    // IPAddress ip(192, 168, 88, 78);
    // WiFi.config(ip);
    if (WiFi.status() == WL_NO_MODULE)
    {
        dbg("Communication with WiFi module failed!");
        while (true)
        {
        }
    }

    while (status != WL_CONNECTED)
    {
        dbg("Attempting to connect to SSID: %s.", wifiSSID);
        status = WiFi.begin(wifiSSID, wifiPassword);
        delay(5000);
    }

    dbg("Connected to the WiFi network. IP: %s.", WiFi.localIP());

    // OTAStorage storage();
    // ArduinoOTA.begin(ip, "Living Room", "lonelybinary", otaStorage);

    mqttClient.stop();
    mqttClient.setUsernamePassword(this->username, this->password);

    while (!mqttClient.connect(this->broker, this->port))
    {
        dbg("MQTT connection failed! Error code: %s.", mqttClient.connectError());
        delay(5000);
    }

    mqttClient.onMessage(&Connector::handleMessage);

    mqttClient.subscribe(this->topic);
    dbg("Subscribed to MQTT topic %s.", this->topic);
}

void Connector::handleMessage(int messageSize)
{
    dbg("Received MQTT message of size %d.", messageSize);
    Connector *instance = Connector::getInstance();
    instance->triggerReceived();
}

void Connector::triggerReceived()
{
    // String messageTopic = mqttClient.messageTopic();
    // char top[messageTopic.length()];
    // strcpy(top, messageTopic.c_str());

    // dbg("trigger comparing %s = %s", top, this->topic);

    // if (strcmp(top, this->topic) == 0)
    // {
    this->doc.clear();

    DeserializationError error = deserializeJson(this->doc, mqttClient);

    if (error)
    {
        dbg("deserializeJson() failed: %s.", error.c_str());
        return;
    }

    this->callback(doc);
    // }
}

void Connector::setWifi(const char *ssid, const char *password)
{
    this->wifiSSID = ssid;
    this->wifiPassword = password;
}

void Connector::setMqtt(const char *broker, uint16_t port, const char *username, const char *password)
{
    this->broker = broker;
    this->port = port;
    this->username = username;
    this->password = password;
}

void Connector::onMessage(const char *top, void (*callback)(StaticJsonDocument<2048>))
{
    // strcpy(this->topic, top);
    // strcat(this->topic, "/status");

    // dbg("Setting MQTT topic to %s.", this->topic);

    // strcpy(this->hbTopic, top);
    // strcat(this->hbTopic, "/hb");

    this->topic = "home/livingroom/light/ambient/status";
    this->hbTopic = "home/livingroom/light/ambient/hb";

    dbg("Setting MQTT topic to %s.", this->topic);
    dbg("Setting MQTT heartbeat topic to %s.", this->hbTopic);

    this->callback = callback;
}

Connector::Connector()
{
    this->mqttClient = MqttClient(this->wifiClient);
}
