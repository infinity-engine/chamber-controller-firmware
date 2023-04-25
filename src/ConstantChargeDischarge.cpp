#include "ConstantChargeDischarge.h"
#include "ReadWriteEXPAPI.h"
#include "ConversationAPI.h"
#define LIMIT_TO_BE_CHECK false

ConstantChargeDischarge::ConstantChargeDischarge(uint8_t channelId)
{
    reset(channelId);
}

void ConstantChargeDischarge::reset(unsigned char cell_id, unsigned char mode)
{
    measurement = {
        0,                  // current
        0,                  // voltage
        {0, 0, 0, 0, 0, 0}, // temperatures
        0                   // avgtemperatue
    };
    // cell_id = 0 means no channel
    parameters = {cell_id, 4.9, 2.0, 80, -20, 5}; // default value need to change according to the cell info from cloud
    expParamters = {mode, 0, 0, 0, 0, 0, 0, 0, 0.1, 0, 0, 0, 25, 4.2, 4.2, 1000};
    chmMeas = {0, 0};
    curRowIndex = 0; // point to multiplier
    noOfSubExps = 0;
    nthCurSubExp = 0;
    curExpStatus = EXP_NOT_STARTED;
    isExpConfigured = false;
    isRowConfigured = false;
    overallMultiplier = 0;
    currentMultiplierIndex = 0;
    overallStatus = EXP_NOT_STARTED;
    isConAmTe = true;
    ambTemp = 25;
}

/**
 * @brief Only Fetched the Exp Config to the particular Exp Object
 * it doesn't start the exp.
 * @param channelId
 * @return true
 * @return false
 */
bool ConstantChargeDischarge::placeNewSubExp(ReadWriteExpAPI *api)
{
    bool status = true;
    prepareForNextSubExp();
    if (ISLOGENABLED)
    {
        Serial.print(F("Placing sub-exp ..."));
        Serial.println(nthCurSubExp);
    }
    if (api->setUpNextSubExp(this))
    {
        // update the exp from the data received and already placed on object property.
        status = true;
        isRowConfigured = true;
    }
    else
    {
        // block the channel
        status = false;
        ISLOGENABLED ? Serial.println(F("Failed")) : 0;
    }
    return status;
}

/**
 * @brief should be called after reading the expconfig from sd card
 * and after seting isRowConfigured = true
 * increases the subExpCount by one
 * set currentRow index = 1
 * @return true
 * @return false
 */
bool ConstantChargeDischarge::startCurrentSubExp()
{
    if (!isRowConfigured)
    {
        return false;
    }
    curExpStatus = EXP_RUNNING;
    overallStatus = EXP_RUNNING;
    setup();
    timeReset();
    if (curRowIndex == 1 && currentMultiplierIndex == 1 && nthCurSubExp == 1)
    {
        // only if this is the first exp of first sub exp of first cycle
        expStartTime = millis();
        if (expParamters.mode == DriveCycle)
        {
            expParamters.sampleIndicator = 1; // for the first time program expect that you already loaded the drivecycle and then call on this function
            // this will also prevent on loading the drivecycle for the firsttime while inside performDriveCycle()
            // cause time is ticking
        }
    }
    return true;
}

/**
 * @brief should be used to prepare to place the next sub exp
 *
 * @return true
 * @return false
 */
bool ConstantChargeDischarge::prepareForNextSubExp()
{
    isRowConfigured = false;
    curExpStatus = EXP_NOT_STARTED;
    if (currentMultiplierIndex == 0 || (curRowIndex == expParamters.multiplier && nthCurSubExp == noOfSubExps))
    {
        currentMultiplierIndex += 1; // only for the first time and
        nthCurSubExp = 0;
    }
    curRowIndex = 1; // multiplier index for cur row.
    nthCurSubExp += 1;
    return true;
}

/**
 * @brief to reinitiate time for the sub experiment
 * can also be called on row multiplier index increase
 */
void ConstantChargeDischarge::timeReset()
{
    expParamters.startTime = millis();
    expParamters.prevTime = expParamters.startTime;
    expParamters.prevDriveCycleSampleUpdate = expParamters.startTime;
    expParamters.sampleIndicator = 0; // rolls back to 0;important in case the total_n_samples*sampleTime > timelimit
}

