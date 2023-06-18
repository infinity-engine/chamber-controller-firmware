#ifndef CONFIG_CONST
#define CONFIG_CONST
#include <Arduino.h>

extern uint8_t no_of_temp_sen_connected_cell[];
extern const uint16_t temp_average_sample_count;
extern const uint16_t vol_average_sample_count;
extern const uint16_t cur_average_sample_count;
extern const float acs_sensitivity;
extern const float ntc_a, ntc_b, ntc_c;
extern uint8_t no_of_dht_sensor_connected;
extern const byte *const cell_temperature_addresses[];

extern const byte cell_voltage_addresses[6];

extern const byte cell_current_addresses[6];
extern const uint8_t tem_sen_ads_location[6][6];

extern const uint8_t dis_cur_force[];

extern const uint8_t vol_sen_ads_location[];
extern const uint8_t cur_sen_ads_location[];

extern const uint8_t cell_relay_location[];
extern const uint8_t chamber_heater_relay_pin;
extern const uint8_t chamber_cooler_relay_pin;

extern const uint8_t chip_select_pin_location_discharger[];
extern const uint8_t fan_control_pin_location_discharger[];
extern const uint8_t temp_measure_pin_location_discharger[];
extern const uint8_t cur_measure_pin_location_discharger[];

extern const uint8_t no_of_discharger_connected;
extern const int *const store[];

extern const unsigned int sample_update_delay; // in ms;used to limit how frequently the experiment data are logged at the time of charging experiment

extern bool IS_ESP_CRASHED;

#endif // CONFIG_CONST