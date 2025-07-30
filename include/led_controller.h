#ifndef LedController_H
#define LedController_H
#include <FastLED.h>

class LedController {
    CRGB *leds;
    CRGB *targetLeds;
    unsigned long lastLedCycle = 0;
    unsigned long lastAnimCycle = 0;
    uint8_t numLeds;

    uint8_t cycleBlendAmount = 5; // How much to blend each cycle

public:
    LedController(uint8_t dataPin, uint8_t numLeds);

    void setColor(uint8_t r, uint8_t g, uint8_t b);

    void setColorTemp(uint8_t colorTemp);

    void setColorHex(const char *colorHex);

    void turnOff();

    void loop();
};

#endif
