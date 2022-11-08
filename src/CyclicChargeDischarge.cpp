#include <Arduino.h>
#include <string.h>

#ifndef CONFIG_CONST
#define CONFIG_CONST
#include "config_const.h"
#endif

#ifndef PROTOTYPE
#define PROTOTYPE
#include "functionPrototype.h"
#endif

#ifndef READWRITEEXPAPI
#define READWRITEEXPAPI
#include "ReadWriteEXPAPI.cpp"
#endif

#define LIMIT_TO_BE_CHECK false

class ConstantChargeDischarge
{
public:
    unsigned isFinished; // 0 for not finished/running, 1 for finished, 2 for stopped for safety
    struct CellMeasurement measurement;
    struct CellParameters parameters;
    struct ExperimentParameters expParamters;
    struct ChamberMeasurement chmMeas;

    ConstantChargeDischarge(unsigned char cell_id, unsigned char mode = ConstantCurrentDischarge)
    {
        isFinished = EXP_NOT_STARTED;
        measurement = {
            parameters.cellId,
            0,                  // current
            0,                  // voltage
            {0, 0, 0, 0, 0, 0}, // temperatures
            0                   // avgtemperatue
        };
        parameters = {cell_id, 4.2, 3.0, 80, -20};
        expParamters = {mode, 0, 0, 0, 0, 0, 0, 0.1, 0, 0};
        chmMeas = {0, 0};
    }

    void setup(bool timeReset = true)
    {
        if (timeReset){
            expParamters.startTime = millis();
        }
        // set of instruction when to start a particular experiment
        switch (expParamters.mode)
        {
        case ConstantCurrentCharge:
            setCellChargeDischarge(parameters.cellId, relay_cell_charge);
            setChargerCurrent(parameters.cellId,expParamters.currentRate);
            break;
        case ConstantCurrentDischarge:
            setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
            setDischargerCurrent(parameters.cellId, expParamters.currentRate);
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
            setChargerCurrent(parameters.cellId,0);
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
            setChargerCurrent(parameters.cellId,0);
            break;
        default:
            break;
        }
    }

    unsigned char performAction(ReadWriteExpAPI &api)
    {
        measureCellTemperature(parameters.cellId, measurement.temperature);
        measurement.avgTemperature = measureAvgCellTemp(parameters.cellId,measurement.temperature);
        measurement.voltage = measureCellVoltage(parameters.cellId);
        unsigned char status = EXP_RUNNING;
        switch (expParamters.mode)
        {
        case ConstantCurrentCharge:
            measurement.current = measureCellCurrentACS(parameters.cellId);
            if ((measurement.voltage > parameters.maxVoltage || measurement.voltage < parameters.minVoltage) && LIMIT_TO_BE_CHECK)
            {
                Serial.println(F("Voltage limit crossed"));
                status = EXP_FINISHED;
            }
            if ((measurement.avgTemperature > parameters.maxTemp || measurement.avgTemperature < parameters.minTemp) && LIMIT_TO_BE_CHECK)
            {
                Serial.println(F("Temperature limit crossed"));
                status = EXP_STOPPED;
            }
            if ((abs(abs(measurement.current) - abs(expParamters.currentRate)) > expParamters.curToll) && LIMIT_TO_BE_CHECK)
            {
                Serial.println(F("Re-initialize ChargeDischarge"));
                setup(false);
            }
            break;
        case ConstantCurrentDischarge:
            measurement.current = getDischargerCurrent(parameters.cellId);
            if ((measurement.voltage > parameters.maxVoltage || measurement.voltage < parameters.minVoltage) && LIMIT_TO_BE_CHECK)
            {
                // even though you don't want to perform experiment on voltage limit
                // there should be one for safety purpose
                // change the limit for forceful experiment
                Serial.println(F("Voltage limit crossed"));
                status = EXP_FINISHED; // completed
            }
            if ((measurement.avgTemperature > parameters.maxTemp || measurement.avgTemperature < parameters.minTemp) && LIMIT_TO_BE_CHECK)
            {
                Serial.println(F("Temperature limit crossed"));
                status = EXP_STOPPED; // stopped
            }
            if ((abs(abs(measurement.current) - abs(expParamters.currentRate)) > expParamters.curToll) && LIMIT_TO_BE_CHECK)
            {
                Serial.println(F("Re-initialize ChargeDischarge"));
                setup(false);
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
            status = perFormDriveCycle(api);
            break;
        default:
            status = EXP_NOT_STARTED;
            break;
        }
        // update the time parameters
        unsigned long curTime = millis();
        if (expParamters.timeLimit > 0)
        {
            // if time limit is set
            if (curTime - expParamters.startTime >= expParamters.timeLimit*1000)
            {
                // set time has reached
                Serial.println(F("Time's up."));
                status = EXP_FINISHED; // completed
            }
        }
        if (status != EXP_RUNNING)
        {
            finish();
        }
        return status;
    }

    unsigned char perFormDriveCycle(ReadWriteExpAPI &api, int sampleTime = 1000, unsigned long curTime = millis())
    {
        
        unsigned status = EXP_RUNNING; // 0 for not finsished, 1 for finished, 2 for stopped
        if (curTime > expParamters.prevTime + sampleTime)
        {
            //log_(&expParamters);
            if (expParamters.sampleIndicator == 0)
            {
                // just for the first time
                if (api.fillNextDriveCyclePortion(parameters.cellId, &expParamters))
                {
                    expParamters.sampleIndicator += 1;
                }
                else
                {
                    // some error occur so stop
                    status = EXP_STOPPED;
                    return status;
                }
            }
            Serial.print(F("Drive-cycle sample indicator "));
            Serial.println(expParamters.sampleIndicator);
            unsigned char indicator = ((expParamters.sampleIndicator - 1) % DriveCycleBatchSize); // get the position on cureent batch,array indicator [0 <-> (batchsize-1)]
            float cur_A = expParamters.samples_batch[indicator];
            if (cur_A >= 0){
                setDischargerCurrent(parameters.cellId, cur_A);
                measurement.current = getDischargerCurrent(parameters.cellId);
            }else{
                setChargerCurrent(parameters.cellId,cur_A);
                measurement.current = getCurrentACS(parameters.cellId);
            }
            
            expParamters.sampleIndicator += 1;
            // Serial.println(expParamters.sampleIndicator);
            if (expParamters.sampleIndicator > expParamters.total_n_samples)
            {
                status = EXP_FINISHED; // completed
            }
            else if (indicator >= DriveCycleBatchSize)
            {
                // get the new set of samples
                Serial.println(F("Filling next set of samples."));
                if (!api.fillNextDriveCyclePortion(parameters.cellId, &expParamters))
                {
                    // some error occur so stop
                    status = EXP_STOPPED;
                }
            }
            expParamters.prevTime = curTime;
        }
        chmMeas.avgHum = measureChamberAverageHumidity();
        chmMeas.avgTemp = measureChamberAverageTemperature();
        return status;
    }
};
