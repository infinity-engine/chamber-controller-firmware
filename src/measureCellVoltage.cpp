#include <Arduino.h>
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif
#include <math.h>

#ifndef CONFIG_CONST
#define CONFIG_CONST
#include "config_const.h"
#endif

float measureCellVoltage(unsigned char cell_id)
{
    // cell_id starts from 1,2...6
    // returns cell voltages in Volt measured with the help of ads

    float sum = 0.0;
    bool address[4];
    byte tem = pgm_read_byte(&cell_voltage_addresses[cell_id - 1]);
    for (uint8_t i = 0; i < 4; i++)
    {
        address[i] = bitRead(tem, 7 - i);
    }
    channelTheMux(address);
    // set the proper channel
    for (unsigned int i = 0; i < vol_average_sample_count; i++)
    {
        sum += measureFromADS(vol_sen_ads_location[cell_id - 1]);
    }
    return float(sum / vol_average_sample_count);
}