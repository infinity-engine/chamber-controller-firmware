#include <DHT.h>

#ifndef CUSTOM_H
#define CUSTOM_H
#include "functionPrototype.h"
#include "config_atmega.h"
#endif

void channelTheMux(bool []);
float * measureCellTemperature(unsigned char);
float measureFromADS(unsigned char);
float measureCellVoltage(unsigned char);
float measureCellCurrentACS(unsigned char);
float measureChamberTemperature(DHT *);
float measureChamberHumidity(DHT *);

void doNothing();