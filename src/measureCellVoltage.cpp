#include <Arduino.h>
#include "config_atmega.h"
#include "functionPrototype.h"
#include <math.h>


float measureCellVoltage(unsigned char cell_id)
{
    // cell_id starts from 1,2...6
    //returns cell voltages in Volt measured with the help of ads
    
    float sum = 0.0;
    channelTheMux(cell_voltage_addresses[cell_id-1]);
    // set the proper channel
    for (unsigned int i=0;i<vol_average_sample_count;i++){
        sum += measureFromADS(vol_sen_ads_location[cell_id-1]);
    }
    return float(sum/vol_average_sample_count);
}