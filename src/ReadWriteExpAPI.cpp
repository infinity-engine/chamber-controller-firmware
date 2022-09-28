#include <Arduino.h>
#include <ArduinoJson.h>
#ifndef CONFIG_ATMEGA
#include "config_atmega.h"
#endif
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif
#include <SPI.h>
#include <SD.h>

extern File myFile;

class ReadWriteExpAPI{
    public:
        unsigned char currentSubExpNo[N_CELL_CAPABLE];
        bool overallFinishedStatus[N_CELL_CAPABLE];
        unsigned char mode_curr_exp[N_CELL_CAPABLE];
        String expName[N_CELL_CAPABLE];

        ReadWriteExpAPI(){
           for (unsigned char i=0;i<N_CELL_CAPABLE;i++){
            currentSubExpNo[i] = 0;
            overallFinishedStatus[i] = true;//no exp placed
            mode_curr_exp[i] = 0;
           }
        }
        void setup(String exp_no){
            //get the experiment no from the network module to perfrom
            // read the sd card for that particular file availability
            //store the expno in the property
            if (SD.exists(exp_no)){
                Serial.println(F("Exp config exists."));
            }
            else{
                Serial.println(F("Exp config not found in SD card."));
            }
            return;
        }

        // ExperimentParameters getNextSubExpParameter(unsigned  char channel_id){
        //     return;
        // }

        void fillNextDriveCyclePortion(unsigned char channel_id,float *current_batch[DriveCycleBatchSize]){
            //read the next set of current for the drive cycle
            //copy all the value in to the received pointer
            if (mode_curr_exp[channel_id] != DriveCycle){
                //only valid for drive cycle exp.
                return;
            }
        }


};