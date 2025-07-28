# arduino-wifi-fastled-node

Arduino node of smart home automation controlling ws2811 led strips.

```
#define NUM_LEDS 114

#define WIFI_SSID wifi_ssid
#define WIFI_PASS password

#define MQTT_BROKER mqtt_url
#define MQTT_USERNAME mqtt_username
#define MQTT_PASSWORD mqtt_password
#define MQTT_PORT 1883

#define MQTT_TOPIC "livingroom/controller"
#define MQTT_LIVINGROOM_TOPIC "livingroom/lighting/ambient"
#define MQTT_KITCHEN_TOPIC "kitchen/lighting/side"
```
