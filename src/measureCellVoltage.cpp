#include <Arduino.h>
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif
#include <math.h>

extern bool cell_voltage_addresses[6][4];
extern const unsigned int vol_average_sample_count;
extern unsigned char vol_sen_ads_location[6];

float measureCellVoltage(unsigned char cell_id)
{
    // cell_id starts from 1,2...6
    // returns cell voltages in Volt measured with the help of ads

    float sum = 0.0;
    channelTheMux(cell_voltage_addresses[cell_id - 1]);
    // set the proper channel
    for (unsigned int i = 0; i < vol_average_sample_count; i++)
    {
        sum += measureFromADS(vol_sen_ads_location[cell_id - 1]);
    }
    return float(sum / vol_average_sample_count);
}