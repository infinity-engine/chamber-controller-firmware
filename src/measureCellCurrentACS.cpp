#include <Arduino.h>
#include "functionPrototype.h"
#include "config_const.h"

float measureCellCurrentACS(unsigned char cell_id)
{
    // cell_id starts from 1,2...6
    // return measured current in Ampere from ACS sensor
    // takes around 8ms/sample
    // 107 ms for 10 samples
    float V_middle = V_Ref / 2; // chanege accroding to the input voltage of +5V in arduino or the circuit
    float offset = 0;           // after calibration
    float sum = 0.0;
    // set the proper channel for for mux to ads
    bool address[4];
    byte tem = pgm_read_byte(&cell_current_addresses[cell_id - 1]);
    for (uint8_t i = 0; i < 4; i++)
    {
        address[i] = bitRead(tem, 7 - i);
    }
    channelTheMux(address);
    delay(AcsSettleDelay); // give some time to acs to settle
    for (unsigned int i = 0; i < cur_average_sample_count; i++)
    {
        sum += measureFromADS(cur_sen_ads_location[cell_id - 1]); // takes most of the time because of using adc
    }
    float acsValue = float(sum / cur_average_sample_count);
    // Serial.println(acsValue, 5);
    return -float((V_middle - acsValue + offset) / acs_sensitivity);
}