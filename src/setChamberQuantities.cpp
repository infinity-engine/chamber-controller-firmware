#include "config_atmega.h"
#include "functionPrototype.h"

extern const unsigned char chamber_heater_relay_pin;
extern const unsigned char chamber_cooler_relay_pin;

void setChamberTemperature(float set_temp, float current_temp)
{
    // sets the desirable temperature of the chamber
    float allowed_var = 0.5; // allowed variance of chamber temperature
    if (set_temp > current_temp + allowed_var)
    {
        // turn on the heater
        controlRelay(chamber_heater_relay_pin, relay_on);
        // turn off the cooler
        controlRelay(chamber_cooler_relay_pin, relay_off);
    }
    else if (set_temp < current_temp - allowed_var)
    {
        // turn on the coller
        controlRelay(chamber_cooler_relay_pin, relay_on);
        // turn off the heater
        controlRelay(chamber_heater_relay_pin, relay_off);
    }
}