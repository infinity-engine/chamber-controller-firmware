#include "ReadWriteEXPAPI.h"
#include "ConstantChargeDischarge.h"

/**
 * @brief Construct a new Read Write Exp A P I object
 *
 */
ReadWriteExpAPI::ReadWriteExpAPI()
{
    reset();
}

void ReadWriteExpAPI::reset(char *expNameNew)
{
    strcpy(expName, (const char *)expNameNew);
    for (uint8_t cellId = 0; cellId < N_CELL_CAPABLE; cellId++)
    {
        isHeaderWritten[cellId] = false;
        isOpDirChecked[cellId] = false;
        logSDPointer[cellId] = 0;
        dcPtr[cellId] = 0;
    }
}

/**
 * @brief Set the experiment overall setup like cyclic count,cell paramters etc
 *
 * @param ccd
 * @return true
 * @return false
 */
bool ReadWriteExpAPI::setup(ConstantChargeDischarge *ccd)
{
    if (strlen(expName) == 0)
    {
        return false;
    }
    StaticJsonDocument<300> doc;
    String configPath = String(expName) + "/" + String(expName) + "_" + ccd->parameters.cellId;
    configPath += "/inputs";
    // set info for the cell
    sd.chdir("/");
    if (sd.exists(configPath + "/config.json"))
    {
        // update the celll info
        file = sd.open(configPath + "/config.json");
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            file.close();
            return false;
        }
        else
        {
            ccd->parameters.cellId = doc["channelNumber"];
            ccd->overallMultiplier = doc["overallMultiplier"];
            ccd->isConAmTe = doc["isConAmTe"];
            ccd->ambTemp = doc["ambTemp"];
            ccd->noOfSubExps = doc["noOfSubExp"];
        }
    }
    else
    {
        Serial.println(F("Config not found."));
        return false;
    }
    ccd->isExpConfigured = true;
    file.close();
    sd.chdir("/");
    if (sd.exists(configPath + "/cell_info.json"))
    {
        // update the celll info
        file = sd.open(configPath + "/cell_info.json");
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        else
        {
            ccd->parameters.maxTemp = doc["maxTemp"];
            ccd->parameters.minTemp = doc["minTemp"];
            ccd->parameters.maxVoltage = doc["maxVolt"];
            ccd->parameters.minVoltage = doc["minVolt"];
            ccd->parameters.maxCurrent = doc["maxCur"];
        }
    }
    file.close();

    return true;
}

/**
 * @brief with the current experiment name. availabe on the object property
 * read through the sd card and fill the given expParams with approprite experiment setup
 * @param cellId nth cell 1-Max_cell_capable
 * @param expParamters parameters for the sub experiments
 * @return true on successful new sub exp placing
 * @return false otherwise
 */
