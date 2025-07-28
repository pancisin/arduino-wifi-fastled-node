#include <ArduinoJson.h>
#include <SPI.h>
#include "connector.h"
#include "led_controller.h"

LedController ledController = LedController(13, NUM_LEDS);

void handleMessage(StaticJsonDocument<2048> doc)
{
  boolean on = doc["on"];
  uint8_t r = doc["rgb"][0];
  uint8_t g = doc["rgb"][1];
  uint8_t b = doc["rgb"][2];

  const char *hex = doc["hex"];

  uint8_t colorTemp = doc["colorTemp"];
  uint8_t brightness = doc["brightness"];
  uint8_t transitionTime = doc["transitionTime"];

  if (!on)
  {
    ledController.turnOff();
  }
  else if (colorTemp > 0)
  {
    ledController.setColorTemp(colorTemp);
  }
  else if (strlen(hex) != 0)
  {
    ledController.setColorHex(hex);
  }
  else
  {
    ledController.setColor(r, g, b);
  }
}

Connector *connector = Connector::getInstance();

void setup()
{
  Serial.begin(9600);

  Serial.println("Starting Living Room Controller...");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  connector->setWifi(WIFI_SSID, WIFI_PASS);
  connector->setMqtt(MQTT_BROKER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
  connector->onMessage(MQTT_LIVINGROOM_TOPIC, handleMessage);

  connector->initialize();
}

void loop()
{
  connector->isAlive();
  ledController.loop();
}
