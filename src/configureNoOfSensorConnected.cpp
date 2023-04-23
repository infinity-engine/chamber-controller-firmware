#include "functionPrototype.h"
#include "config_const.h"
#include "config_atmega.h"

void configureNoOfSensorConnected()
{
    // for temp sensor on cell
    for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
    {
        float temps[no_of_temp_sen_connected_cell[i]];
        measureCellTemperature(i + 1, temps);
        uint8_t valid_no_sensor = 0;
        for (uint8_t j = 0; j < MaxNoTempSenPossible; j++)
        {
            if (temps[j] > TemperatureCutOffFilterLimit)
                valid_no_sensor++;
        }
        Serial.print(F("CH-"));
        Serial.print(i + 1);
        Serial.print(F(": No temp sensors - "));
        Serial.println(valid_no_sensor);
        no_of_temp_sen_connected_cell[i] = valid_no_sensor;
    }

    // for dht sensor

    uint8_t valid_no_sensor = 0;
    for (uint8_t i = 0; i < MaxNoDhtSenPossible; i++)
    {
        float t = measureChamberTemperature(i);
        if (!isnan(t))
            valid_no_sensor++;
    }
    Serial.print(F("No dht sensors - "));
    Serial.println(valid_no_sensor);
    no_of_dht_sensor_connected = valid_no_sensor;
}