#include "logger.hpp"
#include <Arduino.h>

void dbg(const char format[], ...) {
    char output[128];
    va_list args;
    va_start(args, format);

    vsnprintf(output, sizeof(output), format, args);  // Safe version with bounds checking
    Serial.println(output);

    va_end(args);
}
