#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif
#include <MCP_DAC.h>
#define current_multiplier_out 1.064
#define current_multiplier_in 1.0681

extern MCP4921 MCP[];
extern const unsigned fan_control_pin_location_discharger[6];
extern const unsigned temp_measure_pin_location_discharger[6];
extern const unsigned cur_measure_pin_location_discharger[6];

float previous_cur_measurement[6] = {0, 0, 0, 0, 0, 0};
float previous_temp_measurement[6] = {0, 0, 0, 0, 0, 0};

// refer to https://github.com/infinity-engine/electronic-load.git for the circuit diagram
//  all the formula are referred to the circuit diagram.

void setDischargerCurrent(unsigned char discharger_id, float set_current)
{
    // discharger id 1,2,3,4,5,6
    // max mcp value = 4095 -> 5V
    // min mcp value = 0 -> 0V
    // set voltage can be calculated with the following formula
    // set_voltage*R1/(R1+R2) = current_flow*R10
    // or set_voltage = current_flow*0.01*(340k/10k)
    // or set_voltage = 0.34*current_flow

    float set_voltage = 0.34 * set_current * current_multiplier_out;
    unsigned int mcp_eqv = int(myMap(set_voltage, 0, 5, 0, 4095));
    MCP[discharger_id - 1].analogWrite(mcp_eqv, 0);
}

float getDischargerCurrent(unsigned char discharger_id)
{
    // calculates running average with the average of 10 samples
    // running average for previous 1/(1-beta) = 10 samples
    //  beta = 0.9;
    // amplifier gain from the discharger circuit
    //  1+R4/R3 = 1+330k/10k = 34;
    // voltage to current equivalent
    // measured_current = measured_voltage/R10
    //  i = (v/gain)/0.01;
    const float beta = 0;
    const float gain = 34;
    const float R10 = 0.01; // in ohms
    const unsigned char no_of_samples = 10;
    float sum = 0;
    float current_measurement = 0;
    // dump few values from analog pin
    analogRead(cur_measure_pin_location_discharger[discharger_id - 1]);
    for (unsigned char i = 0; i < no_of_samples; i++)
    {
        sum += analogRead(cur_measure_pin_location_discharger[discharger_id - 1]);
    }
    current_measurement = sum / no_of_samples;
    current_measurement = myMap(current_measurement, 0, 1023, 0, 5);                  // scale the 8 bit arduino mega measurement to 5 volt.
    current_measurement = (current_measurement / gain) / R10 / current_multiplier_in; // scale the voltage to current
    previous_cur_measurement[discharger_id - 1] = beta * previous_cur_measurement[discharger_id - 1] + (1 - beta) * current_measurement;
    return previous_cur_measurement[discharger_id - 1];
}

float getDischargerMosfetTemp(unsigned char discharger_id)
{
    const float beta = 0.9;                             // for running average
    const float a = 283786.2, b = 0.06593, c = 49886.0; // check the datasheet
    const unsigned char no_of_samples = 10;
    const float V_0 = 5.0;      // Volt
    const float R11 = 200000.0; // ohm
    float sum = 0;
    float current_measurement = 0;
    // dump a few measurement
    analogRead(temp_measure_pin_location_discharger[discharger_id - 1]);
    for (unsigned char i = 0; i < no_of_samples; i++)
    {
        sum += analogRead(temp_measure_pin_location_discharger[discharger_id - 1]);
    }
    current_measurement = sum / no_of_samples;
    current_measurement = myMap(current_measurement, 0, 1023, 0, 5);                                                       // scale the 8 bit, 2^8 = 1024 arduino mega measurement to 5 volt.
    current_measurement = (-1.0 / b) * (log(((R11 * current_measurement) / (a * (V_0 - current_measurement))) - (c / a))); // scalling the voltage to temp in degC
    previous_temp_measurement[discharger_id - 1] = beta * previous_temp_measurement[discharger_id - 1] + (1 - beta) * current_measurement;
    return previous_temp_measurement[discharger_id - 1];
}

void takeApprActForDischFan(unsigned char discharger_id, bool over_write, bool fan_status)
{
    // take appropriate action for discharger fan
    //  turn on the discharger fan on high temperature rise
    const float maximumAllowableMosfetTemp = 50;
    float currMosfetTemp = getDischargerMosfetTemp(discharger_id);
    if (!over_write)
    {
        // if you directly want to acces the fan
        // then send over_write = true

        if (currMosfetTemp >= maximumAllowableMosfetTemp)
        {
            // turn on the discharger fan
            digitalWrite(fan_control_pin_location_discharger[discharger_id - 1], true);
        }
        else
        {
            // keep the discharger fan off
            digitalWrite(fan_control_pin_location_discharger[discharger_id - 1], false);
        }
    }
    else
    {
        digitalWrite(fan_control_pin_location_discharger[discharger_id - 1], fan_status);
    }
}

float myMap(float x, float in_min, float in_max, float out_min, float out_max)
{
    // maps input from one range to different range
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
