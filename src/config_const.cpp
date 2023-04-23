#ifndef CONFIG_ATMEGA
#include "config_atmega.h"
#include "config_const.h"
#endif

#include <Arduino.h>

uint8_t no_of_temp_sen_connected_cell[6] = {6, 6, 6, 6, 6, 6}; // set the no of temperature connected in each cells
extern const uint16_t temp_average_sample_count = 2;           // set the no. of samples for avereging the temperature readings

// voltage sensor
extern const uint16_t vol_average_sample_count = 5; // set the no. of samples for averaging the voltage measurement

// current sensor
extern const uint16_t cur_average_sample_count = 10; // set the no. of samples for averaging the current measurement
extern const float acs_sensitivity = 0.1;            // 0.1 - 20A range, 0.185 - 5A range, 0.066 - 30A range

// NTC 100K parameters
/*
float a = 283786.2;
float b = 0.06593;
float c = 49886.0;
*/
extern const float ntc_a = 283786.2, ntc_b = 0.06593, ntc_c = 39886.0; // 0.4 deg drop per 1000 drop

// DHT sensor
uint8_t no_of_dht_sensor_connected = 4; // Max 4

// In fromat of s0,s1,s2,s3.
//  s0 - LSB and s3 - MSB
//  These are the signals which is required for CD74HC module to point to desired channel
const byte T_Address_1[] PROGMEM = {B00000000, B10000000, B01000000, B11000000, B00100000, B10100000}; // cell 1 -> 0,1,2,3,4,5
const byte T_Address_2[] PROGMEM = {B01100000, B11100000, B00010000, B10010000, B01010000, B11010000}; // cell 2 -> 6,7,8,9,10,11
const byte T_Address_3[] PROGMEM = {B00110000, B10110000, B01110000, B11110000, B00000000, B10000000}; // cell 3 -> 12,13,14,15,0,1
const byte T_Address_4[] PROGMEM = {B01000000, B11000000, B00100000, B10100000, B01100000, B11100000}; // cell 4 -> 2,3,4,5,6,7
const byte T_Address_5[] PROGMEM = {B00010000, B10010000, B01010000, B11010000, B00110000, B10110000}; // cell 5 -> 8,9,10,11,12,13
const byte T_Address_6[] PROGMEM = {B01110000, B11110000, B00000000, B10000000, B01000000, B11000000}; // cell 6 -> 14,15,0,1,2,3

extern const byte *const cell_temperature_addresses[] PROGMEM =
    {
        T_Address_1, T_Address_2, T_Address_3, T_Address_4, T_Address_5, T_Address_6};

extern const byte cell_voltage_addresses[6] PROGMEM =
    {
        // LSB->MSB
        B00000000, //{false, false, false, false}, // cell 1 voltage -> 0
        B10000000, //{true, false, false, false},  // cell 2 voltage -> 1
        B01000000, //{false, true, false, false},  // cell 3 voltage -> 2
        B11000000, //{true, true, false, false},   // cell 4 voltage -> 3
        B00100000, //{false, false, true, false},  // cell 5 voltage -> 4
        B10100000, //{true, false, true, false}    // cell 6 voltage -> 5
};

extern const byte cell_current_addresses[6] PROGMEM =
    {
        // LSB->MSB
        // s0-s3
        B00010000, //{false, false, false, true}, // cell 1 current -> 8
        B10010000, //{true, false, false, true},  // cell 2 current -> 9
        B01010000, //{false, true, false, true},  // cell 3 current -> 10
        B11010000, //{true, true, false, true},   // cell 4 current -> 11
        B00110000, //{false, false, true, true},  // cell 5 current -> 12
        B10110000  //{true, false, true, true}    // cell 6 current -> 13
};

// Location for all the cell temperature sensor, on different ads pins of ads 1115
extern const uint8_t tem_sen_ads_location[6][6] =
    {
        {adc1, adc1, adc1, adc1, adc1, adc1}, // cell 1
        {adc1, adc1, adc1, adc1, adc1, adc1}, // cell 2
        {adc1, adc1, adc1, adc1, adc2, adc2}, // cell 3
        {adc2, adc2, adc2, adc2, adc2, adc2}, // cell 4
        {adc2, adc2, adc2, adc2, adc2, adc2}, // cell 5
        {adc2, adc2, adc3, adc3, adc3, adc3}  // cell 6
};

extern const uint8_t dis_cur_force[6] = {A12, A13, A14, A15, 48, 49};

extern const uint8_t vol_sen_ads_location[6] = {adc0, adc0, adc0, adc0, adc0, adc0}; // ADS Pins for cell voltage measurement
extern const uint8_t cur_sen_ads_location[6] = {adc0, adc0, adc0, adc0, adc0, adc0}; // ADS Pins for cell current measurement

extern const uint8_t cell_relay_location[6] = {38, 36, 34, 32, 30, 28}; // on which pin of arduino' mega relay's are connected
extern const uint8_t chamber_heater_relay_pin = 42;
extern const uint8_t chamber_cooler_relay_pin = 40;

extern const uint8_t chip_select_pin_location_discharger[6] = {7, 9, 10, 14, 15, 22};
extern const uint8_t fan_control_pin_location_discharger[6] = {24, 26, 37, 44, 46, 47};
extern const uint8_t temp_measure_pin_location_discharger[6] = {A6, A7, A8, A9, A10, A11};
extern const uint8_t cur_measure_pin_location_discharger[6] = {A0, A1, A2, A3, A4, A5};

extern const uint8_t no_of_discharger_connected = 6;

extern const unsigned int sample_update_delay = 1500;
