#include <ArduinoJson.h>
#include <SPI.h>
#include <avr/wdt.h>
#include "connector.hpp"
#include "led_controller.hpp"
#include "logger.hpp"
#include "secrets.h"


int freeRam() {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

LedController ledController = LedController(13, NUM_LEDS);
unsigned long lastMemoryReport = 0;

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
    delay(1000);

    wdt_disable();

    dbg("Starting Living Room Controller...");
    dbg("Free RAM: %d bytes", freeRam());

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    connector->setWifi(WIFI_SSID, WIFI_PASS);
    connector->setMqtt(MQTT_BROKER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
    connector->onMessage(MQTT_LIVINGROOM_TOPIC, handleMessage);

    connector->initialize();

    dbg("Setup complete. Free RAM: %d bytes", freeRam());
}

void loop() {
    connector->isAlive();
    ledController.loop();

    unsigned long currentMillis = millis();
    if (currentMillis - lastMemoryReport >= 60000) {
        lastMemoryReport = currentMillis;
        dbg("Free RAM: %d bytes", freeRam());
    }
}
