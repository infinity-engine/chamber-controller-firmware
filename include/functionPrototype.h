#ifndef PROTOTYPE
#define PROTOTYPE

#include <DHT.h>
#include "config_atmega.h"
#include <SdFat.h>

class ReadWriteExpAPI;
class ConversationAPI;
class ConstantChargeDischarge;

struct CallibrationParameters
{
    float acsOffset;
    float currentMultiplierOut;
    float currentMultiplierIn;
};

struct CellMeasurement
{
    float current;
    float voltage;
    float temperature[MaxNoTempSenPossible];
    float avgTemperature;
};

struct ChamberMeasurement
{
    float avgHum;
    float avgTemp;
};

struct CellParameters
{
    unsigned char cellId; // also channelId starts from 1,2,3...
    float maxVoltage;
    float minVoltage;
    float maxTemp;
    float minTemp;
    float maxCurrent;
};

struct ExperimentParameters
{
    unsigned char mode; // 1 - "ConstantCurrentCharge"| 2 - "ConstantCurrentDischarge"| 3 - "ConstantResistanceCharge"| 4 - "ConstantResistanceDischarge"|5 - "ConstantPowerCharge"| 6- "ConstantPowerDischarge";
    float currentRate;  // absolute value for both charging and discharging
    float resVal;
    float powVal;
    unsigned long timeLimit;
    unsigned long startTime;
    unsigned long prevTime;                   // last instant on which measurement sdata has been updated
    unsigned long prevDriveCycleSampleUpdate; // last instant of time on which the drive cycle value has been fetched and set
    float curToll;                            // currentTollerence
    unsigned int sampleIndicator;             // drive cycle sample indicator 0 for no point, point among [1-total_n_samples]
    unsigned int total_n_samples;             // total no. of samples in the drive cycle
    unsigned int multiplier;                  // useful if you want to repeat the same sub experiment for multiple times
    float ambTemp;                            // only used when isConAmTe is false
    float holdVolt;                           // only used when Hold experiment is on progress
    float voltLimit;                          // if until specified
    unsigned int sampleTime;                  // refers to drivecylce sample delay in ms
    float samples_batch[DriveCycleBatchSize];
};

void channelTheMux(const bool address[]);
float *measureCellTemperature(unsigned char cell_id, float *temps);
float measureFromADS(unsigned char channel_id);
float measureCellVoltage(unsigned char cell_id, float prevValue);
float measureCellCurrentACS(unsigned char cell_id, float prevValue);
float measureChamberTemperature(uint8_t);
float measureChamberHumidity(uint8_t);
float measureChamberAverageTemperature();
float measureChamberAverageHumidity();
void setChamberTemperature(float set_temp, float current_temp);
void controlRelay(const unsigned char relay_pin, bool relay_status);
void setCellChargeDischarge(unsigned char cell_id, bool status);
void setDischargerCurrent(unsigned char discharger_id, float set_current);
float getDischargerCurrent(unsigned char discharger_id, float prevValue);
float getDischargerMosfetTemp(unsigned char discharger_id);
void takeApprActForDischFan(unsigned char discharger_id, bool over_write = false, bool fan_status = false);
void setChargerCurrent(unsigned char discharger_id, float set_current);
float getCurrentACS(unsigned char discharger_id, float prevValue);
float myMap(float x, float in_min, float in_max, float out_min, float out_max);
float measureAvgCellTemp(unsigned char cell_id, float *temps = NULL);
void pinInit();
void runExp();
void fillExp();
void updateProgressBar(unsigned long count, unsigned long totalCount, int lineToPrintOn);
uint16_t getFreeSram();
void log_(struct ExperimentParameters *exp);
void dumpFile();
bool lookAndStartExp(ReadWriteExpAPI *api, ConversationAPI *cpi, ConstantChargeDischarge *expArray);
void asAllExpFinished();
void configureNoOfSensorConnected();
void clearLine(uint8_t line);
void lcd_init();
void updateLCDView(bool force = false);
void handleStatusForChannel(uint8_t channelId, uint8_t y, uint8_t x);
void updateLCDArrow(uint8_t startIndex, uint8_t width, uint8_t lineNo, uint16_t interval = 200);
void restChamber();
void debug();
float getMovingAverage(float newValue, float prevValue, float beta = 0.4);
void calibrationInit();
#endif // PROTOTYPE