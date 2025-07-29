#include "led_controller.h"
#include "color_utils.h"
#include "logger.h"

LedController::LedController(uint8_t dataPin, int numLeds)
{
    this->numLeds = numLeds;

    this->leds = new CRGB[this->numLeds];
    this->targetLeds = new CRGB[this->numLeds];

    FastLED.addLeds<WS2812Controller800Khz, 13, RGB>(this->leds, this->numLeds);
    // FastLED.addLeds<WS2811, 13>(this->leds, this->numLeds);
    FastLED.setBrightness(255);
    fill_solid(targetLeds, this->numLeds, CRGB::Black);

    // FastLED.setCorrection(TypicalLEDStrip);
    // FastLED.setTemperature(Tungsten40W);

    FastLED.clear();
}

void LedController::loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastLedCycle > 25)
    {
        lastLedCycle = currentMillis;

        for (int i = 0; i < this->numLeds; i++)
        {
            if (leds[i] != targetLeds[i])
            {
                leds[i] = blend(leds[i], targetLeds[i], this->cycleBlendAmount);
            }
        }

        FastLED.show();
    }

    if (currentMillis - lastAnimCycle > 500)
    {
        lastAnimCycle = currentMillis;
        shift_left(targetLeds, this->numLeds);
    }
}

void LedController::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    fill_solid(targetLeds, this->numLeds, CRGB(r, g, b));
}

void LedController::setColorTemp(uint8_t colorTemp)
{
    CRGB color = HeatColor(colorTemp);
    fill_solid(targetLeds, this->numLeds, color);
}

void LedController::setColorHex(const char *colorHex)
{
    long colorCode = strtol(&colorHex[1], NULL, 16);

    dbg("Setting color to %s (0x%06lX).", colorHex, colorCode);

    CRGB newColor = CRGB(colorCode);
    fill_point_gradient(targetLeds, this->numLeds, newColor, CRGB::Black);
}

void LedController::turnOff()
{
    fill_solid(targetLeds, this->numLeds, CRGB(0, 0, 0));
}