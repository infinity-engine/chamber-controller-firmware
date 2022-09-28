#include <DHT.h>
#ifndef CONFIG_ATMEGA
#include "config_atmega.h"
#endif
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
    float currentRate;
    float resVal;
    float powVal;
    unsigned long timeLimit;
    unsigned long startTime;
    unsigned long prevTime;
    float curToll;//currentTollerence
    unsigned int sampleIndicator;//drive cycle sample indicator
    unsigned int samples;
};

void channelTheMux(bool address[]);
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
float myMap(float x, float in_min, float in_max, float out_min, float out_max);
float measureAvgCellTemp(unsigned char cell_id);
unsigned char perFormDriveCycle(CellParameters &parameters,CellMeasurement &measurement,ExperimentParameters &expParms,ChamberMeasurement &chmMeas,int sampleTime=1000,unsigned long curTime = millis());
void pinInit();
void resetChannel(unsigned char channelId);
void runExp();
void fillExp();
void initExp();