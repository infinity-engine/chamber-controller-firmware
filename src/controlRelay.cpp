#include <Arduino.h>

void controlRelay(const unsigned char relay_pin, bool relay_status)
{
    digitalWrite(relay_pin, relay_status);
}