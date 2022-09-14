#include <Arduino.h>
#include <string.h>
#ifndef CONFIG_ATMEGA
#include "config_atmega.h"
#endif
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif

class ConstantChargeDischarge
{
public:
    unsigned isFinished; // 0 for not finished/running, 1 for finished, 2 for stopped for safety
    struct CellMeasurement measurement;
    struct CellParameters parameters;
    struct ExperimentParameters expParamters;
    unsigned int driveCycleSampleIndicator = 0; // represent on which sample it is currently on

    ConstantChargeDischarge(unsigned char cell_id, unsigned char mode = 2)
    {
        isFinished = 0;
        measurement = {
            parameters.cellId,
            0,                  // current
            0,                  // voltage
            {0, 0, 0, 0, 0, 0}, // temperatures
            0                   // avgtemperatue
        };
        parameters = {cell_id, 4.2, 3.0, 80, -20};
        expParamters = {mode, 0, 0, 0, 0, 0, 0, 0, 0.1, 0, 20};
    }

    void setup()
    {
        // set of instruction when to start a particular experiment
        switch (expParamters.mode)
        {
        case ConstantCurrentCharge:
            setCellChargeDischarge(parameters.cellId, relay_cell_charge);
            // setChargerCurrent(cell_id,chargeRate);
            break;
        case ConstantCurrentDischarge:
            setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
            setDischargerCurrent(parameters.cellId, expParamters.disChargeRate);
            expParamters.startTime = millis();
            takeApprActForDischFan(parameters.cellId, true, true);
            break;
        case ConstantResistanceCharge:
            break;
        case ConstantResistanceDischarge:
            break;
        case ConstantPowerCharge:
            break;
        case ConstantPowerDischarge:
            break;
        case DriveCycle:
            expParamters.startTime = millis();
            takeApprActForDischFan(parameters.cellId, true, true);
            break;
        default:
            break;
        }
    }

    void finish()
    {
        // set of instruction after stopping of the experiment
        switch (expParamters.mode)
        {
        case ConstantCurrentCharge:
            setCellChargeDischarge(parameters.cellId, relay_cell_discharge); // by default it's should connected to discharger according to control circuit
            // setChargerCurrent(cell_id,0);
            break;
        case ConstantCurrentDischarge:
            setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
            setDischargerCurrent(parameters.cellId, 0);
            break;
        case ConstantResistanceCharge:
            break;
        case ConstantResistanceDischarge:
            break;
        case ConstantPowerCharge:
            break;
        case ConstantPowerDischarge:
            break;
        case DriveCycle:
            setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
            setDischargerCurrent(parameters.cellId, 0);
            break;
        default:
            break;
        }
    }

    unsigned char performAction()
    {
        // memcpy(measurement.temperature,measureCellTemperature(parameters.cellId,measurement.temperature),sizeof(measurement.temperature));
        measureCellTemperature(parameters.cellId, measurement.temperature);
        measurement.voltage = measureCellVoltage(parameters.cellId);
        unsigned char status = 0;
        switch (expParamters.mode)
        {
        case ConstantCurrentCharge:
            measurement.current = measureCellCurrentACS(parameters.cellId);
            if (measurement.voltage > parameters.maxVoltage || measurement.voltage < parameters.minVoltage)
            {
                // Serial.println(F("Voltage limit crossed"));
                status = 1;
            }
            if (measurement.avgTemperature > parameters.maxTemp || measurement.avgTemperature < parameters.minTemp)
            {
                // Serial.println(F("Temperature limit crossed"));
                status = 2;
            }
            if (abs(abs(measurement.current) - abs(expParamters.chargeRate)) > expParamters.curToll)
            {
                // Serial.println(F("Re-initialize ChargeDischarge"));
                setup();
            }
            break;
        case ConstantCurrentDischarge:
            measurement.current = getDischargerCurrent(parameters.cellId);
            if (expParamters.timeLimit > 0)
            {
                // if time limit is set
                unsigned long curTime = millis();
                if (curTime - expParamters.startTime >= expParamters.timeLimit)
                {
                    // set time has reached
                    status = 1; // completed
                }
            }
            if (measurement.voltage > parameters.maxTemp || measurement.voltage < parameters.minVoltage)
            {
                // even though you don't want to perform experiment on voltage limit
                // there should be one for safety purpose
                // change the limit for forceful experiment
                // Serial.println(F("Voltage limit crossed"));
                status = 1; // completed
            }
            if (measurement.avgTemperature > parameters.maxTemp || measurement.avgTemperature < parameters.minTemp)
            {
                // Serial.println(F("Temperature limit crossed"));
                status = 2; // stopped
            }
            if (abs(abs(measurement.current) - abs(expParamters.chargeRate)) > expParamters.curToll)
            {
                // Serial.println(F("Re-initialize ChargeDischarge"));
                setup();
            }
            break;
        case ConstantResistanceCharge:
            break;
        case ConstantResistanceDischarge:
            break;
        case ConstantPowerCharge:
            break;
        case ConstantPowerDischarge:
            break;
        case DriveCycle:
            status = perFormDriveCycle(parameters, measurement, expParamters);
            //some how calling the above functin is changing the object
            break;
        default:
            status = 0;
            break;
        }
        // update the time parameters
        unsigned long curTime = millis();
        if (expParamters.timeLimit > 0)
        {
            // if time limit is set
            if (curTime - expParamters.startTime >= expParamters.timeLimit)
            {
                // set time has reached
                status = 1; // completed
            }
        }
        isFinished = status;
        if (status != 0)
        {
            finish();
        }
        return status;
    }
};