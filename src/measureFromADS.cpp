#include <Adafruit_ADS1X15.h>
extern Adafruit_ADS1115 ads;

float measureFromADS(unsigned char channel_id)
{
    long int adc;
    float volts;
    adc = ads.readADC_SingleEnded(channel_id);
    volts = ads.computeVolts(adc);
    return volts;
}