#include <Arduino.h>
#include <ArduinoJson.h>
#ifndef CONFIG_ATMEGA
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

#ifndef CYCCHDIS
#include "CyclicChargeDischarge.cpp"
#endif

class ReadWriteExpAPI
{
public:
    unsigned char currentSubExpNo[N_CELL_CAPABLE];
    bool overallFinishedStatus[N_CELL_CAPABLE];
    unsigned char mode_curr_exp[N_CELL_CAPABLE];
    String expName[N_CELL_CAPABLE];

    ReadWriteExpAPI()
    {
        for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
        {
            currentSubExpNo[i] = 0;          // not on any sub exp
            overallFinishedStatus[i] = true; // no exp placed
            mode_curr_exp[i] = 0;            // not valid
        }
    }
    void setUpNextSubExp(unsigned char cellId, const char *exp_no,ConstantChargeDischarge *exp)
    {
        // get the experiment no from the network module to perfrom
        //  read the sd card for that particular file availability
        // store the expno in the property
        cellId--;
        StaticJsonDocument<200> doc;
        if (sd.exists(exp_no))
        {
            Serial.print(exp_no);
            Serial.println(F(" -> config exists."));
            if ((currentSubExpNo[cellId] == 0 and overallFinishedStatus[cellId]) or (currentSubExpNo[cellId] > 0 and overallFinishedStatus[cellId] == false))
            {
                // start the first experiment
                currentSubExpNo[cellId]++;
                overallFinishedStatus[cellId] = false;
            }
            else
            {
                Serial.println(F("No new sub experiment"));
                return;
            }
        }
        else
        {
            Serial.print(exp_no);
            Serial.println(F(" -> config does not exist."));
            return;
        }
        sd.chdir(exp_no);
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
                return;
            }
            unsigned char mode = doc["mode"];
            float currentRate = doc["currentRate"];
            float resVal = doc["resVal"];
            float powVal = doc["powVal"];
            unsigned long timeLimit = doc["timeLimit"];
            exp->expParamters.mode = mode;
            exp->expParamters.resVal = resVal;
            exp->expParamters.powVal = powVal;
            exp->expParamters.currentRate = currentRate;
            exp->expParamters.timeLimit = timeLimit;
            mode_curr_exp[cellId] = mode;
        }
        else
        {
            Serial.println(F("Error opening the config file."));
        }
        file.close();
        return;
    }

    // ExperimentParameters getNextSubExpParameter(unsigned  char channel_id){
    //     return;
    // }

    void fillNextDriveCyclePortion(unsigned char cellId,const char *exp_no)
    {
        cellId--;
        // read the next set of current for the drive cycle
        // copy all the value in to the received pointer
        if (mode_curr_exp[cellId] != DriveCycle)
        {
            // only valid for drive cycle exp.
            return;
        }
        sd.chdir(exp_no);
        String end = "_drivecycle.csv";
        String config = currentSubExpNo[cellId] + end;
        //Serial.println(config);
        file = sd.open(config, FILE_READ);
        if (file)
        {
            
            while (file.available()){
                //Serial.println(file.readStringUntil(',').toFloat());
            }
        }else{
            Serial.println(F("Error opening the config file."));
        }
        Serial.println("Done");
        file.close();
    }
};