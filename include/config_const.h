#include "config_atmega.h"

//A const global variable has internal linkage by default. 
//If you want the variable to have external linkage, apply the extern keyword to the definition, and to all other declarations in other files:

extern const unsigned char no_of_temp_sen_connected_cell[6] = {6, 6, 6, 6, 6, 6}; // set the no of temperature connected in each cells
extern const unsigned int temp_average_sample_count = 100;                       // set the no. of samples for avereging the temperature readings

//voltage sensor
extern const unsigned int vol_average_sample_count = 50;                         // set the no. of samples for averaging the voltage measurement

//current sensor
extern const unsigned int cur_average_sample_count = 250;                        // set the no. of samples for averaging the current measurement
extern const float acs_sensitivity = 0.1; // 0.1 - 20A range, 0.185 - 5A range, 0.066 - 30A range 

// NTC 100K parameters
extern const float ntc_a = 283786.2, ntc_b = 0.06593, ntc_c = 49886.0;


//DHT sensor
extern const unsigned char no_of_dht_sensor_connected = 4;//Max 4

// In fromat of s0,s1,s2,s3.
//  s0 - LSB and s1 - MSB
//  These are the signals which is required for CD74HC module to point to desired channel
bool cell_temperature_addresses[6][6][4] =
    {
        {
            {false, false, false, false}, {true, false, false, false}, {false, true, false, false}, {true, true, false, false}, {false, false, true, false}, {true, false, true, false} // cell 1 -> 0,1,2,3,4,5
        },
        {
            {false, true, true, false}, {true, true, true, false}, {false, false, false, true}, {true, false, false, true}, {false, true, false, true}, {true, true, false, true} // cell 2 -> 6,7,8,9,10,11
        },
        {
            {false, false, true, true}, {true, false, true, true}, {false, true, true, true}, {true, true, true, true}, {false, false, false, false}, {true, false, false, false} // cell 3 -> 12,13,14,15,0,1
        },
        {
            {false, true, false, false}, {true, true, false, false}, {false, false, true, false}, {true, false, true, false}, {false, true, true, false}, {true, true, true, false} // cell 4 -> 2,3,4,5,6,7
        },
        {
            {false, false, false, true}, {true, false, false, true}, {false, true, false, true}, {true, true, false, true}, {false, false, true, true}, {true, false, true, true} // cell 5 -> 8,9,10,11,12,13
        },
        {
            {false, true, true, true}, {true, true, true, true}, {false, false, false, false}, {true, false, false, false}, {false, true, false, false}, {true, true, false, false} // cell 6 -> 14,15,0,1,2,3
        }};

bool cell_voltage_addresses[6][4] =
    {
        {false, false, false, false}, // cell 1 voltage -> 0
        {true, false, false, false},  // cell 2 voltage -> 1
        {false, true, false, false},  // cell 3 voltage -> 2
        {true, true, false, false},   // cell 4 voltage -> 3
        {false, false, true, false},  // cell 5 voltage -> 4
        {true, false, true, false}    // cell 6 voltage -> 5
};

bool cell_current_addresses[6][4] =
    {
        {false, false, false, false}, // cell 1 voltage -> 0
        {true, false, false, false},  // cell 2 voltage -> 1
        {false, true, false, false},  // cell 3 voltage -> 2
        {true, true, false, false},   // cell 4 voltage -> 3
        {false, false, true, false},  // cell 5 voltage -> 4
        {true, false, true, false}    // cell 6 voltage -> 5
};

// Location for all the cell temperature sensor, on different ads pins of ads 1115
unsigned char tem_sen_ads_location[6][6] =
    {
        {adc1, adc1, adc1, adc1, adc1, adc1}, // cell 1
        {adc1, adc1, adc1, adc1, adc1, adc1}, // cell 2
        {adc1, adc1, adc1, adc1, adc2, adc2}, // cell 3
        {adc2, adc2, adc2, adc2, adc2, adc2}, // cell 4
        {adc2, adc2, adc2, adc2, adc2, adc2}, // cell 5
        {adc2, adc2, adc3, adc3, adc3, adc3}  // cell 6
};

unsigned char vol_sen_ads_location[6] = {adc0, adc0, adc0, adc0, adc0, adc0}; // ADS Pins for cell voltage measurement
unsigned char cur_sen_ads_location[6] = {adc0, adc0, adc0, adc0, adc0, adc0}; // ADS Pins for cell current measurement

extern const unsigned char cell_relay_location[6] = {38,36,34,32,30,28};//on which pin of arduino' mega relay's are connected
extern const unsigned char chamber_heater_relay_pin = 42;
extern const unsigned char chamber_cooler_relay_pin = 40;

extern const unsigned chip_select_pin_location_discharger[6] = {7,9,10,14,15,22};
extern const unsigned fan_control_pin_location_discharger[6] = {24,26,37,44,46,47};
extern const unsigned temp_measure_pin_location_discharger[6] = {A6,A7,A8,A9,A10,A11};
extern const unsigned cur_measure_pin_location_discharger[6] = {A0,A1,A2,A3,A4,A5};

extern const unsigned no_of_discharger_connected = 6;