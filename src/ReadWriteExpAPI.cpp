#include "ReadWriteEXPAPI.h"

/**
 * @brief Construct a new Read Write Exp A P I object
 *
 */
ReadWriteExpAPI::ReadWriteExpAPI()
{
    for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
    {
        resetAPIChannel(i);
    }
}

void ReadWriteExpAPI::resetAPIChannel(unsigned char cellId, const char *exp_name)
{
    // program is stucking after calling of this function
    cellId--;
    currentSubExpNo[cellId] = 0;
    mode_curr_exp[cellId] = 0;
    driveCycleSampleIndicator[cellId] = 0;
    strcpy(expName[cellId], exp_name);
    isHeaderWritten[cellId] = true;
    isOpDirChecked[cellId] = false;
    logSDPointer[cellId] = 0;
}

/**
 * @brief with the current experiment name. availabe on the object property
 * read through the sd card and fill the given expParams with approprite experiment setup
 * @param cellId nth cell 1-Max_cell_capable
 * @param expParamters parameters for the sub experiments
 * @return true on successful new sub exp placing
 * @return false otherwise
 */
bool ReadWriteExpAPI::setUpNextSubExp(unsigned char cellId, struct ExperimentParameters *expParamters)
{
    struct ExperimentParameters expCopy = *expParamters; // make a local copy and work on that local copy
    cellId--;
    if (strlen(expName[cellId]) == 0)
    {
        Serial.println(F("Exp name not configured"));
        return false;
    }
    StaticJsonDocument<300> doc;
    sd.chdir("/");
    if (sd.exists(expName[cellId]))
    {
        if (ISLOGENABLED)
        {
            Serial.print(expName[cellId]);
            Serial.println(F(" -> config exists."));
        }
        currentSubExpNo[cellId]++;
    }
    else
    {
        Serial.print(expName[cellId]);
        Serial.println(F(" -> config does not exist."));
        return false;
    }

    char config[MAX_EXP_NAME_LENGTH + 10] = "";
    strcat(config, (const char *)expName[cellId]);
    strcat(config, "/inputs");
    sd.chdir(config);
    const char end[20] = "_config.json";
    char buff[4] = ""; // storage to convert sub experiment no. to char array
    itoa(currentSubExpNo[cellId], buff, 10);
    strcpy(config, buff);
    strcat(config, end);
    // Serial.println(config);
    file = sd.open(config, FILE_READ);
    if (file)
    {
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            file.close();
            return false;
        }
        unsigned char mode = doc["mode"];
        float currentRate = doc["currentRate"];
        float resVal = doc["resVal"];
        float powVal = doc["powVal"];
        unsigned long timeLimit = doc["timeLimit"];
        if (mode == DriveCycle)
        {
            expCopy.total_n_samples = doc["total_n_samples"];
        }
        unsigned int multiplier = doc["multiplier"];
        float ambTemp = doc["ambTemp"];
        if (mode == Hold)
        {
            float holdVolt = doc["holdVolt"];
            expCopy.holdVolt = holdVolt;
        }

        expCopy.mode = mode;
        expCopy.resVal = resVal;
        expCopy.powVal = powVal;
        expCopy.currentRate = currentRate;
        expCopy.timeLimit = timeLimit;
        mode_curr_exp[cellId] = mode;
        expCopy.multiplier = multiplier;
        expCopy.ambTemp = ambTemp;
        isHeaderWritten[cellId] = false; // so that when writing the logs in csv file write the header row
    }
    else
    {
        Serial.println(F("Couldn't find the Sub-Exp."));
        file.close();
        return false;
    }
    file.close();
    *expParamters = expCopy; // assign the local copy back to the original reference
    return true;
}

