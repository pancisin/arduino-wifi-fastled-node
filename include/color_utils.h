#ifndef COLOR_UTILS_INCLUDE
#define COLOR_UTILS_INCLUDE
#include <stdint.h>

void shift_left(struct CRGB *arr, uint8_t numToMove);

void fill_point_gradient(CRGB *targetArray, uint8_t numToFill,
                         CRGB &color1, const CRGB &color2);

#endif