/**
 * @brief //should be called after placing a new sub
 * or going on next index of the row
 *
 * @param timeResetSubExp
 */
void ConstantChargeDischarge::setup()
{
    // set of instruction when to start a particular experiment
    switch (expParamters.mode)
    {
    case ConstantCurrentCharge:
        setCellChargeDischarge(parameters.cellId, relay_cell_charge);
        setChargerCurrent(parameters.cellId, expParamters.currentRate);
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
        expParamters.sampleIndicator = 0; // for drivecycle reset back to beginning
        break;
    case Rest:
        setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
        setDischargerCurrent(parameters.cellId, 0);
        break;
    case Hold:
        setCellChargeDischarge(parameters.cellId, relay_cell_charge);
        setDischargerCurrent(parameters.cellId, 0);
        break;
    default:
        setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
        setDischargerCurrent(parameters.cellId, 0);
        break;
    }
}

void ConstantChargeDischarge::finish()
{
    // set of instruction after stopping of the experiment
    switch (expParamters.mode)
    {
    case ConstantCurrentCharge:
        setCellChargeDischarge(parameters.cellId, relay_cell_discharge); // by default it's should connected to discharger according to control circuit
        setChargerCurrent(parameters.cellId, 0);
        break;
    case ConstantCurrentDischarge:
        setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
        setDischargerCurrent(parameters.cellId, 0);
        takeApprActForDischFan(parameters.cellId, true, false);
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
        setChargerCurrent(parameters.cellId, 0);
        takeApprActForDischFan(parameters.cellId, true, false);
        break;
    case Rest:
        setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
        setDischargerCurrent(parameters.cellId, 0);
        setChargerCurrent(parameters.cellId, 0);
        takeApprActForDischFan(parameters.cellId, true, false);
        break;
    case Hold:
        setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
        setDischargerCurrent(parameters.cellId, 0);
        setChargerCurrent(parameters.cellId, 0);
        takeApprActForDischFan(parameters.cellId, true, false);
        break;
    default:
        break;
    }
}

/**
 * @brief Take Measurement
 * send experiment status == EXP_FINSIHED only when all the row multiplier is finished
 * @param api
 * @return uint8_t
 */
