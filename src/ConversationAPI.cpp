#include "ConversationAPI.h"
#include "ReadWriteEXPAPI.h"
#include "ConstantChargeDischarge.h"

ConversationAPI::ConversationAPI()
{
    Serial2.begin(115200);
}

void ConversationAPI::sendMsgID(const char *msgID)
{
    clearInputBuffer();
    Serial2.println(msgID);
    Serial2.flush();
}

bool ConversationAPI::isDeviceReady()
{
    sendMsgID("IS_READY");
    unsigned long t = millis();
    while (millis() < t + 2000)
    {
        while (Serial2.available())
        {
            String code = Serial2.readStringUntil('\r');

            if (code == "YES")
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool ConversationAPI::isEXPAvailable()
{
    sendMsgID("IS_EXP");
    unsigned long t = millis();
    while (millis() < t + 10000)
    {
        while (Serial2.available())
        {
            String code = Serial2.readStringUntil('\r');
            if (code == "YES")
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool ConversationAPI::writeEXPConfig(ReadWriteExpAPI *api)
{
    bool isExpNameReceived = false;
    while (true)
    {
        sendMsgID("SEND_TEST");
        unsigned long t = millis();
        while (millis() < t + 10000)
        {
            bool flag = false;
            while (Serial2.available())
            {
                flag = true;
                String code = Serial2.readStringUntil('\r');
                unsigned long t2 = millis();
                if (code == "NULL")
                {
                    clearInputBuffer();
                    return true;
                }
                else if (code == "DIR")
                {
                    while (Serial2.available() || millis() < t2 + 200)
                    {
                        String dirName = Serial2.readStringUntil('\r');
                        // for the first time this would be the directory name
                        if (!isExpNameReceived)
                        {
                            String prefix = dirName.substring(0, dirName.indexOf('/')); // first was testId/inputs/....
                            api->reset((char *)prefix.c_str());
                            isExpNameReceived = true;
                        }
                        if (!api->createDir(dirName.c_str()))
                            return false;
                    }
                }
                else if (code == "FILE")
                {
                    while (Serial2.available() || millis() < t2 + 200)
                    {
                        String path = Serial2.readStringUntil('\r');
                        if (path.endsWith(".csv"))
                        { // for drive cycle file
                            char terminator = '\r';
                            if (!api->writeToFileStream(path.c_str(), &Serial2, terminator))
                                return false;
                        }
                        else
                        {
                            char buffer[300];
                            int bytesRead = Serial2.readBytesUntil('\n', buffer, sizeof(buffer));
                            // you don't need to provide any delay here as readBytesUntil is already provided with 1000ms delay
                            buffer[bytesRead] = '\0'; // add null terminator at end of string
                            if (!api->writeToFile(path.c_str(), buffer))
                                return false;
                        }
                    }
                }
            }
            if (flag)
                break;
        }
    }
    return false;
}

bool ConversationAPI::outputInit()
{
    sendMsgID((const char *)"IN_SD_EXP");
    unsigned long t = millis();
    if (!isDeviceReady())
        return false;
    while (millis() < t + 10000)
    {
        while (Serial2.available())
        {
            String code = Serial2.readStringUntil('\r');
            if (code == "YES")
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

void ConversationAPI::clearInputBuffer()
{
    int d = 50; // wait for 50ms atleast
    unsigned long t = millis();
    while (millis() < t + d)
    {
        while (Serial2.available())
        {
            Serial2.read();
        }
    }
}

void ConversationAPI::readInstructions(ReadWriteExpAPI *api, ConstantChargeDischarge *ccd)
{
    while (Serial2.available())
    {
        String msgID = Serial2.readStringUntil('\r');
        if (msgID == "START")
        {
            String expName = Serial2.readStringUntil('\r');
            Serial.print(F("Received instruction to start exp "));
            Serial.println(expName);
            if (expName.length() > 0)
            {
                api->reset((char *)"c7e7"); // set the expname
                // api.loadExps(exps); call it after
            }
        }
    }
}