#include <DHT.h>

#ifndef CUSTOM_H
#define CUSTOM_H
#include "functionPrototype.h"
#include "config_atmega.h"
#endif

extern DHT  dht_1(DHTPIN_1, DHTTYPE_1);
extern DHT  dht_2(DHTPIN_1, DHTTYPE_2);
extern DHT dht_3(DHTPIN_1, DHTTYPE_3);
extern DHT dht_4(DHTPIN_1, DHTTYPE_4);
float measureChamberHumidity(DHT * sensor_id){
    //measures the chamber temperature with the help of DHT sensor
    float h = sensor_id->readHumidity();
    return h;
}