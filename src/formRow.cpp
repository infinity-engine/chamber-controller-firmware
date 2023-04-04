#include <Arduino.h>
#include "ConstantChargeDischarge.h"
#include "config_atmega.h"

void formRow(char *row, ConstantChargeDischarge *exp)
{
    char buff[20] = "";
    dtostrf((millis() - exp->expStartTime) / 1000.0, 2, 2, buff);
    strcat(row, buff);

    strcat(row, ",");
    dtostrf(exp->measurement.voltage, 3, 3, buff);
    strcat(row, buff);

    strcat(row, ",");
    dtostrf(exp->measurement.current, 3, 3, buff);
    strcat(row, buff);

    strcat(row, ",");
    dtostrf(exp->chmMeas.avgTemp, 3, 3, buff);
    strcat(row, buff);

    strcat(row, ",");
    dtostrf(exp->chmMeas.avgHum, 3, 3, buff);
    strcat(row, buff);

    for (uint8_t i = 0; i < no_of_temp_sen_connected_cell[exp->parameters.cellId - 1]; i++)
    {
        strcat(row, ",");
        dtostrf(exp->measurement.temperature[i], 2, 2, buff);
        strcat(row, buff);
    }

    // Serial.println(row);
}