#include "Arduino.h"
#include"config_atmega.h"

extern const unsigned no_of_discharger_connected;
extern const unsigned fan_control_pin_location_discharger[6];
extern const uint8_t dis_cur_force[6];

void pinInit(){
    for (unsigned char i=0;i<no_of_discharger_connected;i++){
        pinMode(fan_control_pin_location_discharger[i],OUTPUT);
        digitalWrite(fan_control_pin_location_discharger[i],LOW);
    }
    for (unsigned char i=0;i<no_of_discharger_connected;i++){
        pinMode(dis_cur_force[i],OUTPUT);
        digitalWrite(dis_cur_force[i],LOW);
    }
    pinMode(S0,OUTPUT);
    pinMode(S1,OUTPUT);
    pinMode(S2,OUTPUT);
    pinMode(S3,OUTPUT);
}