bool ReadWriteExpAPI::fillNextDriveCyclePortion(unsigned char cellId, struct ExperimentParameters *expParamters, uint8_t n_samples)
{
    // log_(expParamters);
    struct ExperimentParameters expCopy = *expParamters; // make a local copy of the structure
    cellId--;
    // read the next set of current for the drive cycle
    // copy all the value in to the received pointer
    if (mode_curr_exp[cellId] != DriveCycle)
    {
        // only valid for drive cycle exp.
        return false;
    }
    sd.chdir("/");
    char config[MAX_EXP_NAME_LENGTH + 10] = "";
    strcpy(config, (const char *)expName[cellId]);
    strcat(config, "/inputs");
    sd.chdir(config);
    const char end[20] = "_drivecycle.csv";
    char buff[4]; // storage to convert sub experiment no. to char array
    itoa(currentSubExpNo[cellId], buff, 10);
    strcpy(config, buff);
    strcat(config, end);
    // Serial.println(config);
    file = sd.open(config, FILE_READ); // warning!! with this line there is a possible chance of heap and stack overlap because of memory overflow

    if (file)
    {
        if (file.seek(driveCycleSampleIndicator[cellId]))
        {
            for (uint8_t i = 0; i < n_samples; i++)
            {
                // clearing the previous value in case end
                expCopy.samples_batch[i] = 0;
            }
            for (uint8_t i = 0; i < n_samples and file.available(); i++)
            {
                float data = file.readStringUntil('\n').toFloat();
                // Serial.println(data, 4);
                expCopy.samples_batch[i] = data;
            }
        }
        driveCycleSampleIndicator[cellId] = file.position(); // get the postion of next byte from where it to be read or write
    }
    else
    {
        Serial.println(F("Error opening the config file."));
        file.close();
        return false;
    }
    file.close();
    *expParamters = expCopy; // assign the local copy back to the origin
    // for (uint8_t i = 0; i < 20; i++)
    // {
    //     Serial.print(expParamters->samples_batch[i]);
    //     Serial.print(" ");
    // }
    // Serial.println();
    // log_(expParamters);
    return true;
}

bool ReadWriteExpAPI::logReadings(unsigned char cellId, char *row)
{
    // on an average takes 15ms
    cellId--;
    // Serial.print(F("Available RAM "));
    // Serial.print(freeMemory());
    // Serial.println(F("Bytes"));
    sd.chdir("/");
    char path[MAX_EXP_NAME_LENGTH + 10] = "";
    sprintf(path, "%s/outputs", expName[cellId]);
    // Serial.println(path);

    if (!isOpDirChecked[cellId])
    {
        // if it is the first sub experiment of the series
        if (sd.exists(path))
        {
            // clean the existing directory
            //  to log new outputs
            FatFile cwd;
            if (!cwd.open(path))
            {
                Serial.println(F("CWD path open failed."));
            }
            if (!cwd.rmRfStar())
            {
                Serial.println(F("O/P dir exist. Remove failed."));
                return false;
            }
            Serial.println(F("Cleaned o/p dir."));
            cwd.close();
        }
        sd.chdir("/");
        if (!sd.mkdir(path))
        {
            Serial.println(F("O/P dir make failed"));
            return false;
        }
        isOpDirChecked[cellId] = true;
    }

    sd.chdir("/");

    if (!sd.chdir(path))
    {
        Serial.println(F("Dir change failed."));
        return false;
    }

    char config[15] = "";
    sprintf(config, "%d_logs.csv", currentSubExpNo[cellId]);
    // Serial.println(config);

    file = sd.open(config, O_WRONLY | O_CREAT);
    if (file)
    {
        if (!isHeaderWritten[cellId])
        {
            // write the header row first
            char head[100] = "";
            formHead(head, cellId + 1);

            // Serial.println(head);
            file.println(head);
            logSDPointer[cellId] = file.position();
            isHeaderWritten[cellId] = true;
        }
        // Serial.println(row);
        if (file.seek(logSDPointer[cellId]))
        {
            file.println(row);
            logSDPointer[cellId] = file.position();
        }
    }
    else
    {
        Serial.println(F("Output directory creation failed."));
        file.close();
        return false;
    }
    file.close();
    // sd.ls("/",LS_R);
    return true;
}

void ReadWriteExpAPI::formHead(char *head, uint8_t cellId)
{
    cellId--;
    const char *temp = "Time,Volt,Current";
    strcat(head, temp);
    for (uint8_t i = 0; i < no_of_temp_sen_connected_cell[cellId]; i++)
    {
        strcat(head, ",T");
        char buff[3] = "";
        itoa(i + 1, buff, 10);
        strcat(head, buff);
    }
    strcat(head, ",Ch_T,Ch_H");
}

bool ReadWriteExpAPI::cleanDir()
{

    if (!file)
    {
        Serial.println(F("invalid file."));
        return false;
    }
    char f_name[20];
    file = file.openNextFile(FILE_WRITE);
    while (file)
    {
        if (!file.isDirectory())
        {
            file.getName(f_name, 20);
            Serial.println(f_name);
            if (!sd.remove(f_name))
            {
                Serial.println(F("file remove failed"));
                file.close();
                return false;
            }
            else
            {
                Serial.println(F("file remove success"));
            }
        }
        file = file.openNextFile();
    }
    file.close();
    return true;
}