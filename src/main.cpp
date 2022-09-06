#include <Arduino.h>

// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include "avr8-stub.h"
#ifndef PROTOTYPE
#define PROTOTYPE
#include "functionPrototype.h"
#endif
#ifndef CONFIG_ATMEGA
#define CONFIG_ATMEGA
#include "config_atmega.h"
#endif
#ifndef CONFIG_CONST
#define CONFIG_CONST
#include "config_const.h"
#endif
#ifndef CYCCHDIS
#define CYCCHDIS
#include "CyclicChargeDischarge.cpp"
#endif
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
Adafruit_ADS1115 ads;

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "MCP_DAC.h"
MCP4921 MCP[6] = {{}, {}, {}, {}, {}, {}}; // create 6 instances of MCP4921

#include <SPI.h>
#include <SD.h>
File myFile;

DHT dht[] = {{DHTPIN_1, DHTTYPE_1}, {DHTPIN_2, DHTTYPE_2}, {DHTPIN_3, DHTTYPE_3}, {DHTPIN_4, DHTTYPE_4}};

struct myStructure
{
  ConstantChargeDischarge *exp[N_CELL_CAPABLE];
  unsigned char curExpStatus[N_CELL_CAPABLE]; // corresponds to each cell configured
  bool isFreeForNewExps[N_CELL_CAPABLE];      // corresponds to each cell configured, also point to last setof exps
  bool isLastExp[N_CELL_CAPABLE];
  unsigned int noOfSubExps[N_CELL_CAPABLE];
  unsigned int nthCurExp[N_CELL_CAPABLE];
};
struct myStructure exps;

void initExp(){
  exps = {
    {NULL,NULL,NULL,NULL,NULL,NULL},
    {0,0,0,0,0,0},
    {true,true,true,true,true,true},
    {true,true,true,true,true,true},
    {0,0,0,0,0,0},
    {0,0,0,0,0,0}
    };
}

void fillExp()
{
  for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
  {
    if (exps.isFreeForNewExps[i])
    {
      // get new set of sub experiment from an api call store them in sd card
      // create a new instance of the first sub exp
      // start the exp
      // exps.exp[i] = instance;
      // set noOfSubExps
      exps.curExpStatus[i] = 0;
      exps.isFreeForNewExps[i] = false;
      exps.nthCurExp[i] = 1;
      if (exps.nthCurExp[i] == exps.noOfSubExps[i])
      {
        exps.isLastExp[i] = true;
      }
    }
  }
}

void runExp()
{
  for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
  {
    if (exps.exp[i] != NULL && exps.curExpStatus[i] == 0 && exps.isFreeForNewExps[i] == false)
    {
      // get the details and do what you wanna do with it and update curExpStatus if required
      // potential new api calls
    }
    if (exps.curExpStatus[i] == 1)
    {
      // if the current experiment is in finished status

      // check whther it was the last experiment among all the set of experiment for the particular cell to run
      if (exps.isLastExp[i])
      {
        // add some finishing touches
      }
      else
      {
        // potential new sd card read, fetch the sub experiment, and run it
        exps.nthCurExp[i] += 1;//increment the sub exp count
        if (exps.nthCurExp[i] == exps.noOfSubExps[i])
        {
          exps.isLastExp[i] = true;
        }
      }
      exps.exp[i] = NULL;
    }
    else if (exps.curExpStatus[i] == 2)
    {
      // if any sub exp has stopped.
    }
  }
}

void resetChannel(unsigned char channelId)
{
  // if somehow the experiment is stopped in between you have to reset it before you can place new experiment on it.
  exps.isFreeForNewExps[channelId] = true; // on setting it true, fillExp will then fetch for new available exp
  exps.curExpStatus[channelId] = 0;
  exps.exp[channelId] = NULL;
}

void test(){
  ConstantChargeDischarge e1 = {1};
}

void setup()
{
  // debug_init();
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  while (!ads.begin())
  {
    Serial.println(F("ADS initialization failed."));
    delay(500);
    Serial.println(F("Trying to reinitialize."));
    delay(1000);
  }
  Serial.println(F("ADS initialization success."));
  while (!SD.begin(SD_card_module_cs))
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
  test();
}

void loop()
{
  // put your main code here, to run repeatedly:
  initExp();
  fillExp();
  runExp();
}