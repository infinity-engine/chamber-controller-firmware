#ifndef PROTOTYPE
#define PROTOTYPE

#include <DHT.h>
#include "config_atmega.h"
#include <SPI.h>
#include <SdFat.h>

struct CellMeasurement{
    unsigned char cellId;
    float current;
    float voltage;
    float temperature[maxNoOfTempSensorPerCell];
    float avgTemperature;
};

struct ChamberMeasurement
{
    float avgHum;
    float avgTemp;
};

struct CellParameters{
    unsigned char cellId;
    float maxVoltage;
    float minVoltage;
    float maxTemp;
    float minTemp;
};

struct ExperimentParameters{
    unsigned char mode; // 1 - "ConstantCurrentCharge"| 2 - "ConstantCurrentDischarge"| 3 - "ConstantResistanceCharge"| 4 - "ConstantResistanceDischarge"|5 - "ConstantPowerCharge"| 6- "ConstantPowerDischarge";
    float currentRate;//absolute value for both charging and discharging
    float resVal;
    float powVal;
    unsigned long timeLimit;
    unsigned long startTime;
    unsigned long prevTime;
    float curToll;//currentTollerence
    unsigned int sampleIndicator;//drive cycle sample indicator 0 for no point, point among [1-total_n_samples]
    unsigned int total_n_samples;//total no. of samples in the drive cycle
    float samples_batch[DriveCycleBatchSize];
};

void channelTheMux(const bool address[]);
float * measureCellTemperature(unsigned char cell_id,float *temps);
float measureFromADS(unsigned char channel_id);
float measureCellVoltage(unsigned char cell_id);
float measureCellCurrentACS(unsigned char cell_id);
float measureChamberTemperature(DHT *sensor_id);
float measureChamberHumidity(DHT *sensor_id);
float measureChamberAverageTemperature();
float measureChamberAverageHumidity();
void setChamberTemperature(float set_temp, float current_temp);
void controlRelay(const unsigned char relay_pin, bool relay_status);
void setCellChargeDischarge(unsigned char cell_id, bool status);
void setDischargerCurrent(unsigned char discharger_id, float set_current);
float getDischargerCurrent(unsigned char discharger_id);
float getDischargerMosfetTemp(unsigned char discharger_id);
void takeApprActForDischFan(unsigned char discharger_id, bool over_write = false, bool fan_status = false);
void setChargerCurrent(unsigned char discharger_id, float set_current);
float getCurrentACS(unsigned char discharger_id);
float myMap(float x, float in_min, float in_max, float out_min, float out_max);
float measureAvgCellTemp(unsigned char cell_id, float *temps = NULL);
void pinInit();
void resetChannel(unsigned char channelId,bool hardReset = true);
void reserveChannel(unsigned char channelId);
void runExp();
void fillExp();
void initExp();
bool placeNewSubExp(uint8_t channelId);
void measureAndRecord(uint8_t channelId);
void updateProgressBar(unsigned long count, unsigned long totalCount, int lineToPrintOn);
uint16_t getFreeSram();
void log_(struct ExperimentParameters *exp);
void debug();
void dumpFile();


#endif //PROTOTYPE