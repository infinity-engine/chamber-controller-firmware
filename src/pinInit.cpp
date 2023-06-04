#include "Arduino.h"
#include "config_atmega.h"
#include "config_const.h"
#include "functionPrototype.h"
#include <DHT.h>
extern DHT dht[];
extern CallibrationParameters calParams[];
void pinInit()
{

    pinMode(LED_BUILTIN, OUTPUT);

    for (unsigned char i = 0; i < no_of_discharger_connected; i++)
    {
        pinMode(fan_control_pin_location_discharger[i], OUTPUT);
        digitalWrite(fan_control_pin_location_discharger[i], LOW);
    }
    for (unsigned char i = 0; i < no_of_discharger_connected; i++)
    {
        pinMode(dis_cur_force[i], OUTPUT);
        digitalWrite(dis_cur_force[i], HIGH); // force to 0 current
        // takeApprActForDischFan(i,HIGH, HIGH);//by default turn on the discharger fan
        pinMode(cell_relay_location[i], OUTPUT);
        digitalWrite(cell_relay_location[i], relay_off);
    }

    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    bool address[] = {false, false, false, false};
    channelTheMux(address);

    // esp-interrupt control
    pinMode(ESP_INT_PIN, OUTPUT);
    digitalWrite(ESP_INT_PIN, LOW);

    pinMode(chamber_cooler_relay_pin, OUTPUT);
    pinMode(chamber_heater_relay_pin, OUTPUT);

    digitalWrite(chamber_cooler_relay_pin, relay_off);
    digitalWrite(chamber_heater_relay_pin, relay_off);

    // for atmega inteerupts
    pinMode(ATMEGA_INT_PIN, INPUT);
    // attachInterrupt(digitalPinToInterrupt(ATMEGA_INT_PIN), void, RISING);

    for (uint8_t i = 0; i < no_of_dht_sensor_connected; i++)
    {
        dht[i].begin();
    }
}