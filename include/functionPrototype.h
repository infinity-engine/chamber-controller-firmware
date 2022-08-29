#include <DHT.h>

void channelTheMux(bool address[]);
float * measureCellTemperature(unsigned char cell_id);
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