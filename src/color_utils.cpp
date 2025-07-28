#include "color_utils.h"
#include <FastLED.h>

void shift_left(struct CRGB *arr, int numToMove)
{
    CRGB temp = arr[0];

    for (int i = 0; i < numToMove - 1; i++)
    {
        arr[i] = arr[i + 1];
    }

    arr[numToMove - 1] = temp;
}

void fill_point_gradient(struct CRGB *targetArray, int numToFill,
                         CRGB &color1, const CRGB &color2)
{
    int splitPoint = ceil((double)numToFill / 2);
    int step = 255 / splitPoint;

    for (int i = 0; i < splitPoint; i++)
    {
        nblend(color1, color2, step * i);

        targetArray[i] = color1;
        targetArray[numToFill - i - 1] = color1;
    }
}