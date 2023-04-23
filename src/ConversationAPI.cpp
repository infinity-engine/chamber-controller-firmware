#include "ConversationAPI.h"
#include "ReadWriteEXPAPI.h"
#include "ConstantChargeDischarge.h"

#include <HardwareSerial.h>

ConversationAPI::ConversationAPI()
{
    Serial2.begin(115200);
}

void ConversationAPI::sendMsgID(const char *msgID)
{
    clearInputBuffer();
    Serial2.print(msgID);
    Serial2.print('\n');
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
    sendMsgID("IS_EXP");
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
}

bool ConversationAPI::writeEXPConfig(ReadWriteExpAPI *api)
{
    Serial.println(F("Reading and writing it on SD Card...."));
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
                String code = Serial2.readStringUntil('\n');
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
                        String dirName = Serial2.readStringUntil('\n');
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
                        String path = Serial2.readStringUntil('\n');
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
                else if (code == "INVALID")
                {
                    return false;
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

void ConversationAPI::triggerEspInt()
{
    digitalWrite(ESP_INT_PIN, HIGH);
}

void ConversationAPI::resetEspInt()
{
    digitalWrite(ESP_INT_PIN, LOW);
}

bool ConversationAPI::isReadyToStartEXP(String chArray)
{
    String msg = "START\n" + chArray;
    sendMsgID(msg.c_str());
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

void ConversationAPI::sendMeasurement(uint8_t channelId, char *msg)
{
    if (Serial2.availableForWrite() == 0)
    {
        Serial.println("Serial2 buffer is full");
    }

    // triggerEspInt();
    Serial2.print("<");
    Serial2.print(channelId);
    Serial2.print("\nMT\n");
    Serial2.print(msg);
    Serial2.print("\n>");
    // Serial.println(Serial2.readStringUntil('\n'));
    // resetEspInt();
}

void ConversationAPI::incrementMultiplier(uint8_t channelID, uint8_t rowId)
{
    if (Serial2.availableForWrite() == 0)
    {
        Serial.println("Serial2 buffer is full");
    }

    // triggerEspInt();
    Serial2.print('<');
    Serial2.print(channelID);
    Serial2.print("\nIM\n");
    Serial2.print(rowId);
    Serial2.print("\n>");
    // resetEspInt();
}

void ConversationAPI::setStatus(uint8_t status, uint8_t channelID, uint8_t rowId)
{
    if (Serial2.availableForWrite() == 0)
    {
        Serial.println("Serial2 buffer is full");
    }

    // triggerEspInt();
    Serial2.print("<");
    Serial2.print(channelID);
    Serial2.print("\nSS\n");
    Serial2.print(rowId);
    Serial2.print('\n');
    Serial2.print(status);
    Serial2.print("\n>");
    // resetEspInt();
}

void ConversationAPI::readInstructions(ReadWriteExpAPI *api, ConstantChargeDischarge *ccd)
{
    while (Serial2.available())
    {
        String msgID = Serial2.readStringUntil('\n');
        if (msgID == "START")
        {
            String expName = Serial2.readStringUntil('\n');
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

void ConversationAPI::recvWithStartEndMarkers()
{
    static boolean recvInProgress = false;
    static unsigned int ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial2.available() > 0 && newData == false)
    {
        rc = Serial2.read();

        if (recvInProgress == true)
        {
            if (rc != endMarker)
            {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars)
                {
                    ndx = numChars - 1;
                }
            }
            else
            {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker)
        {
            recvInProgress = true;
        }
    }
}