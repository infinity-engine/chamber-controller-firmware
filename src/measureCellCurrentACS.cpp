#include <Arduino.h>
#include "functionPrototype.h"
extern bool cell_current_addresses[6][4];
extern const unsigned int cur_average_sample_count;
extern unsigned char cur_sen_ads_location[6];
extern const float acs_sensitivity;

float measureCellCurrentACS(unsigned char cell_id)
{
    // cell_id starts from 1,2...6
    // return measured current in Ampere from ACS sensor
    float sum = 0.0;
    // set the proper channel for for mux to ads
    channelTheMux(cell_current_addresses[cell_id - 1]);
    for (unsigned int i = 0; i < cur_average_sample_count; i++)
    {
        sum += measureFromADS(cur_sen_ads_location[cell_id - 1]);
        delay(3); // give some time to acs to settle
    }
    float acsValue = float(sum / cur_average_sample_count);
    return float((2.5 - (acsValue * (5.0 / 1024.0))) / acs_sensitivity);
}