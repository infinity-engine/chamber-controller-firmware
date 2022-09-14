#include <Arduino.h>
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif

#ifndef MEMORY
#define MEMORY
#include <MemoryFree.h>
#endif

unsigned char perFormDriveCycle(CellParameters &parameters, CellMeasurement &measurement, ExperimentParameters &expParms, int sampleTime, unsigned long curTime)
{
  unsigned status = 0;//0 for not finsished, 1 for finished, 2 for stopped
  unsigned int cycle_size = expParms.samples;
  float drive_cycle[cycle_size] = {0, 0.5, 2, 1.5, 1, 2.5, 1.5, 3.5, 1, 2, 1, 1, 1.5, 1.5, 1, 1.5, 2, 1.5, 1, 0};
  if (curTime > expParms.prevTime + sampleTime)
  {
    // for some reason whenever I am accessing drive_cycle[expParms.sampleIndicator] crashing
    // About segfaults -- just check the boundary values of array indices and you ought to be OK. If you're using pointers, then please be careful with pointer arithmetic.
    unsigned int indicator = expParms.sampleIndicator;
    indicator = indicator % cycle_size;//this is must to prevent segmentations error
    //Serial.println(indicator);
    //Serial.println(drive_cycle[indicator]);
    setDischargerCurrent(1, drive_cycle[indicator]);
    expParms.sampleIndicator += 1;
    //Serial.println(expParms.sampleIndicator);
    if (expParms.sampleIndicator >= expParms.samples)
    {
      //expParms.sampleIndicator = 0;
      status = 1;
    }
    expParms.prevTime = curTime;
  }
  measurement.current = getDischargerCurrent(parameters.cellId);
  return status;
}