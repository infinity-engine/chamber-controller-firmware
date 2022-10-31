#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef CONFIG_ATMEGA
#define CONFIG_ATMEGA
#include "config_atmega.h"
#endif

#ifndef PROTOTYPE
#define PROTOTYPE
#include "functionPrototype.h"
#endif

#include <SPI.h>
#include <SdFat.h>
extern SdFat sd;
extern File dir;
extern File file;

class ReadWriteExpAPI
{
private:
    unsigned char currentSubExpNo[N_CELL_CAPABLE];
    unsigned char mode_curr_exp[N_CELL_CAPABLE];
    String expName[N_CELL_CAPABLE];

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

    void resetAPIChannel(unsigned char cellId, String exp_name = "")
    {
        cellId--;
        currentSubExpNo[cellId] = 0;
        mode_curr_exp[cellId] = 0;
        driveCycleSampleIndicator[cellId] = 0;
        expName[cellId] = exp_name;
    }

    /**
     * @brief with the current experiment name. availabe on the object property
     * read through the sd card and fill the given expParams with approprite experiment setup
     * @param cellId nth cell 1-Max_cell_capable
     * @param expParamters parameters for the sub experiments
     * @return true on successful new sub exp placing
     * @return false otherwise
     */
    bool setUpNextSubExp(unsigned char cellId, ExperimentParameters &expParamters)
    {
        cellId--;
        if (expName[cellId].length() == 0)
        {
            Serial.println(F("Exp name not configured"));
            return false;
        }
        StaticJsonDocument<200> doc;
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
        sd.chdir(expName[cellId]);
        String end = "_config.json";
        String config = currentSubExpNo[cellId] + end;
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
            expParamters.mode = mode;
            expParamters.resVal = resVal;
            expParamters.powVal = powVal;
            expParamters.currentRate = currentRate;
            expParamters.timeLimit = timeLimit;
            mode_curr_exp[cellId] = mode;
        }
        else
        {
            Serial.println(F("Couldn't find the Sub-Exp."));
            return false;
        }
        file.close();
        return true;
    }

    bool fillNextDriveCyclePortion(unsigned char cellId, float *drive_cycle, uint8_t n_samples = DriveCycleBatchSize)
    {
        cellId--;
        // read the next set of current for the drive cycle
        // copy all the value in to the received pointer
        if (mode_curr_exp[cellId] != DriveCycle)
        {
            // only valid for drive cycle exp.
            return false;
        }
        sd.chdir(expName[cellId]);
        String end = "_drivecycle.csv";
        String config = currentSubExpNo[cellId] + end;
        // Serial.println(config);
        file = sd.open(config, FILE_READ);
        if (file)
        {
            if (file.seek(driveCycleSampleIndicator[cellId]))
            {
                for (unsigned char i = 0; i < n_samples; i++)
                {
                    // clearing the previous value in case end
                    drive_cycle[i] = 0;
                }
                for (unsigned char i = 0; i < n_samples and file.available(); i++)
                {
                    float data = file.readStringUntil(',').toFloat();
                    Serial.println(data, 4);
                    drive_cycle[i] = data;
                    // Serial.println(i);
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
        return true;
    }
};