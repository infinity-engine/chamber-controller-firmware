#include <Arduino.h>
#include "functionPrototype.h"
#include "config_const.h"

/**
 * @brief
 * cell_id starts from 1,2...6
 * time consumption = 8ms/sample/sensor
 * 6 sensors and 2 samples == 97ms
 * @param cell_id
 * @param temps
 * @return float*
 */
float *measureCellTemperature(unsigned char cell_id, float *temps)
{
    // cell_id starts from 1,2...6
    // time consumption = 8ms/sample/sensor

    // 6 sensors and 2 samples == 97ms

    float V_0 = V_Ref;    // Volt,, for the given smps it's that voltage
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
        delay(AcsSettleDelay); // give some time to acs to settle
        for (unsigned int i = 0; i < temp_average_sample_count; i++)
        {
            raw = measureFromADS(tem_sen_ads_location[cell_id - 1][curr_sen]);
            // Serial.println(raw);
            sum += raw;
        }

        // get the average reading
        sum /= float(temp_average_sample_count);

        // convert into  °C formula provided by datasheet ntc 100k
        float t = (-1.0 / ntc_b) * (log(((R_1 * sum) / (ntc_a * (V_0 - sum))) - (ntc_c / ntc_a)));
        // Serial.println(t);

        temps[curr_sen] = getMovingAverage(t, temps[curr_sen]);
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