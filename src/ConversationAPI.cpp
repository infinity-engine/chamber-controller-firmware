#include "ConversationAPI.h"
#include "ReadWriteEXPAPI.h"

ConversationAPI::ConversationAPI()
{
    Serial2.begin(2000000);
}
void ConversationAPI::sendMsgID(const char *msgID)
{
    Serial2.println(msgID);
}
bool ConversationAPI::isDeviceReady()
{
    clearInputBuffer();
    Serial2.flush();
    sendMsgID((const char *)"IS_READY");
    unsigned long t = millis();
    while (millis() < t + 2000)
    {
        while (Serial2.available())
        {
            String code = Serial2.readStringUntil('\n');
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
    clearInputBuffer();
    Serial2.flush();
    sendMsgID((const char *)"IS_EXP");
    unsigned long t = millis();
    while (millis() < t + 10000)
    {
        while (Serial2.available())
        {
            String code = Serial2.readStringUntil('\n');
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
    return false;
}

bool ConversationAPI::writeEXPConfig(ReadWriteExpAPI *api)
{
    while (true)
    {
        clearInputBuffer();
        Serial2.flush();
        sendMsgID((const char *)"IS_READY");
        unsigned long t = millis();
        while (millis() < t + 10000)
        {
            bool flag = false;
            while (Serial2.available())
            {
                flag = true;
                String code = Serial2.readStringUntil('\n');
                if (code == "NULL")
                {
                    return true;
                }
                else if (code == "DIR")
                {
                    while (Serial2.available())
                    {
                        String dirName = Serial2.readStringUntil('\n');
                        if (!api->createDir(dirName.c_str()))
                            return false;
                    }
                }
                else if (code == "FILE")
                {
                    while (Serial2.available())
                    {
                        String path = Serial2.readStringUntil('\n');
                        String content = Serial2.readStringUntil('\n');
                        if (!api->writeToFile(path.c_str(), content.c_str()))
                            return false;
                    }
                }
            }
            if (flag)
                break;
        }
    }
    return false;
}

void ConversationAPI::clearInputBuffer()
{
    while (Serial2.available())
    {
        Serial2.read();
    }
}