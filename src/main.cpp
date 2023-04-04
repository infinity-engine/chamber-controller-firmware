#include <Arduino.h>

// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include "avr8-stub.h"

#include "functionPrototype.h"
#include "config_const.h"
#include "ReadWriteEXPAPI.h"
#include "ConstantChargeDischarge.h"
#include "config_atmega.h"
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "MCP_DAC.h"
MCP4921 MCP[6] = {{}, {}, {}, {}, {}, {}}; // create 6 instances of MCP4921

#include <SdFat.h>
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
#if SD_FAT_TYPE == 0
SdFat sd;
File dir;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 dir;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile dir;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile dir;
FsFile file;
#else // SD_FAT_TYPE
#error invalid SD_FAT_TYPE
#endif // SD_FAT_TYPE

#ifndef MEMORY
#define MEMORY
#include <MemoryFree.h>
#endif

// LCD address 0X27
#ifndef LCD
#define LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#endif
byte zero[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};
byte one[] = {
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000};
byte two[] = {
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000};
byte three[] = {
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100};
byte four[] = {
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110};
byte five[] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111};

DHT dht[] = {{DHTPIN_1, DHTTYPE_1}, {DHTPIN_2, DHTTYPE_2}, {DHTPIN_3, DHTTYPE_3}, {DHTPIN_4, DHTTYPE_4}};

ConstantChargeDischarge exps[N_CELL_CAPABLE]; // 177*6 = 1062 bytes global space

void formRow(char *row, ConstantChargeDischarge *exp);

// allocate store for the objects in global space

ReadWriteExpAPI api; // 177 bytes

/**
 * @brief get new set of sub experiment, for all the channels from an api call store them in sd card;
 * create a new instance of the first sub exp;
 * store the exp name in the readwrite api
 * exps.exp[i] = instance;
 * set noOfSubExps;
 * Also, start the first sub experiment of each series.
 * potential new api call;
 */
void fillExp()
{
}

/**
 * @brief Run the particular sub experiment for each channel
 * if the sub experiment is completed then call readwrite api and place the new sub experiment and run it.
 *
 */
void runExp()
{
  for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
  {
    // if any exp object is found for this channel
    if (exps[i].overallStatus == EXP_RUNNING)
    {
      // if exp is on running status
      if (exps[i].curExpStatus == EXP_RUNNING)
      {
        // there is an sub-exp placed, which is running
        if (exps[i].expParamters.mode == DriveCycle)
        {
          // frequent update is only necessary in drive cycle mode
          measureAndRecord(i + 1, false);
        }
        else if (millis() - exps[i].expParamters.prevTime > sample_update_delay)
        {
          measureAndRecord(i + 1, false);
        }
      }

      // this step will be triggered on completion of previous sub exp as well as on the starting of new set of exp
      if (exps[i].curExpStatus == EXP_FINISHED)
      {
        // if the current sub-experiment is in finished status with all the row multiplier completed
        if (ISLOGENABLED)
        {
          Serial.print(F("Sub exp "));
          Serial.print(exps[i].nthCurSubExp);
          Serial.println(F(" finished."));
        }
        // check whther it was the last experiment among all the set of experiment for the particular cell to run
        if (exps[i].currentMultiplierIndex == exps[i].overallMultiplier && exps[i].nthCurSubExp == exps[i].noOfSubExps)
        {
          // on it's last cycle
          Serial.print(F("All cycle has been completed; CH - "));
          Serial.println(i + 1);
          exps[i].overallStatus = EXP_FINISHED;
          continue;
        }
        else if (exps[i].nthCurSubExp == exps[i].noOfSubExps)
        {
          // not on its last cycle
          Serial.print(F("This cycle has been completed; CH - "));
          Serial.println(i + 1);
          exps[i].nthCurSubExp = 0;
          exps[i].currentMultiplierIndex += 1;
        }
        if (exps[i].placeNewSubExp(&api))
        {
          exps[i].startCurrentSubExp(); // start and reserve the channel
        }
        else
        {
          exps[i].curExpStatus = EXP_STOPPED;
          exps[i].overallStatus = EXP_STOPPED;
          // insert logic to send the status to cloud
        }
      }
      if (exps[i].curExpStatus == EXP_STOPPED)
      {
        // if any sub exp has stopped.
      }
    }
  }
}

void measureAndRecord(uint8_t channelId, bool logOnSerial)
{
  // get the details and do what you wanna do with it and update curExpStatus if required
  // potential sd card calls
  uint8_t i = channelId - 1;
  exps[i].performAction(api);
  char row[150] = "";
  formRow(row, &exps[i]);
  if (!api.logReadings(&exps[i], row))
  {
    Serial.println(F("Log data failed"));
    exps[i].curExpStatus = EXP_STOPPED;
    exps[i].overallStatus = EXP_STOPPED;
    return;
  }
  if (logOnSerial || ISLOGENABLED)
  {
    Serial.print(F("Current:"));
    Serial.print(exps[i].measurement.current, 4);
    Serial.print(F(",Voltage:"));
    Serial.print(exps[i].measurement.voltage);
    Serial.print(F(",Cell Temp.(°C):"));
    Serial.print(exps[i].measurement.avgTemperature);
    Serial.print(F(",Chamber Humidity(%):"));
    Serial.print(exps[i].chmMeas.avgHum);
    Serial.print(F(",Chamber Temp.(°C):"));
    Serial.println(exps[i].chmMeas.avgTemp);
  }
}

