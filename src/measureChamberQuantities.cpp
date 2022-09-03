#include <DHT.h>
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif

extern const unsigned char no_of_dht_sensor_connected;
extern DHT dht[];

float measureChamberAverageTemperature()
{
    // returns the average of temperature(degC) reading from all the dht sensors
    float sum = 0;
    for (unsigned char i = 0; i < no_of_dht_sensor_connected; i++)
    {
        sum += measureChamberTemperature(&dht[i]);
    }
    return float(sum / no_of_dht_sensor_connected);
}
float measureChamberTemperature(DHT *sensor_id)
{
    // measures the chamber temperature(degC) with the help of DHT sensor
    float t = sensor_id->readTemperature();
    return t;
}

float measureChamberAverageHumidity()
{
    // returns the average of humidity(%) reading from all the dht sensors
    float sum = 0;
    for (unsigned char i = 0; i < no_of_dht_sensor_connected; i++)
    {
        sum += measureChamberHumidity(&dht[i]);
    }
    return float(sum / no_of_dht_sensor_connected);
}
float measureChamberHumidity(DHT *sensor_id)
{
    // measures the chamber humidity(%) with the help of DHT sensor
    float h = sensor_id->readHumidity();
    return h;
}