uint8_t ConstantChargeDischarge::performAction(ReadWriteExpAPI &api, ConversationAPI &cpi)
{
    // time consumption - minimum 88ms; max 174 ms
    if (expParamters.mode == DriveCycle)
    {
        if (millis() - expParamters.prevTime < (expParamters.sampleTime / 2.0))
        {
            return curExpStatus;
        }
    }
    else
    {
        if (millis() - expParamters.prevTime < sample_update_delay)
        {
            return curExpStatus;
        }
    }
    float hold_tolerance = 0.1;
    measureCellTemperature(parameters.cellId, measurement.temperature); // 97ms for 6 sensors and 2 samples
    measurement.avgTemperature = measureAvgCellTemp(parameters.cellId, measurement.temperature);
    measurement.voltage = measureCellVoltage(parameters.cellId); // 39ms for for 5 samples
    chmMeas.avgHum = measureChamberAverageHumidity();
    chmMeas.avgTemp = measureChamberAverageTemperature();

    unsigned char status = EXP_RUNNING;
    unsigned long curTime = millis();
    expParamters.prevTime = curTime;

    switch (expParamters.mode)
    {
    case ConstantCurrentCharge:
        measurement.current = measureCellCurrentACS(parameters.cellId);
        if (expParamters.voltLimit != 0 && measurement.voltage > expParamters.voltLimit)
        {
            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Voltage limit achieved."));
            status = EXP_FINISHED;
        }
        if ((measurement.voltage > parameters.maxVoltage || measurement.voltage < parameters.minVoltage) && LIMIT_TO_BE_CHECK)
        {
            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Voltage limit crossed."));
            status = EXP_STOPPED;
        }
        if ((measurement.avgTemperature > parameters.maxTemp || measurement.avgTemperature < parameters.minTemp) && LIMIT_TO_BE_CHECK)
        {
            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Temperature limit crossed"));
            status = EXP_STOPPED;
        }
        if ((abs(abs(measurement.current) - abs(expParamters.currentRate)) > expParamters.curToll) && LIMIT_TO_BE_CHECK)
        {
            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Re-initialize ChargeDischarge"));
            setup();
        }
        break;
    case ConstantCurrentDischarge:
        measurement.current = getDischargerCurrent(parameters.cellId);
        if (expParamters.voltLimit != 0 && measurement.voltage < expParamters.voltLimit)
        {
            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Voltage limit achieved."));
            status = EXP_FINISHED;
        }
        if ((measurement.voltage > parameters.maxVoltage || measurement.voltage < parameters.minVoltage) && LIMIT_TO_BE_CHECK)
        {
            // even though you don't want to perform experiment on voltage limit
            // there should be one for safety purpose
            // change the limit for forceful experiment
            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Voltage limit crossed"));
            status = EXP_FINISHED; // completed
        }
        if ((measurement.avgTemperature > parameters.maxTemp || measurement.avgTemperature < parameters.minTemp) && LIMIT_TO_BE_CHECK)
        {

            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Temperature limit crossed"));
            status = EXP_STOPPED; // stopped
        }
        if ((abs(abs(measurement.current) - abs(expParamters.currentRate)) > expParamters.curToll) && LIMIT_TO_BE_CHECK)
        {
            Serial.print(F("CH "));
            Serial.print(parameters.cellId);
            Serial.println(F(": Re-initialize ChargeDischarge"));
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
        status = perFormDriveCycle(api);
        break;
    case Rest:
        // do nothing only just mark measurement,
        // outside time check will automaticly check it
        measurement.current = getDischargerCurrent(parameters.cellId);
        break;
    case Hold:
        // assuming the hold period will be to prevent the natural fall of cell voltage
        // it would be in the charge mode
        // if you are care about frequent relay change
        //  then it should be taken care by sample_update_delay from outside where it's gets called.
        if (measurement.voltage > expParamters.holdVolt + hold_tolerance)
        {
            setCellChargeDischarge(parameters.cellId, relay_cell_discharge);
        }
        else if (measurement.voltage < expParamters.holdVolt - hold_tolerance)
        {
            setCellChargeDischarge(parameters.cellId, relay_cell_charge);
        }
        measurement.current = measureCellCurrentACS(parameters.cellId);
        break;
    default:
        status = EXP_NOT_STARTED;
        break;
    }
    // update the time parameters
    // record data

    recordData(api, cpi);

    if (expParamters.timeLimit > 0)
    {
        // if time limit is set
        if (curTime - expParamters.startTime >= expParamters.timeLimit * 1000)
        {
            // set time has reached
            if (ISLOGENABLED)
            {
                Serial.print(F("CH "));
                Serial.print(parameters.cellId);
                Serial.println(F(": Time's up / row multiplier index."));
            }

            if (curRowIndex == expParamters.multiplier)
            {
                status = EXP_FINISHED; // completed
                finish();
                cpi.setStatus(EXP_FINISHED, parameters.cellId, nthCurSubExp);
            }
            else
            {
                cpi.incrementMultiplier(parameters.cellId, nthCurSubExp);
                curRowIndex += 1;
                timeReset();
            }
        }
    }
    if (status != EXP_RUNNING)
    {
        overallStatus = status;
        finish();
    }
    return status;
}

/**
 * @brief
 *
 * @param api
 * @param sampleTime defines time delay between each sample present in the drivecycle
 * @param curTime
 * @return uint8_t
 */
uint8_t ConstantChargeDischarge::perFormDriveCycle(ReadWriteExpAPI &api, unsigned long curTime)
{
    unsigned status = EXP_RUNNING;
    static bool isOnDischarge;
    if (curTime > expParamters.prevDriveCycleSampleUpdate + expParamters.sampleTime)
    {
        if (expParamters.sampleIndicator == 0)
        {
            // just for the first time
            if (api.fillNextDriveCyclePortion(this))
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
        // if (ISLOGENABLED)
        // {
        //     Serial.print(F("CH "));
        //     Serial.print(parameters.cellId);
        //     Serial.print(F(": DC SI "));
        //     Serial.println(expParamters.sampleIndicator);
        // }
        // get the position on cureent batch,array indicator [0 <-> (batchsize-1)]
        unsigned char indicator = ((expParamters.sampleIndicator - 1) % DriveCycleBatchSize);
        float cur_A = expParamters.samples_batch[indicator];

        if (cur_A >= 0)
        {
            setDischargerCurrent(parameters.cellId, cur_A);
            isOnDischarge = true;
        }
        else
        {
            setChargerCurrent(parameters.cellId, cur_A);
            isOnDischarge = false;
        }

        expParamters.sampleIndicator += 1;

        if (expParamters.sampleIndicator > expParamters.total_n_samples)
        {
            expParamters.sampleIndicator = 0; // roll back to start
            // next row multiplier index will only start when timelimit is hit
        }

        else if (indicator >= DriveCycleBatchSize)
        {
            // get the new set of samples
            if (ISLOGENABLED)
            {
                Serial.print(F("CH "));
                Serial.print(parameters.cellId);
                Serial.println(F(": Filling next set of samples."));
            }
            if (!api.fillNextDriveCyclePortion(this))
            {
                // some error occur so stop
                status = EXP_STOPPED;
            }
        }

        expParamters.prevDriveCycleSampleUpdate = millis();
    }

    // get rest of the measurement
    if (isOnDischarge)
    {
        measurement.current = getDischargerCurrent(parameters.cellId);
    }
    else
    {
        measurement.current = getCurrentACS(parameters.cellId);
    }

    return status;
}

void ConstantChargeDischarge::recordData(ReadWriteExpAPI &api, ConversationAPI &cpi)
{
    // get the details and do what you wanna do with it and update curExpStatus if required
    // potential sd card calls
    char row[150] = "";
    formRow(row);
    cpi.sendMeasurement(parameters.cellId, row);
    if (!api.logReadings(this, row))
    {
        Serial.println(F("Log data failed"));
        curExpStatus = EXP_STOPPED;
        return;
    }
    if (ISLOGENABLED)
    {
        Serial.print(F("CH "));
        Serial.print(parameters.cellId);
        Serial.print(F(": Current:"));
        Serial.print(measurement.current, 4);
        Serial.print(F(",Voltage:"));
        Serial.print(measurement.voltage);
        Serial.print(F(",Cell Temp.(°C):"));
        Serial.print(measurement.avgTemperature);
        Serial.print(F(",Chamber Humidity(%):"));
        Serial.print(chmMeas.avgHum);
        Serial.print(F(",Chamber Temp.(°C):"));
        Serial.println(chmMeas.avgTemp);
    }
}

void ConstantChargeDischarge::formRow(char *row)
{
    char buff[20] = "";
    dtostrf((millis() - expStartTime) / 1000.0, 2, 2, buff);
    strcat(row, buff);

    strcat(row, ",");
    dtostrf(measurement.voltage, 3, 2, buff);
    strcat(row, buff);

    strcat(row, ",");
    dtostrf(measurement.current, 3, 2, buff);
    strcat(row, buff);

    // this step is necessary to segegrate, data for json capcuture for cell temp senos capture
    // has dependency on how the network units's convertRowIntoJsonPayload function work

    if (no_of_dht_sensor_connected > 0)
    {
        strcat(row, ",");
        dtostrf(chmMeas.avgTemp, 2, 1, buff);
        strcat(row, buff);
        strcat(row, ",");
        dtostrf(chmMeas.avgHum, 2, 1, buff);
        strcat(row, buff);
    }
    else
    {
        strcat(row, ",");
        strcat(row, "NAN");
        strcat(row, ",");
        strcat(row, "NAN");
    }

    for (uint8_t i = 0; i < no_of_temp_sen_connected_cell[parameters.cellId - 1]; i++)
    {
        strcat(row, ",");
        dtostrf(measurement.temperature[i], 2, 1, buff);
        strcat(row, buff);
    }
}