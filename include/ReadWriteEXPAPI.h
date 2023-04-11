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

class ConstantChargeDischarge; // forward declaration to avoid circular depedency

class ReadWriteExpAPI
{
private:
    char expName[MAX_EXP_NAME_LENGTH];   // for output logs file in csv. first row(label).
    bool isOpDirChecked[N_CELL_CAPABLE]; // to create the output directory
    unsigned long dcPtr[N_CELL_CAPABLE]; // file pointer to point on which drive cycle it is currently was
    // file pointer in terms of no of character
    // ignore if your experiment isn't a drive cycle or series of current
    unsigned long logSDPointer[N_CELL_CAPABLE];
    bool isHeaderWritten[N_CELL_CAPABLE];

public:
    /**
     * @brief Construct a new Read Write Exp A P I object
     *
     */
    ReadWriteExpAPI();

    void reset(char *expNameNew = (char *)"");

    bool setup(ConstantChargeDischarge *ccd);

    bool setUpNextSubExp(ConstantChargeDischarge *ccd);

    bool fillNextDriveCyclePortion(ConstantChargeDischarge *ccd, uint8_t n_samples = DriveCycleBatchSize);

    bool logReadings(ConstantChargeDischarge *ccd, char *row);

    void formHead(char *head, uint8_t cellId);

    bool cleanDir();

    bool createDir(const char *, bool clean = true);
    bool writeToFile(const char *, char *);
    bool writeToFileStream(const char *, Stream *stream, char readUntil = '\n');

    bool loadExps(ConstantChargeDischarge *expArray);
    int bytesAvailable(Stream *stream);
};

#endif // ReadWriteEXPAPI_H