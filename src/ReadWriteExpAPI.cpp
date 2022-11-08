#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef CONFIG_CONST
#define CONFIG_CONST
#include "config_const.h"
#endif

#ifndef PROTOTYPE
#define PROTOTYPE
#include "functionPrototype.h"
#endif

#include <SPI.h>
#include <SdFat.h>
extern SdFs sd;
extern FsFile dir;
extern FsFile file;

#ifndef MEMORY
#define MEMORY
#include <MemoryFree.h>
#endif


class ReadWriteExpAPI
{
private:
    unsigned char currentSubExpNo[N_CELL_CAPABLE];
    unsigned char mode_curr_exp[N_CELL_CAPABLE];
    char expName[N_CELL_CAPABLE][MAX_EXP_NAME_LENGTH];
    bool isHeaderWritten[N_CELL_CAPABLE]; // for output logs file in csv. first row(label).
    bool isOpDirChecked[N_CELL_CAPABLE];  // to create the output directory
    unsigned long driveCycleSampleIndicator[N_CELL_CAPABLE];
    // file pointer in terms of no of character
    // ignore if your experiment isn't a drive cycle or series of current
public:
    /**
     * @brief Construct a new Read Write Exp A P I object
     *
     */
    ReadWriteExpAPI()
    {
        for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
        {
            resetAPIChannel(i);
        }
    }

    void resetAPIChannel(unsigned char cellId, const char *exp_name = "")
    {
        // program is stucking after calling of this function
        cellId--;
        currentSubExpNo[cellId] = 0;
        mode_curr_exp[cellId] = 0;
        driveCycleSampleIndicator[cellId] = 0;
        strcpy(expName[cellId], exp_name);
        isHeaderWritten[cellId] = true;
        isOpDirChecked[cellId] = false;
    }

    /**
     * @brief with the current experiment name. availabe on the object property
     * read through the sd card and fill the given expParams with approprite experiment setup
     * @param cellId nth cell 1-Max_cell_capable
     * @param expParamters parameters for the sub experiments
     * @return true on successful new sub exp placing
     * @return false otherwise
     */
    bool setUpNextSubExp(unsigned char cellId, struct ExperimentParameters *expParamters)
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
            Serial.print(expName[cellId]);
            Serial.println(F(" -> config exists."));
            currentSubExpNo[cellId]++;
        }
        else
        {
            Serial.print(expName[cellId]);
            Serial.println(F(" -> config does not exist."));
            return false;
        }
        sd.chdir((String)expName[cellId] + "/inputs");
        const char end[20] = "_config.json";
        char buff[4]; // storage to convert sub experiment no. to char array
        itoa(currentSubExpNo[cellId], buff, 10);
        char config[20];
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
            expCopy.mode = mode;
            if (mode == DriveCycle)
            {
                expCopy.total_n_samples = doc["total_n_samples"];
            }
            expCopy.resVal = resVal;
            expCopy.powVal = powVal;
            expCopy.currentRate = currentRate;
            expCopy.timeLimit = timeLimit;
            mode_curr_exp[cellId] = mode;
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

    bool fillNextDriveCyclePortion(unsigned char cellId, struct ExperimentParameters *expParamters, uint8_t n_samples = DriveCycleBatchSize)
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
        sd.chdir("/" + (String)expName[cellId] + "/inputs");
        const char end[20] = "_drivecycle.csv";
        char buff[4]; // storage to convert sub experiment no. to char array
        itoa(currentSubExpNo[cellId], buff, 10);
        char config[20 + 4];
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

    bool logReadings(unsigned char cellId, struct CellMeasurement *cellM, struct ChamberMeasurement *chaM)
    {
        struct CellMeasurement cellMCopy = *cellM;
        struct ChamberMeasurement chaMCopy = *chaM;
        // Serial.println(sizeof(chaMCopy));
        // Serial.println(sizeof(cellMCopy));
        cellId--;
        // Serial.print("Free SRAM ");
        // Serial.print(getFreeSram());
        // Serial.println(" Bytes");


        char path[MAX_EXP_NAME_LENGTH + 1];
        sprintf(path,"/%s",expName[cellId]);
        if(!sd.chdir(path)){
            Serial.println(F("Dir change failed."));
            return false;
        }
        //Serial.println(path);
        if (!isOpDirChecked[cellId])
        {
            // if it is the first sub experiment of the series
            if (sd.exists(path))
            {
                // remove the existing directory
                //  to log new outpurs
                file = sd.open("outputs");
                if (!cleanDir())
                {
                    Serial.println(F("O/P dir exist. Remove failed."));
                    return false;
                }
                Serial.println(F("Cleaned o/p dir."));
            }
            else
            {
                sd.mkdir(path);
            }
            isOpDirChecked[cellId] = true;
        }
        if (!sd.chdir(path))
        {
            Serial.println(F("Dir change failed."));
            return false;
        }
        char config[15];
        sprintf(config,"%d_logs.csv",currentSubExpNo[cellId]);
        //Serial.println(config);

        char head[200] = "";
        strcat(head, "o");

        file = sd.open(config, O_WRONLY | O_CREAT);
        if (file)
        {
            Serial.println("DEAD Zone");
            Serial.println(cellMCopy.avgTemperature);
            Serial.println(cellMCopy.current);
            Serial.println(cellMCopy.voltage);
            Serial.println(chaMCopy.avgHum);
            Serial.println(chaMCopy.avgTemp);
            char head[200] = "";
            if (!isHeaderWritten[cellId])
            {
                // write the header row first
                const char *temp = "Time,Volt,Current";
                strcat(head, temp);
                for (uint8_t i = 0; i < no_of_temp_sen_connected_cell[cellId]; i++)
                {
                    strcat(head, ",T");
                    char buff[3] =  "";
                    itoa(i + 1, buff, 10);
                    strcat(head, buff);
                }
                strcat(head, ",Ch_T,Ch_H");
                file.println(head);
                isHeaderWritten[cellId] = true;
            }
            char msg[100];
            char buff[20];//take only 5 digit float
            sprintf(buff,"%.3f,%.3f",(double)cellMCopy.voltage,(double)cellMCopy.current);
            strcat(msg,buff);
            for(uint8_t i=0;i<no_of_temp_sen_connected_cell[cellId];i++){
                strcat(msg,",");
                sprintf(buff,",%.2f",(double)cellMCopy.temperature[i]);
                strcat(msg,buff);
            }
            sprintf(buff,",%.2f,%.2f",(double)chaMCopy.avgTemp,(double)chaMCopy.avgHum);
            strcat(msg,buff);
            file.println(msg);
        }
        else
        {
            Serial.println(F("Output directory creation failed."));
            file.close();
            return false;
        }
        file.close();
        *cellM = cellMCopy;
        *chaM = chaMCopy;
        return true;
    }

    bool cleanDir()
    {
        if (!file)
        {
            Serial.println(F("invalid file."));
            return false;
        }
        char f_name[20];
        file = file.openNextFile(O_TRUNC);
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
};