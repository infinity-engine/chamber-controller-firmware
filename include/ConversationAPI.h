#ifndef ConversationAPI_H
#define ConversationAPI_H
#include <Arduino.h>

#include <SdFat.h>
extern SdFs sd;
extern FsFile dir;
extern FsFile file;

class ReadWriteExpAPI;
class ConstantChargeDischarge;

class ConversationAPI
{
public:
    ConversationAPI();
    void sendMsgID(const char *);
    bool isDeviceReady();
    void clearInputBuffer();
    bool isEXPAvailable();
    bool writeEXPConfig(ReadWriteExpAPI *api);
    bool outputInit();
    void readInstructions(ReadWriteExpAPI *api, ConstantChargeDischarge *ccd);
};
#endif