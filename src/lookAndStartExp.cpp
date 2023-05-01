#include "functionPrototype.h"
#include "ConstantChargeDischarge.h"
#include "ConversationAPI.h"
#include "ReadWriteEXPAPI.h"

#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;

bool lookAndStartExp(ReadWriteExpAPI *api, ConversationAPI *cpi, ConstantChargeDischarge *expArray)
{
    if (!cpi->isDeviceReady())
    {
        Serial.println(F("ESP not ready"));
        clearLine(3);
        lcd.print(F("Network-U not ready"));
        return false;
    }
    if (!cpi->isEXPAvailable())
    {
        Serial.println(F("EXP not available"));
        clearLine(3);
        lcd.print(F("EXP not available"));
        return false;
    }
    Serial.println(F("EXP available"));
    clearLine(3);
    lcd.print(F("EXP available"));
    if (!cpi->writeEXPConfig(api))
    {
        Serial.println(F("EXP config write failed."));
        clearLine(3);
        lcd.print(F("Config write failed"));
        return false;
    }
    Serial.println(F("EXP config write success."));
    Serial.println(F("Loading Exps..."));
    clearLine(3);
    lcd.print(F("Loading EXPs.."));

    if (!api->loadExps(expArray))
    {
        Serial.println(F("Load Exp Failed!"));
        clearLine(3);
        lcd.print(F("Load EXPs failed"));
        return false;
    }
    String chArray = "";
    for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
    {
        if (expArray[i].curExpStatus == EXP_RUNNING)
        {
            if (chArray.length() > 0)
            {
                chArray += ',';
            }
            chArray += i + 1;
        }
    }

    if (!cpi->isReadyToStartEXP(chArray))
    {
        Serial.println(F("ESP is not ready for starting the exp."));
        clearLine(3);
        lcd.print(F("Network-U exp dec."));
        return false;
    }
    return true;
}