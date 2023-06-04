#include <functionPrototype.h>
extern CallibrationParameters calParams[];
void calibrationInit()
{
    // For ACS Sensors
    calParams[0].acsOffset = 0; // for channel 1
    calParams[1].acsOffset = 0; // for channel 2
    calParams[2].acsOffset = 0; // for channel 3
    calParams[3].acsOffset = 0; // for channel 4
    calParams[4].acsOffset = 0; // for channel 5
    calParams[5].acsOffset = 0; // for channel 6

    // for Dischargers

    // for channel 1
    calParams[0].currentMultiplierIn = 1.06;
    calParams[0].currentMultiplierOut = 0.95;

    // for channel 2
    calParams[1].currentMultiplierIn = 1.06;
    calParams[1].currentMultiplierOut = 0.95;

    // for channel 3
    calParams[2].currentMultiplierIn = 1.06;
    calParams[2].currentMultiplierOut = 0.95;

    // for channel 4
    calParams[3].currentMultiplierIn = 1.06;
    calParams[3].currentMultiplierOut = 0.95;

    // for channel 5
    calParams[4].currentMultiplierIn = 1.06;
    calParams[4].currentMultiplierOut = 0.95;

    // for channel 6
    calParams[5].currentMultiplierIn = 1.06;
    calParams[5].currentMultiplierOut = 0.95;
}