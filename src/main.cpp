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
#include <SPI.h>
Adafruit_ADS1115 ads;

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "MCP_DAC.h"
MCP4921 MCP[6] = {{}, {}, {}, {}, {}, {}}; // create 6 instances of MCP4921

#include <SPI.h>
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

struct myStructure
{
  ConstantChargeDischarge *exp[N_CELL_CAPABLE]; // store the object for any sub experiment per channel
  unsigned int noOfSubExps[N_CELL_CAPABLE];     // total no of sub-exp to be run for this particular channel
  unsigned int nthCurExp[N_CELL_CAPABLE];       // points to what no of experiment the current channel is running
  unsigned char curExpStatus[N_CELL_CAPABLE];   // corresponds to each cell configured; -> sub-exp
  bool isFreeForNewExps[N_CELL_CAPABLE];        // corresponds to each cell configured, also point to last setof exps
  bool isLastExp[N_CELL_CAPABLE];
};
struct myStructure exps; // 54 bytes, exp only holds the ConstantChargeDischarge's address

void formRow(char *row,ConstantChargeDischarge *exp);


ConstantChargeDischarge expStore[N_CELL_CAPABLE]; // 177*6 = 1062 bytes global space

// allocate store for the objects in global space

ReadWriteExpAPI api; // 177 bytes

/**
 * @brief open all the channel for set of experiments
 *
 */
void initExp()
{
  exps = {
      {NULL, NULL, NULL, NULL, NULL, NULL},                                                                   // exp
      {EXP_NOT_STARTED, EXP_NOT_STARTED, EXP_NOT_STARTED, EXP_NOT_STARTED, EXP_NOT_STARTED, EXP_NOT_STARTED}, // cur exp status
      {false, false, false, false, false, false},                                                             // is free for new exps
      {true, true, true, true, true, true},                                                                   // is last exp
      {0, 0, 0, 0, 0, 0},                                                                                     // no of sub exps
      {0, 0, 0, 0, 0, 0}};                                                                                    // nth cur exp
  for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
  {
    expStore[i].reset(i + 1);
  }
}

/**
 * @brief get new set of sub experiment from an api call store them in sd card;
 * create a new instance of the first sub exp;
 * store the exp name in the readwrite api
 * exps.exp[i] = instance;
 * set noOfSubExps;
 * Also, start the first sub experiment of each series.
 * potential new api call;
 */
void fillExp()
{
  for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
  {
    if (exps.isFreeForNewExps[i])
    {
      if (false)
      {
        resetChannel(i + 1);
        // get the exp name and no of sub exp and channel id to place the experiment
        // assuming those experiment details are already written in the sd card
        // let say from esp you get the new experiment name to be XXXXXXX
        // so you place the exp name on the api

        // api.resetAPIChannel(i+1,"XXXXXXX");
        // exps.noOfSubExps[i] = set it

        reserveChannel(i + 1);
        // making the sub-exp ->0 and status 1, so that this will be catched by
        //  runExp condition "if (exps.curExpStatus[i] == 1)"
        //  rest of the taken care of by that, like updating the exp parameters and all
        exps.nthCurExp[i] = 0;
        exps.curExpStatus[i] = 1;
      }
    }
  }
}

/**
 * @brief reserve the channel so that no new set of experiment can not be placed on it
 * used when placing new set of experiment on it
 * @param channelId 1-Max channel allowed
 */
void reserveChannel(unsigned char channelId)
{
  channelId--;
  exps.curExpStatus[channelId] = EXP_RUNNING;
  exps.isFreeForNewExps[channelId] = false;
}

/**
 * @brief reset the channel so that it becomes free to place new experiment.
 * If somehow the experiment is stopped in between you have to reset it before you can place new experiment on it.
 * @param channelId 1-max channel allowed
 */
void resetChannel(unsigned char channelId, bool hardReset)
{
  channelId--;
  if (hardReset)
  {
    exps.isFreeForNewExps[channelId] = true; // on setting it true, fillExp will then fetch for new available exp
    exps.isLastExp[channelId] = false;
    exps.noOfSubExps[channelId] = 0;
    exps.nthCurExp[channelId] = 0;
  }
  exps.curExpStatus[channelId] = EXP_NOT_STARTED;
  exps.exp[channelId] = NULL;
}

/**
 * @brief Run the particular sub experiment for each channel
 * if the sub experiment is completed then call readwrite api and place the new sub experiment and run it.
 *
 */
