#ifndef COLOR_UTILS_INCLUDE
#define COLOR_UTILS_INCLUDE

void shift_left(struct CRGB *arr, int numToMove);
void fill_point_gradient(struct CRGB *targetArray, int numToFill,
                         CRGB &color1, const CRGB &color2);

#endif
