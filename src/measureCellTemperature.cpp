#include <Arduino.h>
#include "config_atmega.h"
#include "functionPrototype.h"
#include <math.h>


float* measureCellTemperature(unsigned char cell_id)
{
    // cell_id starts from 1,2...6

    float V_0 = 5.0;//Volt
    float R_1 = 100000.0;//ohm

    //store all the temperature for a particular cell
    float temps[no_of_temp_sen_connected_cell[cell_id - 1]];

    for (unsigned char curr_sen = 0; curr_sen < no_of_temp_sen_connected_cell[cell_id - 1]; curr_sen++)
    {
        float raw = 0;
        float sum = 0;
        channelTheMux(cell_temperature_addresses[cell_id - 1][curr_sen]);
        for (unsigned int i = 0; i < temp_average_sample_count; i++)
        {
            raw = measureFromADS(tem_sen_ads_location[cell_id][curr_sen]);
            sum += raw;
        }

        //get the average reading
        sum /= float(temp_average_sample_count);

        //convert into  °C formula provided by datasheet ntc 100k
        float t = (-1.0/ntc_b)*(log(((R_1*sum)/(ntc_a*(V_0-sum)))-(ntc_c/ntc_a)));
        temps[curr_sen] = t;
    }
    return temps;
}