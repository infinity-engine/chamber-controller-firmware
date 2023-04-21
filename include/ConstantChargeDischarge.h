#ifndef CCHDIS_H
#define CCHDIS_H
#include <Arduino.h>
#include "config_const.h"
#include "functionPrototype.h"

class ReadWriteExpAPI; // forward declaration to avoid circular depedency
class ConversationAPI;
class ConstantChargeDischarge
{
public:
    unsigned isFinished; // 0 for not finished/running, 1 for finished, 2 for stopped for safety
    struct CellMeasurement measurement;
    struct CellParameters parameters;
    struct ExperimentParameters expParamters;
    struct ChamberMeasurement chmMeas;

    // experiment related informaation
    int noOfSubExps;                     // how many rows
    int nthCurSubExp;                    // nt row or nth sub experiment 1- max sub
    unsigned char curExpStatus;          // status of that particular sub experiment
    unsigned int curRowIndex;            // points to current row index/multiplier
    unsigned int overallMultiplier;      // points to no of cycle all the sub experiment has to be repeated
    unsigned int currentMultiplierIndex; // points to current index;
    bool isExpConfigured;                // tells whether the object has been cofigured with a valid exp configuration
    bool isRowConfigured;                // tells whether the next sub exp is cofigured or not

    uint8_t overallStatus;      // when the whole experiment on this channel is finished; if any row status is other than (running or not_started) that would reflect to overall status
    unsigned long expStartTime; // starts when first sub experiment starts, will be used to refer the measured parameters time
    ConstantChargeDischarge(uint8_t channelId = 0);

    bool startCurrentSubExp();
    bool prepareForNextSubExp();
    void reset(unsigned char cell_id = 0, unsigned char mode = 2);
    void setup();
    void timeReset();
    void finish();

    bool isConAmTe; // is consistant ambient temperature throughout the experiment
    float ambTemp;

    uint8_t performAction(ReadWriteExpAPI &api, ConversationAPI &cpi);
    uint8_t perFormDriveCycle(ReadWriteExpAPI &api, unsigned long curTime = millis());
    bool placeNewSubExp(ReadWriteExpAPI *api);
    void recordData(ReadWriteExpAPI &api, ConversationAPI &cpi);
    void formRow(char *);
};
#endif // CCHDIS_H