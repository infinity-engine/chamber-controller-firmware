#include <DHT.h>
#include "functionPrototype.h"
#include "config_const.h"

extern DHT dht[];

float prevChTemp[4] = {0, 0, 0, 0};
float prevChHum[4] = {0, 0, 0, 0};

float measureChamberAverageTemperature()
{
    // returns the average of temperature(degC) reading from all the dht sensors
    float sum = 0;
    for (unsigned char i = 0; i < no_of_dht_sensor_connected; i++)
    {
        sum += measureChamberTemperature(i);
    }
    return float(sum / no_of_dht_sensor_connected);
}
/**
 * @brief
 *
 * @param sensor_id 0-max
 * @return float
 */
float measureChamberTemperature(uint8_t sensor_id)
{
    // measures the chamber temperature(degC) with the help of DHT sensor
    float t = dht[sensor_id].readTemperature();
    prevChTemp[sensor_id] = getMovingAverage(t, prevChTemp[sensor_id], 0.1);
    return prevChTemp[sensor_id];
}

float measureChamberAverageHumidity()
{
    // returns the average of humidity(%) reading from all the dht sensors
    float sum = 0;
    for (unsigned char i = 0; i < no_of_dht_sensor_connected; i++)
    {
        sum += measureChamberHumidity(i);
    }
    return float(sum / no_of_dht_sensor_connected);
}

/**
 * @brief measures chamber humidty
 *
 * @param sensor_id 0-max
 * @return float in percent
 */
float measureChamberHumidity(uint8_t sensor_id)
{
    // measures the chamber humidity(%) with the help of DHT sensor
    float h = dht[sensor_id].readHumidity();
    prevChHum[sensor_id] = getMovingAverage(h, prevChHum[sensor_id], 0.1);
    return prevChHum[sensor_id];
}