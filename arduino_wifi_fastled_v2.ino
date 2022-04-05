#include "_livingroom.h"

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <FastLED.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>

#define ledStrips 2
int ledPins[ledStrips] = { 13, 12 };

String ledTopic[ledStrips] = {
  MQTT_LIVINGROOM_TOPIC,
  MQTT_KITCHEN_TOPIC
};

CRGB leds[ledStrips][NUM_LEDS];
CRGB targetLeds[ledStrips][NUM_LEDS];

WiFiClient wifiClient;
int status = WL_IDLE_STATUS;

MqttClient mqttClient(wifiClient);

unsigned long lastHbMillis;
unsigned long lastLedCycle;
unsigned long lastWiFiCheck;

void dbg(char *format, ...) {
  char output[128];
  va_list args;
  va_start(args, format); 
  
  vsprintf(output, format, args);
  Serial.println(String(millis() / 1000L, DEC) + "s: " + output);
  
  va_end(args);
  return;
}

void checkWiFi() {
  unsigned long currentMillis = millis();
  if (abs(currentMillis - lastWiFiCheck) > 5000) {
    lastWiFiCheck = currentMillis;
    
    status = WiFi.status();
  
    if (status == WL_DISCONNECTED || status == WL_CONNECTION_LOST) {
      initializeComunication();
    }
  }
}

void handleHeartbeat () {
  unsigned long currentMillis = millis();
  if (abs(currentMillis - lastHbMillis) > 5000) {
    lastHbMillis = currentMillis;
    String hbTopic = String(MQTT_TOPIC) + "/hb";

    mqttClient.beginMessage(hbTopic.c_str(), true);
    mqttClient.print(String(lastHbMillis).c_str());
    mqttClient.endMessage();
    
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  if (currentMillis - lastHbMillis > 4900) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void ledCycle () {
  unsigned long currentMillis = millis();
  if (abs(currentMillis - lastLedCycle) > 75) {
    lastLedCycle = currentMillis;
    
    for (int i = 0; i < NUM_LEDS; i++) {
      for (int ledsIndex = 0; ledsIndex < ledStrips; ledsIndex++) {
        leds[ledsIndex][i] = blend(leds[ledsIndex][i], targetLeds[ledsIndex][i], 25);
      }
    }
    
    FastLED.show();
  }
}

void initializeComunication() {
  if (WiFi.status() == WL_NO_MODULE) {
    dbg("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    dbg("Please upgrade the firmware (%s) to version: %s.", fv, WIFI_FIRMWARE_LATEST_VERSION);
  }

  while (status != WL_CONNECTED) {
    dbg("Attempting to connect to SSID: %s.", WIFI_SSID);
     
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    delay(5000);
  }

  dbg("Connected to the WiFi network.");

  mqttClient.stop();
  
  mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
  if (!mqttClient.connect(MQTT_BROKER, MQTT_PORT)) {
    dbg("MQTT connection failed! Error code: %s.", mqttClient.connectError());
    while (1);
  }

  mqttClient.onMessage(callback);

  for (int i = 0; i < ledStrips; i++) {
    String topic = String(ledTopic[i]) + "/status";
    mqttClient.subscribe(topic);    

    dbg("Subscribed to MQTT topic %s.", topic.c_str());
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  initializeComunication();

  FastLED.addLeds<WS2812, 13, RGB>(leds[0], NUM_LEDS);
  FastLED.addLeds<WS2812, 12, RGB>(leds[1], NUM_LEDS);

  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setTemperature(Tungsten40W);

  FastLED.clear();
}

void callback(int messageSize) {
  String topic = mqttClient.messageTopic();
  dbg("Received JSON data (%d bytes).", messageSize);

  for (int ledsIndex = 0; ledsIndex < ledStrips; ledsIndex++) {
    if (topic.startsWith(ledTopic[ledsIndex])) {  
      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, mqttClient);
    
      if (error) {
        dbg("deserializeJson() failed: ", error.f_str());
        return;
      }
    
      boolean on = doc["on"];
      uint8_t r = doc["rgb"][0];
      uint8_t g = doc["rgb"][1];
      uint8_t b = doc["rgb"][2];
      
      uint8_t colorTemp = doc["colorTemp"];
      uint8_t brightness = doc["brightness"];
      uint8_t transitionTime = doc["transitionTime"];
      JsonArray colorMap = doc["map"];
    
      if (!on) {
        fill_solid(targetLeds[ledsIndex], NUM_LEDS, CRGB(0, 0, 0));
      } else if (!colorMap.isNull()) {
        size_t mapSize = colorMap.size();

        for (int i = 0; i < NUM_LEDS; i++) {
          char* hex = colorMap[i % mapSize].as<char*>();
          long colorCode = strtol(&hex[1], NULL, 16);
          targetLeds[ledsIndex][i].setColorCode(colorCode);
        }
      } else if (colorTemp > 0) {
        CRGB color = HeatColor(colorTemp);
        fill_solid(targetLeds[ledsIndex], NUM_LEDS, color);
      } else {
        fill_solid(targetLeds[ledsIndex], NUM_LEDS, CRGB(r, g, b));
      }

      return;
    }
  } 
}

void loop() {
  checkWiFi();
  
  mqttClient.poll();

  handleHeartbeat();

  ledCycle();
}