bool ReadWriteExpAPI::setUpNextSubExp(ConstantChargeDischarge *ccd)
{
    StaticJsonDocument<400> doc;
    sd.chdir("/");
    String config = String(expName) + "/" + String(expName) + "_" + ccd->parameters.cellId + "/inputs/" + ccd->nthCurSubExp + "_config.json";
    // Serial.println(config);
    if (sd.exists(config))
    {
        if (ISLOGENABLED)
        {
            Serial.print(expName);
            Serial.print(F(", CH "));
            Serial.print(ccd->parameters.cellId);
            Serial.println(F(", config exists."));
        }
    }
    else
    {
        Serial.print(expName);
        Serial.print(F(", CH "));
        Serial.print(ccd->parameters.cellId);
        Serial.println(F(", config doesn't exist."));
        return false;
    }
    file = sd.open(config, FILE_READ);
    if (file)
    {
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            file.close();
            return false;
        }
        unsigned char mode = doc["mode"];
        float currentRate = doc["currentRate"];
        float resVal = doc["resVal"];
        float powVal = doc["powVal"];
        unsigned long timeLimit = doc["timeLimit"];
        if (mode == DriveCycle)
        {
            ccd->expParamters.total_n_samples = doc["total_n_samples"];
        }
        unsigned int multiplier = doc["multiplier"];
        float ambTemp = doc["ambTemp"];
        if (mode == Hold)
        {
            float holdVolt = doc["holdVolt"];
            ccd->expParamters.holdVolt = holdVolt;
        }
        if (mode == DriveCycle)
        {
            int sampleTime = doc["sampleTime"]; // in ms
            if (sampleTime > 0)
            {
                // if it founds then only change it otherwise leave it as default set by object.reset()
                ccd->expParamters.sampleTime = sampleTime;
            }
            else
            {
                // default values is 1000ms
                ccd->expParamters.sampleTime = 1000;
            }
        }
        float voltLimet = doc["voltLimit"];

        ccd->expParamters.voltLimit = voltLimet;
        ccd->expParamters.mode = mode;
        ccd->expParamters.resVal = resVal;
        ccd->expParamters.powVal = powVal;
        ccd->expParamters.currentRate = currentRate;
        ccd->expParamters.timeLimit = timeLimit;
        ccd->expParamters.multiplier = multiplier;
        ccd->expParamters.ambTemp = ambTemp;
        isHeaderWritten[ccd->parameters.cellId - 1] = false; // so that when writing the logs in csv file write the header row
    }
    else
    {
        Serial.println(F("Couldn't find the Sub-Exp."));
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool ReadWriteExpAPI::fillNextDriveCyclePortion(ConstantChargeDischarge *ccd, uint8_t n_samples)
{
    unsigned long t = millis();
    uint8_t cellIndex = ccd->parameters.cellId - 1;
    if (ccd->expParamters.mode != DriveCycle)
    {
        // only valid for drive cycle exp.
        return false;
    }
    sd.chdir("/");
    String exNameStr = String(expName);
    String config = "";
    config += exNameStr + "/" + exNameStr + "_" + ccd->parameters.cellId + "/inputs/" + ccd->nthCurSubExp + "_driveCycle.csv";
    // Serial.println(config);
    file = sd.open(config, FILE_READ);

    if (file)
    {
        if (file.seek(dcPtr[cellIndex]))
        {
            for (uint8_t i = 0; i < n_samples; i++)
            {
                // clearing the previous value in case end
                ccd->expParamters.samples_batch[i] = 0;
            }
            if (!file.available())
            {
                // roll back to begininig
                dcPtr[cellIndex] = 0;
                file.seek(0);
            }
            for (unsigned int i = 0; i < n_samples; i++)
            {
                if (dcPtr[cellIndex] == 0)
                {
                    file.readStringUntil('\n'); // to bypass the header name of the .csv file
                }
                file.readStringUntil(','); // to bypass the time column of the  first coulumn
                float data = file.readStringUntil('\n').toFloat();
                // Serial.println(data, 4);
                if (i + 1 >= ccd->expParamters.total_n_samples)
                {
                    // Serial.println(i);
                    break;
                }
                ccd->expParamters.samples_batch[i] = data;
                if (!file.available())
                {
                    // roll back to begininig
                    dcPtr[cellIndex] = 0;
                    file.seek(0);
                }
            }
        }
        dcPtr[cellIndex] = file.position(); // get the postion of next byte from where it to be read or write
    }
    else
    {
        Serial.println(F("Error opening the config file."));
        file.close();
        return false;
    }
    file.close();
    Serial.print(F("CH "));
    Serial.print(ccd->parameters.cellId);
    Serial.print(F(": Took "));
    Serial.print(millis() - t);
    Serial.println("ms to load DS.");
    return true;
}

bool ReadWriteExpAPI::logReadings(ConstantChargeDischarge *ccd, char *row)
{
    // on an average takes 15ms
    uint8_t channelId = ccd->parameters.cellId;
    sd.chdir("/");
    char path[2 * MAX_EXP_NAME_LENGTH + 10] = "";
    sprintf(path, "%s/%s_%d/outputs", expName, expName, channelId);
    // Serial.println(path);

    if (!isOpDirChecked[channelId - 1])
    {
        // if it is the first sub experiment of the series
        if (sd.exists(path))
        {
            // clean the existing directory
            //  to log new outputs
            FatFile cwd;
            if (!cwd.open(path))
            {
                Serial.println(F("CWD path open failed."));
            }
            if (!cwd.rmRfStar())
            {
                Serial.println(F("O/P dir exist. Remove failed."));
                return false;
            }
            // Serial.println(F("Cleaned o/p dir."));
            cwd.close();
        }
        sd.chdir("/");
        if (!sd.mkdir(path))
        {
            Serial.println(F("O/P dir make failed"));
            return false;
        }
        isOpDirChecked[channelId - 1] = true;
    }

    sd.chdir("/");

    if (!sd.chdir(path))
    {
        Serial.println(F("Dir change failed."));
        return false;
    }

    char newPath[20] = "";
    sprintf(newPath, "cycle_%d", ccd->currentMultiplierIndex);

    if (!sd.exists(newPath))
    {
        if (!sd.mkdir(newPath))
        {
            Serial.println(F("Cy. Dir make failed."));
            return false;
        }
    }
    if (!sd.chdir(newPath))
    {
        Serial.println(F("Cy. Dir change failed."));
        return false;
    }

    char config[20] = "";
    sprintf(config, "%d_logs.csv", ccd->nthCurSubExp);
    // Serial.println(config);

    file = sd.open(config, O_WRONLY | O_CREAT);
    if (file)
    {
        if (!isHeaderWritten[channelId - 1])
        {
            // write the header row first
            char head[100] = "";
            formHead(head, channelId);

            // Serial.println(head);
            file.println(head);
            logSDPointer[channelId - 1] = file.position();
            isHeaderWritten[channelId - 1] = true;
        }
        // Serial.println(row);
        if (file.seek(logSDPointer[channelId - 1]))
        {
            file.println(row);
            logSDPointer[channelId - 1] = file.position();
        }
    }
    else
    {
        Serial.println(F("Output directory creation failed."));
        file.close();
        return false;
    }
    file.close();
    // sd.ls("/",LS_R);
    return true;
}

/**
 * @brief formate the head for the output file for SD card
 *
 * @param head
 * @param cellId
 */
void ReadWriteExpAPI::formHead(char *head, uint8_t cellId)
{
    cellId--;
    const char *temp = "Time,Volt,Current";
    strcat(head, temp);
    strcat(head, ",Ch_T,Ch_H");
    for (uint8_t i = 0; i < no_of_temp_sen_connected_cell[cellId]; i++)
    {
        strcat(head, ",T");
        char buff[3] = "";
        itoa(i + 1, buff, 10);
        strcat(head, buff);
    }
}

/**
 * @brief clean a file directory
 * given that you already opened a filed and then call this method
 *
 * @return true
 * @return false
 */
bool ReadWriteExpAPI::cleanDir()
{

    if (!file)
    {
        Serial.println(F("invalid file."));
        return false;
    }
    char f_name[20];
    file = file.openNextFile(FILE_WRITE);
    while (file)
    {
        if (!file.isDirectory())
        {
            file.getName(f_name, 20);
            Serial.println(f_name);
            if (!sd.remove(f_name))
            {
                Serial.println(F("file remove failed"));
                file.close();
                return false;
            }
            else
            {
                Serial.println(F("file remove success"));
            }
        }
        file = file.openNextFile();
    }
    file.close();
    return true;
}

bool ReadWriteExpAPI::createDir(const char *dirName, bool clean)
{
    if (!sd.chdir("/"))
        return false;
    if (!sd.exists(dirName))
    {
        if (sd.mkdir(dirName))
            return true;
    }
    else if (clean)
    {
        // remove all the previous content
        FatFile cwd;
        if (!cwd.open(dirName))
        {
            Serial.println(F("CWD path open failed."));
        }
        if (!cwd.rmRfStar())
        {
            Serial.println(F("O/P dir exist. Remove failed."));
            return false;
        }
        // Serial.println(F("Cleaned o/p dir."));
        cwd.close();
        return createDir(dirName);
    }
    return true;
}

bool ReadWriteExpAPI::writeToFileStream(const char *path, Stream *stream, char readUntil)
{

    if (!sd.chdir("/"))
        return false;
    file = sd.open(path, O_WRONLY | O_CREAT);
    if (!file)
        return false;
    int d = 200; // atleast wait for some time to get delayed response
    unsigned long t = millis();
    while (millis() < t + d)
    {
        while (bytesAvailable(stream))
        {
            t = millis();
            char c = stream->read();
            if (c == readUntil)
                break;
            file.print(c);
        }
    }
    file.close();
    return true;
}

bool ReadWriteExpAPI::writeToFile(const char *path, char *content)
{

    if (!sd.chdir("/"))
        return false;
    file = sd.open(path, O_WRONLY | O_CREAT);
    if (!file)
        return false;
    file.print(content);
    file.close();
    return true;
}

int ReadWriteExpAPI::bytesAvailable(Stream *stream)
{
    int bytesAvailable = stream->available();

    if (bytesAvailable >= 60)
    {
        Serial.println(F("Warning: incoming buffer is almost full!"));
    }
    if (bytesAvailable == 64)
    {
        Serial.println(F("Error: incoming buffer overflow!"));
    }
    return bytesAvailable;
}

/**
 * Reads the number of free bytes available on the SD card.
 *
 * This method uses the SdFat library to determine the number of free
 * clusters on the SD card, and then calculates the total number of free
 * bytes based on the block size of the volume. The resulting value is
 * returned as a uint32_t.
 *
 * @return The number of free bytes on the SD card.
 */
uint32_t ReadWriteExpAPI::getFreeBytes()
{
    // Get the number of free clusters on the SD card
    uint32_t freeClusters = sd.vol()->freeClusterCount();

    // Calculate the total number of free bytes based on the volume's block size
    uint32_t blockSize = sd.vol()->sectorsPerCluster() / 2;
    uint32_t freeBytes = freeClusters * blockSize / 1024;

    // Return the total number of free bytes
    return freeBytes;
}

/**
 * @brief Format a sd card
 *
 */
void ReadWriteExpAPI::formatSD()
{
    sd.format();
}

void ReadWriteExpAPI::sizeCheck()
{
    uint32_t freeSize = getFreeBytes();
    Serial.print(F("Available size "));
    Serial.print(freeSize);
    Serial.println(F("KB"));
    if (freeSize <= 2000)
    {
        Serial.print(F("SD card almost full. Formatting ...."));
        formatSD();
        Serial.println(getFreeBytes());
    }
}

/**
 * @brief Load the config and start it
 *
 * @param exps
 * @return true
 * @return false
 */
bool ReadWriteExpAPI::loadExps(ConstantChargeDischarge *exps)
{
    Serial.print(F("Exp - "));
    Serial.println(expName);
    if (!sd.chdir("/"))
    {
        Serial.println(F("Chdir failed!"));
        return false;
    }
    File dir = sd.open(expName);
    if (!dir)
    {
        Serial.println(F("Config not exist"));
        return false;
    }
    bool isExpLoaded[N_CELL_CAPABLE];
    for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
    {
        isExpLoaded[i] = false;
    }
    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            entry.close();
            break;
        }
        if (entry.isDirectory())
        {
            char name[MAX_EXP_NAME_LENGTH + 5];
            entry.getName(name, MAX_EXP_NAME_LENGTH + 5);
            char *p = strtok(name, "_");
            p = strtok(NULL, "_");
            int channelNo = atoi(p);
            Serial.print(F("Loading on channel "));
            Serial.println(channelNo);

            exps[channelNo - 1] = ConstantChargeDischarge(channelNo);

            if (setup(&exps[channelNo - 1]) && exps[channelNo - 1].placeNewSubExp(this))
            {

                isExpLoaded[channelNo - 1] = true;
                if (exps[channelNo - 1].expParamters.mode == DriveCycle)
                {
                    if (!fillNextDriveCyclePortion(&exps[channelNo - 1]))
                        isExpLoaded[channelNo - 1] = false;
                }
            }
            else
            {
                Serial.print(F("Loading failed on channel "));
                Serial.println(channelNo);
            }
        }
        entry.close();
    }
    dir.close();
    for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
    {
        if (isExpLoaded[i])
        {
            exps[i].startCurrentSubExp(); // start and reserve the channel
        }
    }
    return true;
}

/**
 * Sends the file name and data over serial port.
 *
 * @param file: The file to be sent.
 */
void ReadWriteExpAPI::sendFile(File file)
{
    char filename[25];
    file.getName(filename, sizeof(filename));
    Serial.println(filename);
    Serial.write(':');
    while (file.available())
    {
        Serial.write(file.read());
    }
}

/**
 * Sends the contents of the directory over serial port.
 *
 * @param dirname: The name of the directory to be sent.
 */
void ReadWriteExpAPI::sendDirectory(String dirname)
{
    File dir_ = sd.open(dirname);
    while (true)
    {
        File file_ = dir_.openNextFile();
        if (!file_)
        {
            break;
        }
        if (file_.isDirectory())
        {
            char filename[25];
            file_.getName(filename, sizeof(filename));
            sendDirectory(dirname + "/" + filename);
        }
        else
        {
            sendFile(file_);
        }
        file_.close();
    }
    dir_.close();
}