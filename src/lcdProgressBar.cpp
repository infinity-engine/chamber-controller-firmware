#include <Arduino.h>

#ifndef LCD
#define LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#endif
extern LiquidCrystal_I2C lcd;
extern byte zero;
extern byte one;
extern byte two;
extern byte three;
extern byte four;
extern byte five;
void updateProgressBar(unsigned long count, unsigned long totalCount, int lineToPrintOn)
{
    double factor = totalCount / 100.0; // See note above!
    int percent = (count + 1) / factor;
    int number = percent / 5;
    int remainder = percent % 5;
    if (number > 0)
    {
        for (int j = 0; j < number; j++)
        {
            lcd.setCursor(j, lineToPrintOn);
            lcd.write(5);
        }
    }
    if (count +1 != totalCount){
    lcd.setCursor(number, lineToPrintOn);
    lcd.write(remainder);
    }
    if (number < 16)
    {
        for (int j = number + 1; j <= 16; j++)
        {
            lcd.setCursor(j, lineToPrintOn);
            lcd.write(0);
        }
    }
}