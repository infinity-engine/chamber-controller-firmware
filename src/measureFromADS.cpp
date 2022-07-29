#ifndef ADAFRUIT_ADS1X15_H
#define ADAFRUIT_ADS1X15_H
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;
#endif

float measureFromADS(unsigned char channel_id){
    long int adc;
    float volts;
    adc = ads.readADC_SingleEnded(channel_id);
    volts = ads.computeVolts(adc);
    return volts;
}