# Arduino WiFi FastLED Node

IoT-enabled smart home LED controller for WS2811/WS2812 LED strips with WiFi connectivity and MQTT integration.

[![Platform](https://img.shields.io/badge/platform-Arduino-blue.svg)](https://www.arduino.cc/)
[![Framework](https://img.shields.io/badge/framework-PlatformIO-orange.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## 🌟 Features

- **WiFi Connectivity** - Automatic connection with auto-reconnection support
- **MQTT Integration** - Subscribe to topics for real-time LED control
- **FastLED Support** - Control WS2811/WS2812/WS2812B LED strips
- **RGB Color Control** - Full RGB color selection via hex codes or RGB values
- **Color Temperature** - Adjustable warm/cool white settings
- **Non-Blocking Architecture** - System remains responsive during reconnections
- **Watchdog Timer** - Auto-recovery from hangs or crashes
- **Memory Monitoring** - Real-time RAM usage tracking
- **Production Stable** - Designed for 24/7 continuous operation

## 📋 Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [MQTT API](#mqtt-api)
- [Building & Uploading](#building--uploading)
- [Monitoring](#monitoring)
- [Troubleshooting](#troubleshooting)
- [Architecture](#architecture)
- [Contributing](#contributing)
- [License](#license)

## 🔧 Hardware Requirements

### Supported Boards
- **Arduino UNO WiFi Rev2** (ATmega4809) - Primary target
- **ESP32-C3** - Alternative platform for testing

### Components
- WS2811, WS2812, or WS2812B LED strip
- 5V power supply (rated for your LED strip)
- Data line resistor: 220-470Ω (recommended)
- Capacitor: 1000µF across power supply (recommended)

### Wiring
```
Arduino Pin 13 → LED Strip Data Pin
Arduino GND    → LED Strip GND
Power Supply + → LED Strip VCC
Power Supply - → LED Strip GND & Arduino GND (common ground)
```

## 💻 Software Requirements

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- MQTT Broker (e.g., Mosquitto, Home Assistant)
- WiFi network with stable connection

### Dependencies
All dependencies are managed automatically by PlatformIO:

```ini
- FastLED @ ^3.10.1
- WiFiNINA @ ^1.9.1
- ArduinoJson @ ^7.4.2
- ArduinoMqttClient @ ^0.1.8
```

## 📦 Installation

### Using PlatformIO (Recommended)

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/arduino-wifi-fastled-node.git
   cd arduino-wifi-fastled-node
   ```

2. **Install PlatformIO**
   ```bash
   pip install platformio
   ```

3. **Build the project**
   ```bash
   pio run
   ```

### Using Arduino IDE

1. Install required libraries via Library Manager:
   - FastLED
   - WiFiNINA
   - ArduinoJson (v7.x)
   - ArduinoMqttClient

2. Open `src/main.cpp` in Arduino IDE

3. Configure board: `Tools > Board > Arduino UNO WiFi Rev2`

## ⚙️ Configuration

### 1. Create `secrets.h`

Create a file `include/secrets.h` with your credentials:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"

// MQTT Broker Configuration
#define MQTT_USERNAME "mqtt_username"
#define MQTT_PASSWORD "mqtt_password"

#endif
```

### 2. Configure `platformio.ini`

The project supports multiple environments. Edit build flags as needed:

```ini
[env:livingroom_controller]
platform = atmelmegaavr
board = uno_wifi_rev2
framework = arduino
monitor_speed = 9600
build_flags = 
    -D MQTT_BROKER="\"192.168.1.100\""      # Your MQTT broker IP
    -D MQTT_PORT=1883                        # MQTT port
    -D MQTT_LIVINGROOM_TOPIC="\"home/livingroom/light/ambient\""
    -D NUM_LEDS=20                           # Number of LEDs in your strip
```

## 📡 MQTT API

### Subscribe Topic
The controller subscribes to: `<MQTT_TOPIC>/status`

Example: `home/livingroom/light/ambient/status`

### Heartbeat Topic
Publishes heartbeat to: `<MQTT_TOPIC>/hb`

Example: `home/livingroom/light/ambient/hb` (toggles 0/1 every 5 seconds)

### Control Messages

#### Turn LEDs On with Hex Color
```json
{
  "on": true,
  "hex": "#FF0000"
}
```

#### Turn LEDs On with RGB Values
```json
{
  "on": true,
  "rgb": [255, 0, 0]
}
```

#### Set Color Temperature
```json
{
  "on": true,
  "colorTemp": 128
}
```
*Value: 0-255 (0 = cool, 255 = warm)*

#### Turn LEDs Off
```json
{
  "on": false
}
```

### Example: Testing with mosquitto_pub

```bash
# Turn on red
mosquitto_pub -h 192.168.1.100 -u username -P password \
  -t "home/livingroom/light/ambient/status" \
  -m '{"on":true,"hex":"#FF0000"}'

# Turn on green with RGB
mosquitto_pub -h 192.168.1.100 -u username -P password \
  -t "home/livingroom/light/ambient/status" \
  -m '{"on":true,"rgb":[0,255,0]}'

# Turn off
mosquitto_pub -h 192.168.1.100 -u username -P password \
  -t "home/livingroom/light/ambient/status" \
  -m '{"on":false}'
```

## 🚀 Building & Uploading

### Build for Arduino UNO WiFi Rev2
```bash
pio run -e livingroom_controller
```

### Upload via USB (Default - First Time Required)
```bash
pio run -e livingroom_controller --target upload
```

### Upload via OTA (Wireless - After First USB Upload)
```bash
pio run -e livingroom_controller_ota --target upload
```

### Build for ESP32-C3 (Testing)
```bash
pio run -e local_testing --target upload
```

### Monitor Serial Output
```bash
pio device monitor -b 9600
```

Or use the combined upload + monitor:
```bash
# USB upload with monitor
pio run -e livingroom_controller --target upload && pio device monitor -b 9600
```

## 📊 Monitoring

### Serial Output

Connect to serial monitor at **9600 baud** to see:

```
Starting Living Room Controller...
Free RAM: 4523 bytes
Initializing connector...
Watchdog timer enabled (8s timeout)
Connector initialized. Will attempt connections in main loop.
Setup complete. Free RAM: 4487 bytes
Attempting WiFi connection to SSID: YourNetwork (attempt 1)
WiFi connected! IP: 192.168.1.100
Attempting MQTT connection (attempt 1)
MQTT connected!
MQTT callbacks registered
Subscribed to MQTT topic home/livingroom/light/ambient/status
Free RAM: 4465 bytes
```

### Health Indicators

- **LED Heartbeat** - Built-in LED blinks every 5 seconds when connected
- **Free RAM Reports** - Memory reported every 60 seconds
- **MQTT Heartbeat** - Published to `/hb` topic every 5 seconds

### Expected Memory Usage

- **Initial Free RAM:** ~4500 bytes (UNO WiFi Rev2)
- **Runtime Free RAM:** 4300-4700 bytes (should remain stable)
- **Critical Level:** < 1000 bytes (system will auto-reset)

## 🔍 Troubleshooting

### Device Not Connecting to WiFi
1. Verify `WIFI_SSID` and `WIFI_PASS` in `secrets.h`
2. Check WiFi signal strength
3. Ensure 2.4GHz network (5GHz not supported)
4. Watch serial output for error messages

### MQTT Connection Fails
1. Verify MQTT broker IP and port
2. Check `MQTT_USERNAME` and `MQTT_PASSWORD`
3. Test broker with mosquitto_sub:
   ```bash
   mosquitto_sub -h 192.168.1.100 -u username -P password -t "#"
   ```

### LEDs Not Responding
1. Check LED strip wiring (data pin to D13)
2. Verify `NUM_LEDS` matches your strip
3. Ensure common ground between Arduino and LED power supply
4. Check MQTT messages are being received (serial monitor)

### System Resets Every 8 Seconds
- Watchdog timer is triggering
- Usually indicates network unavailable
- Check WiFi and MQTT broker availability
- System will keep retrying until connection succeeds

### Memory Decreasing Over Time
- Monitor "Free RAM" messages in serial output
- Should stay within ±200 bytes over hours/days
- If decreasing: report as bug (shouldn't happen with current fixes)

## 🏗️ Architecture

### Component Overview

```
┌─────────────────────────────────────────────────────┐
│                    Main Loop                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐  │
│  │  Connector   │  │     LED      │  │  Memory  │  │
│  │   isAlive()  │  │ Controller   │  │ Monitor  │  │
│  └──────────────┘  └──────────────┘  └──────────┘  │
└─────────────────────────────────────────────────────┘
           │                  │                │
           ▼                  ▼                ▼
    ┌──────────┐      ┌─────────────┐   ┌──────────┐
    │   WiFi   │      │   FastLED   │   │   RAM    │
    │   MQTT   │      │   Library   │   │  Tracker │
    └──────────┘      └─────────────┘   └──────────┘
```

### Key Classes

- **Connector** - Manages WiFi and MQTT connectivity with state machine
- **LedController** - Handles LED strip control and animations
- **Logger** - Debug output with safe string formatting

### State Machine (Connector)

```
DISCONNECTED → CONNECTING_WIFI → CONNECTING_MQTT → CONNECTED
      ▲                                                  │
      └──────────────────────────────────────────────────┘
                (on connection loss)
```

### Features

- **Non-Blocking Reconnection** - System stays responsive
- **Watchdog Timer** - 8-second timeout for auto-recovery
- **Memory Safety** - Bounds-checked buffers, no leaks
- **Retry Limits** - Max 20 connection attempts before reset

## 🛡️ Stability Features

This project includes production-grade stability improvements:

- ✅ **Watchdog Timer** - Auto-resets on hang (8s timeout)
- ✅ **Non-Blocking State Machine** - Never freezes during reconnection
- ✅ **Memory Leak Prevention** - Stable memory over weeks of operation
- ✅ **Buffer Overflow Protection** - Safe string operations
- ✅ **Connection Retry Limits** - Prevents infinite loops
- ✅ **Real-Time Monitoring** - Memory and connection status

**Designed for 24/7 continuous operation with automatic recovery.**

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [FastLED](https://github.com/FastLED/FastLED) - High-performance LED library
- [ArduinoJson](https://arduinojson.org/) - JSON parsing
- [Arduino MQTT Client](https://github.com/arduino-libraries/ArduinoMqttClient) - MQTT support

## 📞 Support

For issues and questions:
- Open an [Issue](https://github.com/yourusername/arduino-wifi-fastled-node/issues)
- Check existing documentation in the `docs/` folder

---

**Made with ❤️ for the Smart Home community**

