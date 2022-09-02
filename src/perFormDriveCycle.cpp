#include <Arduino.h>

#include "functionPrototype.h"

unsigned char perFormDriveCycle(CellParameters &parameters,CellMeasurement &measurement,ExperimentParameters &expParms,int sampleTime,unsigned long curTime){
    return 0;
    float drive_cycle[] = {0, 0.5, 2, 1.5, 1, 2.5, 1.5, 9.5, 1, 2, 8, 6, 4, 3.5, 3, 2.5, 2, 1.5, 1, 0};
    if (curTime > expParms.prevTime + sampleTime) {
        Serial.println(expParms.sampleIndicator);
        setDischargerCurrent(1, drive_cycle[expParms.sampleIndicator]);
        expParms.sampleIndicator += 1;
        if (expParms.sampleIndicator >= expParms.samples) {
          expParms.sampleIndicator = 0;
        }
        
      }
      measurement.current = getDischargerCurrent(parameters.cellId);
}