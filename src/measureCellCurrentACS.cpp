#include <Arduino.h>
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif

#ifndef CONFIG_CONST
#define CONFIG_CONST
#include "config_const.h"
#endif

float measureCellCurrentACS(unsigned char cell_id)
{
    // cell_id starts from 1,2...6
    // return measured current in Ampere from ACS sensor
    float sum = 0.0;
    // set the proper channel for for mux to ads
    bool address[4];
    byte tem = pgm_read_byte(&cell_current_addresses[cell_id - 1]);
    for (uint8_t i = 0; i < 4; i++)
    {
        address[i] = bitRead(tem, 7 - i);
    }
    channelTheMux(address);
    for (unsigned int i = 0; i < cur_average_sample_count; i++)
    {
        sum += measureFromADS(cur_sen_ads_location[cell_id - 1]);
        delay(3); // give some time to acs to settle
    }
    float acsValue = float(sum / cur_average_sample_count);
    return float((2.5 - (acsValue * (5.0 / 1024.0))) / acs_sensitivity);
}