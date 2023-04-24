#include <Adafruit_ADS1X15.h>
extern Adafruit_ADS1115 ads;

float measureFromADS(unsigned char channel_id)
{
    // on careful observation it was found that each measurement takes 7.8ms when the ads115 is set on 860SPS mode
    // takes 13ms on 475sps
    // 24ms on 250sps
    // 33ms on 128sps
    // if you call ads too frequently you will get 0 value so you gotta wait
    // specially when you measure from diff channel
    long int adc;
    float volts;
    unsigned long t = millis();
    adc = ads.readADC_SingleEnded(channel_id);
    volts = ads.computeVolts(adc); //this would also be to 0 on unsuccess but that can not distinguish betwwen 0 value
    // on onsuccessful transoformation it takes less time so detect that and ask again
    if (millis() - t < 2)
    {
        // on un-successfull transformation
        volts = measureFromADS(channel_id);
    }
    // Serial.print(volts);
    // Serial.print("\ttime \t");
    // Serial.println(millis() - t);
    return volts;
}