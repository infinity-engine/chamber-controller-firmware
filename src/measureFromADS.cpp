#include <Adafruit_ADS1X15.h>
extern Adafruit_ADS1115 ads;

float measureFromADS(unsigned char channel_id)
{
    //on careful observation it was found that each measurement takes 7.8ms when the ads115 is set on 860SPS mode
    long int adc;
    float volts;
    adc = ads.readADC_SingleEnded(channel_id);
    volts = ads.computeVolts(adc);
    return volts;
}