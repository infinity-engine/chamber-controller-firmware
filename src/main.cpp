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

#ifndef MEMORY
#define MEMORY
#include <MemoryFree.h>
#endif

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

void initExp()
{
  exps = {
      {NULL, NULL, NULL, NULL, NULL, NULL},//exp
      {0, 0, 0, 0, 0, 0},//cur exp status
      {true, true, true, true, true, true},//is free for new exps
      {true, true, true, true, true, true},//is last exp
      {0, 0, 0, 0, 0, 0},//no of sub exps
      {0, 0, 0, 0, 0, 0}};//nth cur exp
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
      // potential new api call
      exps.curExpStatus[i] = 0;
      exps.isFreeForNewExps[i] = false;
      exps.nthCurExp[i] = 1;//denotes the nth sub experiment on the current set of experiments
      if (exps.nthCurExp[i] == exps.noOfSubExps[i])
      {
        exps.isLastExp[i] = true;
      }
    }
  }
}

void runExp()
{
  // each cell takes around 135 ms time while discharging
  for (unsigned char i = 0; i < N_CELL_CAPABLE; i++)
  {
    if (exps.exp[i] != NULL && exps.curExpStatus[i] == 0 && exps.isFreeForNewExps[i] == false)
    {
      // get the details and do what you wanna do with it and update curExpStatus if required
      // potential sd card calls
      exps.curExpStatus[i] = exps.exp[0]->performAction();
      Serial.print("Current(A):");
      Serial.print(exps.exp[0]->measurement.current);
      Serial.print(",Voltage:");
      Serial.print(exps.exp[0]->measurement.voltage);
      Serial.print(",Cell Temp. 1:");
      Serial.println(exps.exp[0]->measurement.temperature[0]);
    }
    if (exps.curExpStatus[i] == 1)
    {
      // if the current experiment is in finished status
      Serial.println(F("Sub exp. is finished."));
      // check whther it was the last experiment among all the set of experiment for the particular cell to run
      if (exps.isLastExp[i])
      {
        // add some finishing touches
        Serial.println(F("All sub-exps are finished."));
      }
      else
      {
        // potential new sd card read, fetch the sub experiment, and run it
        exps.nthCurExp[i] += 1; // increment the sub exp count
        if (exps.nthCurExp[i] == exps.noOfSubExps[i])
        {
          exps.isLastExp[i] = true;
        }
      }
      resetChannel(i);
    }
    else if (exps.curExpStatus[i] == 2)
    {
      // if any sub exp has stopped.
      Serial.println("Stopped");
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

void test()
{
  ConstantChargeDischarge e1 = {1,7};
  e1.expParamters.disChargeRate = 0;
  e1.setup();
  // ConstantChargeDischarge e2 = {2};
  // e2.expParamters.disChargeRate = 0.2;
  // e2.setup();
  // ConstantChargeDischarge e3 = {3};
  // e3.expParamters.disChargeRate = 0.2;
  // e3.setup();
  // ConstantChargeDischarge e4 = {4};
  // e4.expParamters.disChargeRate = 0.2;
  // e4.setup();
  // ConstantChargeDischarge e5 = {5};
  // e5.expParamters.disChargeRate = 0.2;
  // e5.setup();
  // ConstantChargeDischarge e6 = {1};
  // e6.expParamters.disChargeRate = 0.2;
  // e6.setup();
  exps.exp[0] = &e1;
  // exps.exp[1] = &e2;
  // exps.exp[2] = &e3;
  // exps.exp[3] = &e4;
  // exps.exp[4] = &e5;
  // exps.exp[5] = &e6;
}

void setup()
{
  //debug_init();
  Serial.begin(115200);
  pinInit();
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
  initExp();
  test();
  Serial.print(F("Available RAM "));
  Serial.print(freeMemory());
  Serial.println(F("Bytes"));
}

void loop()
{
  // put your main code here, to run repeatedly:
  // unsigned long t = millis();
  fillExp();
  // Serial.print("Fill ");
  // Serial.println(millis()-t);
  // t = millis();
  runExp();
  // Serial.print("Run ");
  // Serial.println(millis()-t);
  // unsigned long t1 = millis();
  // myFile = SD.open("exp_12_1.txt",O_WRITE);
  // if (myFile)
  // {
  //   myFile.println("145\\t3.2\\t1.0\\t25\\t25\\t25\\t25\\t24\\t25.5");
  //   myFile.close();
  // }
  // else
  // {
  //   Serial.println("SD write failed");
  // }
  // Serial.println(millis() - t1);
  // delay(1000);
  //Serial.println(measureFromADS(0));
}