void test()
{
  api.reset((char *)"c7e7"); // set the expname
  exps[0] = ConstantChargeDischarge();
  if (api.setup(&exps[0]) && exps[0].placeNewSubExp(&api))
  {
    exps[0].startCurrentSubExp(); // start and reserve the channel
  }

  // // on channel 2
  // resetChannel(2);
  // api.resetAPIChannel(2);
  // if (placeNewSubExp(2))
  // {
  //   exps.noOfSubExps[1] = 4; // this one has to updated from all exp together, you should fetch it from sd card
  //   reserveChannel(2);       // start and reserve the channel
  // }
  // else
  // {
  //   exps.curExpStatus[1] = EXP_NOT_STARTED;
  // }

  // // on channel 3
  // resetChannel(3);
  // api.resetAPIChannel(3);
  // if (placeNewSubExp(3))
  // {
  //   exps.noOfSubExps[2] = 2; // this one has to updated from all exp together, you should fetch it from sd card
  //   reserveChannel(3);       // start and reserve the channel
  // }
  // else
  // {
  //   exps.curExpStatus[2] = EXP_NOT_STARTED;
  // }

  // // on channel 4
  // resetChannel(4);
  // api.resetAPIChannel(4);
  // if (placeNewSubExp(4))
  // {
  //   exps.noOfSubExps[3] = 4; // this one has to updated from all exp together, you should fetch it from sd card
  //   reserveChannel(4);       // start and reserve the channel
  // }
  // else
  // {
  //   exps.curExpStatus[3] = EXP_NOT_STARTED;
  // }

  // // on channel 5
  // resetChannel(5);
  // api.resetAPIChannel(5);
  // if (placeNewSubExp(5))
  // {
  //   exps.noOfSubExps[4] = 4; // this one has to updated from all exp together, you should fetch it from sd card
  //   reserveChannel(5);       // start and reserve the channel
  // }
  // else
  // {
  //   exps.curExpStatus[4] = EXP_NOT_STARTED;
  // }

  // // on channel 6
  // resetChannel(6);
  // api.resetAPIChannel(6, "PGH0485972");
  // if (placeNewSubExp(6))
  // {
  //   exps.noOfSubExps[5] = 4; // this one has to updated from all exp together, you should fetch it from sd card
  //   reserveChannel(6);       // start and reserve the channel
  // }
  // else
  // {
  //   exps.curExpStatus[5] = EXP_NOT_STARTED;
  // }
}

void lcd_init()
{
  lcd.init(); // initialise the lcd
  lcd.createChar(0, zero);
  lcd.createChar(1, one);
  lcd.createChar(2, two);
  lcd.createChar(3, three);
  lcd.createChar(4, four);
  lcd.createChar(5, five);
  lcd.backlight(); // turn on backlight
  lcd.clear();
  lcd.setCursor(0, 0); // column,row
  lcd.print(F("Battery Test Chamber"));
  delay(100);
  lcd.setCursor(0, 2);
  lcd.print(F("Starting Engine"));
  for (uint8_t i = 0; i < 80; i++)
  {
    updateProgressBar(i, 80, 3);
    delay(1);
  }
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Battery Test Chamber"));
  lcd.setCursor(0, 2);
  lcd.print(F("Ready!"));
}

void setup()
{
  // debug_init();
  Serial.begin(2000000);
  Serial.println(F("Starting Engine !!"));
  Serial.print("Free SRAM ");
  Serial.print(getFreeSram());
  Serial.println(" Bytes");
  pinInit();
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // lcd_init();
  while (!ads.begin())
  {
    Serial.println(F("ADS initialization failed."));
    delay(500);
    Serial.println(F("Trying to reinitialize."));
    delay(1000);
  }
  Serial.println(F("ADS initialization success."));
  while (!sd.begin(SD_card_module_cs))
  {
    Serial.println(F("SD initialization failed."));
    delay(1000);
    Serial.println(F("Trying to reinitialize."));
    delay(1000);
  }
  Serial.println(F("SD initialization success."));

  for (unsigned char i = 0; i < no_of_discharger_connected; i++)
  {
    MCP[i].begin(chip_select_pin_location_discharger[i]);
  }
  // sd.ls("/",LS_R);
  // debug();
  test();
}

void loop()
{
  // put your main code here, to run repeatedly:
  runExp();
}
