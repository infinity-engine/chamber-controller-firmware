#ifndef ConversationAPI_H
#define ConversationAPI_H
#include <Arduino.h>

#include <SdFat.h>
extern SdFs sd;
extern FsFile dir;
extern FsFile file;

class ReadWriteExpAPI;
class ConstantChargeDischarge;

const unsigned int numChars = 300;

class ConversationAPI
{
public:
    ConversationAPI();
    void sendMsgID(const char *);
    bool isDeviceReady();
    void clearInputBuffer();
    bool isEXPAvailable();
    bool writeEXPConfig(ReadWriteExpAPI *api);
    void readInstructions(ReadWriteExpAPI *api, ConstantChargeDischarge *ccd);
    void triggerEspInt();
    void resetEspInt();
    void sendMeasurement(uint8_t channelId, char *msg);
    void incrementMultiplier(uint8_t channelID, uint8_t rowId = 0);
    void setStatus(uint8_t status, uint8_t channelID = 0, uint8_t rowId = 0);
    bool isReadyToStartEXP(String chArray);
    char receivedChars[numChars];
    boolean newData = false;
    void recvWithStartEndMarkers();
};
#endif