#include "functionPrototype.h"
#include "ConstantChargeDischarge.h"
#include "ConversationAPI.h"
#include "ReadWriteEXPAPI.h"

bool lookAndStartExp(ReadWriteExpAPI *api, ConversationAPI *cpi, ConstantChargeDischarge *expArray)
{
    if (!cpi->isDeviceReady())
    {
        Serial.println(F("ESP not ready"));
        return false;
    }
    if (!cpi->isEXPAvailable())
    {
        Serial.println(F("EXP not available"));
        return false;
    }
    Serial.println(F("EXP available"));
    if (!cpi->writeEXPConfig(api))
    {
        Serial.println(F("EXP config write failed."));
        return false;
    }
    Serial.println(F("EXP config write success."));
    Serial.println(F("Loading Exps..."));
    if (!api->loadExps(expArray))
    {
        Serial.println(F("Load Exp Failed!"));
        return false;
    }
    return true;
}