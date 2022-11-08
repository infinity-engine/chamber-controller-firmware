#include <Arduino.h>
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif

#ifndef CONFIG_CONST
#define CONFIG_CONST
#include "config_const.h"
#endif

float *measureCellTemperature(unsigned char cell_id, float *temps)
{
    // cell_id starts from 1,2...6

    float V_0 = 5.0;      // Volt
    float R_1 = 200000.0; // ohm

    // store all the temperature for a particular cell
    const byte *T_Address = pgm_read_ptr(&cell_temperature_addresses[cell_id - 1]);
    // for(int i=0;i<6;i++){
    //     Serial.println(pgm_read_byte(&T_Address[i]),BIN);//start with leading 1
    // }
    for (unsigned char curr_sen = 0; curr_sen < no_of_temp_sen_connected_cell[cell_id - 1]; curr_sen++)
    {
        float raw = 0;
        float sum = 0;
        bool address[4];
        byte tem = pgm_read_byte(&T_Address[curr_sen]);
        
        for (uint8_t i = 0; i < 4; i++)
        {
            address[i] = bitRead(tem, 7 - i);
        }
        channelTheMux(address);
        for (unsigned int i = 0; i < temp_average_sample_count; i++)
        {
            raw = measureFromADS(tem_sen_ads_location[cell_id][curr_sen]);
            sum += raw;
        }

        // get the average reading
        sum /= float(temp_average_sample_count);

        // convert into  °C formula provided by datasheet ntc 100k
        float t = (-1.0 / ntc_b) * (log(((R_1 * sum) / (ntc_a * (V_0 - sum))) - (ntc_c / ntc_a)));
        // Serial.println(((R_1 * sum) / (ntc_a * (V_0 - sum))) - (ntc_c / ntc_a));
        temps[curr_sen] = t;
    }
    return temps;
}
float measureAvgCellTemp(unsigned char cell_id, float *result_arr)
{
    float temps[no_of_temp_sen_connected_cell[cell_id - 1]];
    if (result_arr == NULL)
    {
        result_arr = measureCellTemperature(cell_id, temps);
    }
    float sum = 0;
    for (unsigned int i = 0; i < no_of_temp_sen_connected_cell[cell_id - 1]; i++)
    {
        sum += result_arr[i];
    }
    return sum / no_of_temp_sen_connected_cell[cell_id - 1];
}