void runExp()
{
  // time for each channel takes around 135ms while discharging-experiment
  for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
  {
    if (exps.exp[i] != NULL)
    {
      if (exps.curExpStatus[i] == EXP_RUNNING && exps.isFreeForNewExps[i] == false)
      {
        // there is an sub-exp placed, which is running, and the channel is occupied
        measureAndRecord(i + 1);
      }

      // this step will be triggered on completion of previous sub exp as well as on the starting of new set of exp
      if (exps.curExpStatus[i] == EXP_FINISHED)
      {
        // if the current sub-experiment is in finished status
        Serial.print(F("Sub exp. "));
        Serial.print(exps.nthCurExp[i]);
        Serial.println(F(" finished."));
        // check whther it was the last experiment among all the set of experiment for the particular cell to run
        if (exps.isLastExp[i])
        {
          // all set of sub exps for the particular channel has finished.
          // add some finishing touches
          Serial.println(F("All sub-exps are finished."));
          api.resetAPIChannel(i + 1);
          resetChannel(i + 1);
        }
        else
        {
          resetChannel(i + 1, false);
          if (placeNewSubExp(i + 1))
          {
            reserveChannel(1); // start and reserve the channel
          }
          else
          {
            exps.curExpStatus[i] = EXP_STOPPED;
          }
        }
      }
      if (exps.curExpStatus[i] == EXP_STOPPED)
      {
        // if any sub exp has stopped.
      }
    }
  }
}

void measureAndRecord(uint8_t channelId)
{
  // get the details and do what you wanna do with it and update curExpStatus if required
  // potential sd card calls
  uint8_t i = channelId - 1;
  exps.curExpStatus[i] = exps.exp[i]->performAction(api);
  char row[150]="";
  formRow(row,exps.exp[i]);
  if (!api.logReadings(i + 1,row))
  {
    Serial.println(F("Log data failed"));
    exps.curExpStatus[i] = EXP_STOPPED;
    return;
  }
  Serial.print(F("Current:"));
  Serial.print(exps.exp[i]->measurement.current);
  Serial.print(F(",Voltage:"));
  Serial.print(exps.exp[i]->measurement.voltage);
  Serial.print(F(",Cell Temp.(°C):"));
  Serial.print(exps.exp[i]->measurement.avgTemperature);
  Serial.print(F(",Chamber Humidity(%):"));
  Serial.print(exps.exp[i]->chmMeas.avgHum);
  Serial.print(F(",Chamber Temp.(°C):"));
  Serial.println(exps.exp[i]->chmMeas.avgTemp);
}

bool placeNewSubExp(uint8_t channelId)
{
  bool status = true;
  uint8_t i = channelId - 1;
  Serial.print(F("Placing sub-exp "));
  Serial.println(exps.nthCurExp[i] + 1);
  exps.nthCurExp[i] += 1; // increment the sub exp count
  if (exps.nthCurExp[i] == exps.noOfSubExps[i])
  {
    exps.isLastExp[i] = true;
  }
  expStore[i].reset(i+1);
  exps.exp[i] = &expStore[i]; // 177 bytes
  if (api.setUpNextSubExp(channelId, &exps.exp[i]->expParamters))
  {
    // update the exp from the data received and already placed on object property.
    exps.exp[i]->setup();
    status = true;
  }
  else
  {
    // block the channel
    status = false;
    Serial.println(F("Stopped"));
  }
  return status;
}

void test()
{
  resetChannel(1);
  api.resetAPIChannel(1, "BGH0485978");
  if (placeNewSubExp(1))
  {
    exps.noOfSubExps[0] = 3; // this one has to updated from all exp together, you should fetch it from sd card
    reserveChannel(1);       // start and reserve the channel
  }
  else
  {
    exps.curExpStatus[0] = EXP_NOT_STARTED;
  }
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
  Serial.begin(115200);
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
  initExp();
  //sd.ls("/",LS_R);
  //debug();
  test();
}

void loop()
{
  // put your main code here, to run repeatedly:
  // fillExp();
  runExp();
}

void debug()
{
  file = sd.open("BGH0485978/outputs/1_logs.csv");
  if (file)
  {
    dumpFile();
  }
  else
  {
    Serial.println(F("Error opening file."));
  }
  file.close();
  // FatFile cwd;
  // sd.chdir("/");
  // sd.remove("BGH0485978/1_logs.csv");
  // api.resetAPIChannel(1,"BGH0485978");
  // api.currentSubExpNo[0] = 1;
  // api.isHeaderWritten[0] = false;
  // struct CellMeasurement m;
  // struct ChamberMeasurement c;
  // api.logReadings(1,&m,&c);
  // delay(5000);
  // sd.ls("/",LS_R);
}
