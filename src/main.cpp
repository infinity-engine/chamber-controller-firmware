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
#include "ConversationAPI.h"
Adafruit_ADS1115 ads;

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "MCP_DAC.h"
MCP4921 MCP[6] = {{}, {}, {}, {}, {}, {}}; // create 6 instances of MCP4921

#include <SdFat.h>
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
SdFs sd;
FsFile dir;
FsFile file;

#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_card_module_cs, DEDICATED_SPI, SD_SCK_MHZ(16))
#else // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_card_module_cs, SHARED_SPI, SD_SCK_MHZ(16))
#endif // HAS_SDIO_CLASS

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

// allocate store for the objects in global space

ReadWriteExpAPI api; // 177 bytes

ConversationAPI cpi;

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
    if (exps[i].overallStatus == EXP_RUNNING)
    {
      // if exp is on running status
      if (exps[i].curExpStatus == EXP_RUNNING)
      {
        // there is an sub-exp placed, which is running
        exps[i].curExpStatus = exps[i].performAction(api, cpi);
      }

      // this step will be triggered on completion of previous sub exp as well as on the starting of new set of exp
      if (exps[i].curExpStatus == EXP_FINISHED)
      {
        // if the current sub-experiment is in finished status with all the row multiplier completed
        if (ISLOGENABLED)
        {
          Serial.print(F("CH "));
          Serial.print(exps[i].parameters.cellId);
          Serial.print(F(": Sub exp "));
          Serial.print(exps[i].nthCurSubExp);
          Serial.println(F(" finished."));
        }
        // check whther it was the last experiment among all the set of experiment for the particular cell to run
        if (exps[i].currentMultiplierIndex == exps[i].overallMultiplier && exps[i].nthCurSubExp == exps[i].noOfSubExps)
        {
          // on it's last sub exp of last cycle
          Serial.print(F("CH "));
          Serial.print(exps[i].parameters.cellId);
          Serial.println(F(": All cycles completed."));
          exps[i].overallStatus = EXP_FINISHED;
          cpi.setStatus(EXP_FINISHED, exps[i].parameters.cellId); // will update the last cycle and last row
          asAllExpFinished();
          continue;
        }
        else if (exps[i].nthCurSubExp == exps[i].noOfSubExps)
        {
          // not on its last cycle
          Serial.print(F("CH "));
          Serial.print(exps[i].parameters.cellId);
          Serial.print(F(": Cycle "));
          Serial.print(exps[i].currentMultiplierIndex);
          Serial.println(F(" completed."));
          cpi.incrementMultiplier(exps[i].parameters.cellId); // only curent cycle completed
        }

        if (exps[i].placeNewSubExp(&api))
        {
          exps[i].startCurrentSubExp(); // start and reserve the channel
        }
        else
        {
          exps[i].curExpStatus = EXP_STOPPED;
          // insert logic to send the status to cloud
        }
      }

      if (exps[i].curExpStatus == EXP_STOPPED)
      {
        // if any sub exp has stopped.
        exps[i].overallStatus = EXP_STOPPED;
      }
    }
  }
}

void asAllExpFinished()
{
  uint8_t allStatusCombined = EXP_FINISHED;
  for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
  {
    if (&exps[i] != NULL && exps[i].overallStatus == EXP_RUNNING)
    {
      return;
    }
    if (exps[i].overallStatus == EXP_STOPPED)
    {
      // if any of the exp is stopped in any channel then all combined status would be that
      allStatusCombined = EXP_STOPPED;
    }
  }
  Serial.println(F("All exps finished across all channels."));
  cpi.sendMsgID("<END");
  Serial2.print(allStatusCombined);
  Serial2.print("\n>");
}

void test()
{
  // api.reset((char *)"c7e7"); // set the expname
  // api.loadExps(exps);

  for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
  {
    ConstantChargeDischarge exp;
    exps[i] = exp; // empty the prev
  }

  while (!lookAndStartExp(&api, &cpi, exps))
  {
    Serial.println(F("Retrying to get exp."));
    delay(2000);
  }
  Serial.println(F("Exp Started."));
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
  while (!sd.begin(SD_CONFIG))
  {
    Serial.println(F("SD initialization failed."));
    delay(1000);
    Serial.println(F("Trying to reinitialize."));
    delay(1000);
  }
  api.sizeCheck();
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
