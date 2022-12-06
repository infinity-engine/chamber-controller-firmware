#ifndef ReadWriteEXPAPI_H
#define ReadWriteEXPAPI_H

#include "config_const.h"
#include "functionPrototype.h"
#include <SdFat.h>
extern SdFs sd;
extern FsFile dir;
extern FsFile file;
#include <MemoryFree.h>
#include <Arduino.h>
#include <ArduinoJson.h>

class ReadWriteExpAPI
{
private:
    unsigned char mode_curr_exp[N_CELL_CAPABLE];
    char expName[N_CELL_CAPABLE][MAX_EXP_NAME_LENGTH]; // for output logs file in csv. first row(label).
    bool isOpDirChecked[N_CELL_CAPABLE];               // to create the output directory
    unsigned long driveCycleSampleIndicator[N_CELL_CAPABLE];// file pointer in terms of no of character
    // ignore if your experiment isn't a drive cycle or series of current
    unsigned long logSDPointer[N_CELL_CAPABLE];
public:
    unsigned char currentSubExpNo[N_CELL_CAPABLE];

    bool isHeaderWritten[N_CELL_CAPABLE];

    /**
     * @brief Construct a new Read Write Exp A P I object
     *
     */
    ReadWriteExpAPI();

    void resetAPIChannel(unsigned char cellId, const char *exp_name = "");

    bool setUpNextSubExp(unsigned char cellId, struct ExperimentParameters *expParamters);

    bool fillNextDriveCyclePortion(unsigned char cellId, struct ExperimentParameters *expParamters, uint8_t n_samples = DriveCycleBatchSize);

    bool logReadings(unsigned char cellId, char *row);
    
    void formHead(char *head, uint8_t cellId);
    
    bool cleanDir();
};

#endif //ReadWriteEXPAPI_H