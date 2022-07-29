#define adc0 0
#define adc1 1
#define adc2 2
#define adc3 3

// Mux Pins
#define S0 47
#define S1 43
#define S2 41
#define S3 39

//DHT sensor
#define DHTPIN_1 29
#define DHTPIN_2 27
#define DHTPIN_3 25
#define DHTPIN_4 23
#define DHTTYPE_1 DHT22
#define DHTTYPE_2 DHT22
#define DHTTYPE_3 DHT22
#define DHTTYPE_4 DHT22

const unsigned char no_of_temp_sen_connected_cell[6] = {6, 6, 6, 6, 6, 6}; // set the no of temperature connected in each cells
const unsigned int temp_average_sample_count = 100;                       // set the no. of samples for avereging the temperature readings

//voltage sensor
const unsigned int vol_average_sample_count = 50;                         // set the no. of samples for averaging the voltage measurement

//current sensor
const unsigned int cur_average_sample_count = 250;                        // set the no. of samples for averaging the current measurement
const unsigned int acs_sensitivity = 0.1; // 0.1 - 20A range, 0.185 - 5A range, 0.066 - 30A range 

// NTC 100K parameters
const float ntc_a = 283786.2, ntc_b = 0.06593, ntc_c = 49886.0;

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
