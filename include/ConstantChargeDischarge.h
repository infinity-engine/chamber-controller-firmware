#ifndef CCHDIS_H
#define CCHDIS_H
#include <Arduino.h>
#include "config_const.h"
#include "ReadWriteEXPAPI.h"
#include "functionPrototype.h"

class ConstantChargeDischarge
{
public:
    unsigned isFinished; // 0 for not finished/running, 1 for finished, 2 for stopped for safety
    struct CellMeasurement measurement;
    struct CellParameters parameters;
    struct ExperimentParameters expParamters;
    struct ChamberMeasurement chmMeas;
    unsigned int curIndex;

    ConstantChargeDischarge();
    void reset(unsigned char cell_id, unsigned char mode = 2);
    void setup(bool timeReset = true);
    void finish();
    uint8_t performAction(ReadWriteExpAPI &api);
    uint8_t perFormDriveCycle(ReadWriteExpAPI &api, int sampleTime = 1000, unsigned long curTime = millis());
};
#endif //CCHDIS_H