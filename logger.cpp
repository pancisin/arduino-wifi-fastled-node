#include "logger.h"
#include <Arduino.h>

void dbg(char *format, ...)
{
    char output[128];
    va_list args;
    va_start(args, format);

    vsprintf(output, format, args);
    Serial.println(output);

    va_end(args);
    return;
}