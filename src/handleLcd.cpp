#include "functionPrototype.h"
#include "ConstantChargeDischarge.h"
#ifndef LCD
#define LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
extern ConstantChargeDischarge exps[];
#endif
byte zero[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};
byte one[] = {
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000};
byte two[] = {
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000};
byte three[] = {
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100};
byte four[] = {
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110};
byte five[] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111};
byte customChar4[8] = {
    B00100,
    B01010,
    B10001,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};
byte customChar3[8] = {
    B00000,
    B00100,
    B01010,
    B10001,
    B00000,
    B00000,
    B00000,
    B00000};
byte customChar2[8] = {
    B00000,
    B00000,
    B00100,
    B01010,
    B10001,
    B00000,
    B00000,
    B00000};
byte customChar1[8] = {
    B00000,
    B00000,
    B00000,
    B00100,
    B01010,
    B10001,
    B00000,
    B00000};
byte completeChar[8] = {
    B00000,
    B00001,
    B00011,
    B10110,
    B01100,
    B00100,
    B00000,
    B00000};
byte stopChar[8] = {
    B00000,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000,
    B00000};
byte pausedChar[8] = {
    B00000,
    B01010,
    B01010,
    B01010,
    B01010,
    B01010,
    B00000,
    B00000};

void lcd_init()
{
    lcd.init(); // initialise the lcd
    lcd.createChar(0, zero);
    lcd.createChar(1, one);
    lcd.createChar(2, two);
    lcd.createChar(3, three);
    lcd.createChar(4, four);
    lcd.createChar(5, five);

    lcd.createChar(6, customChar1);
    lcd.createChar(7, customChar2);
    lcd.createChar(8, customChar3);
    lcd.createChar(9, customChar4);

    lcd.createChar(10, completeChar);
    lcd.createChar(11, pausedChar);
    lcd.createChar(12, stopChar);
    lcd.backlight(); // turn on backlight
    lcd.clear();
    lcd.setCursor(0, 0); // column,row
    lcd.print(F("Battery Test Chamber"));
    delay(100);
    lcd.setCursor(0, 2);
    lcd.print(F("Starting Engine"));
    clearLine(3);
    for (uint8_t i = 0; i < 80; i++)
    {
        updateProgressBar(i, 80, 3);
        delay(1);
    }
}

/**
 * @brief clear the line and set the cursor at the beginning
 * of that line
 *
 * @param line
 */
void clearLine(uint8_t line)
{
    lcd.setCursor(0, line);
    lcd.print(F("                    "));
    lcd.setCursor(0, line);
}

void updateLCDView(bool force)
{
    unsigned int lcdDelay = 2000;
    static unsigned long prevUpdate = millis();

    if (millis() - prevUpdate > lcdDelay || force)
    {
        prevUpdate = millis();
        // for channel 1-6
        handleStatusForChannel(1, 3, 1);
        handleStatusForChannel(2, 8, 1);
        handleStatusForChannel(3, 13, 1);
        handleStatusForChannel(4, 18, 1);
        handleStatusForChannel(5, 3, 2);
        handleStatusForChannel(6, 8, 2);
    }
    for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
    {
        if (&exps[i] && exps[i].overallStatus == EXP_RUNNING)
        {
            lcd.setCursor(2, 3);
            lcd.print(exps[i].chmMeas.avgTemp);
            lcd.setCursor(14, 3);
            lcd.print(exps[i].chmMeas.avgHum);
            break;
        }
    }
}

void handleStatusForChannel(uint8_t channelId, uint8_t y, uint8_t x)
{
    static uint8_t prevFanPos[6] = {6, 7, 8, 9, 8, 7}; // randomized
    channelId--;
    lcd.setCursor(y, x);
    if (&exps[channelId])
    {
        if (exps[channelId].overallStatus == EXP_RUNNING)
        {
            lcd.write(prevFanPos[channelId]++);
            if (prevFanPos[channelId] > 9)
            {
                prevFanPos[channelId] = 6;
            }
        }
        else if (exps[channelId].overallStatus == EXP_FINISHED)
            lcd.write(10);

        else if (exps[channelId].overallStatus == EXP_STOPPED)
            lcd.write(12);

        else if (exps[channelId].overallStatus == EXP_PAUSED)
            lcd.write(11);
    }
    else
        lcd.print("_");
}

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
    else if (count + 1 != totalCount)
    {
        lcd.setCursor(number, lineToPrintOn);
        lcd.write(remainder);
    }
    else if (number < 16)
    {
        for (int j = number + 1; j <= 16; j++)
        {
            lcd.setCursor(j, lineToPrintOn);
            lcd.write(0);
        }
    }
}

void updateLCDArrow(uint8_t startIndex, uint8_t width, uint8_t lineNo, uint16_t interval)
{
    static uint8_t prevPos = 0;
    static uint32_t lastUpdateTime = millis();
    if (millis() - lastUpdateTime < interval)
        return;
    lastUpdateTime = millis();

    lcd.setCursor(startIndex + prevPos, lineNo);
    lcd.print(F("  ")); // clears the previous arrow
    prevPos++;
    if (prevPos >= width)
        prevPos = 0;
    lcd.setCursor(startIndex + prevPos, lineNo);
    lcd.print(F("->"));
}