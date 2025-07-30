#include <ArduinoJson.h>
#include <SPI.h>
#include "connector.h"
#include "led_controller.h"
#include "logger.h"
#include "secrets.h"

LedController ledController = LedController(13, NUM_LEDS);

void handleMessage(JsonDocument doc) {
    const boolean on = doc["on"];
    const char *hex = doc["hex"];
    const uint8_t colorTemp = doc["colorTemp"];

    if (!on) {
        ledController.turnOff();
    } else if (colorTemp > 0) {
        ledController.setColorTemp(colorTemp);
    } else if (strlen(hex) != 0) {
        ledController.setColorHex(hex);
    } else {
        const JsonArray v = doc["rgb"];
        ledController.setColor(v[0], v[1], v[2]);
    }
}

Connector *connector = Connector::getInstance();

void setup() {
    Serial.begin(9600);

    dbg("Starting Living Room Controller...");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    connector->setWifi(WIFI_SSID, WIFI_PASS);
    connector->setMqtt(MQTT_BROKER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
    connector->onMessage(MQTT_LIVINGROOM_TOPIC, handleMessage);

    connector->initialize();
}

void loop() {
    connector->isAlive();
    ledController.loop();
}
