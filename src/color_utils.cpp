#include "color_utils.hpp"
#include <FastLED.h>

void shift_left(CRGB *arr, const uint8_t numToMove) {
    const CRGB temp = arr[0];

    for (uint8_t i = 0; i < numToMove - 1; i++) {
        arr[i] = arr[i + 1];
    }

    arr[numToMove - 1] = temp;
}

void fill_point_gradient(CRGB *targetArray, const uint8_t numToFill,
                         CRGB &color1, const CRGB &color2) {
    const uint8_t splitPoint = ceil(static_cast<double>(numToFill) / 2);
    const uint8_t step = 255 / splitPoint;

    for (uint8_t i = 0; i < splitPoint; i++) {
        nblend(color1, color2, step * i);

        targetArray[i] = color1;
        targetArray[numToFill - i - 1] = color1;
    }
}
