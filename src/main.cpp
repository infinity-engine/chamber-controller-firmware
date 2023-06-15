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
#endif
extern LiquidCrystal_I2C lcd;

DHT dht[] = {{DHTPIN_1, DHTTYPE_1}, {DHTPIN_2, DHTTYPE_2}, {DHTPIN_3, DHTTYPE_3}, {DHTPIN_4, DHTTYPE_4}};

ConstantChargeDischarge exps[N_CELL_CAPABLE]; // 177*6 = 1062 bytes global space

// allocate store for the objects in global space

ReadWriteExpAPI api; // 177 bytes

ConversationAPI cpi;

CallibrationParameters calParams[N_CELL_CAPABLE];

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
        Serial.print(F("CH "));
        Serial.print(exps[i].parameters.cellId);
        Serial.println(F(": Exp stopped."));
        exps[i].overallStatus = EXP_STOPPED;
        cpi.setStatus(EXP_STOPPED, exps[i].parameters.cellId); // update the cloud
      }
    }
  }
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
    clearLine(3);
    lcd.print(F("Trying to get EXP..."));
    delay(2000);
  }
  Serial.println(F("Exp Started."));
  clearLine(0);
  lcd.print(F("Ongoing Tests"));
  clearLine(1);
  lcd.print(F("C1:_ C2:_ C3:_ C4:_"));
  clearLine(2);
  lcd.print(F("C5:_ C6:_"));
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
  updateLCDView(true);
  Serial.println(F("All exps finished across all channels."));
  clearLine(3);
  lcd.print(F("All finsished"));
  delay(1000);
  cpi.sendMsgID("<END");
  Serial2.print(allStatusCombined);
  Serial2.print("\n>");
  Serial.println(F("Sending data to cloud ...."));
  clearLine(3);
  lcd.print(F("Data -> cloud"));
  cpi.clearInputBuffer();
  while (true)
  {
    cpi.recvWithStartEndMarkers();
    if (cpi.newData)
    {
      if (strcmp("SEND_OK", cpi.receivedChars) == 0)
      {
        Serial.println(F("Send data to cloud success."));
        clearLine(3);
        lcd.print(F("Cloud -> OK"));
        break;
      }
    }
  }
  delay(1000);
  Serial.println(F("Going to take a nap."));
  Serial.print(F("Zzz.."));
  lcd.clear();
  clearLine(3);
  lcd.print(F("Zzz......."));
  for (uint8_t i = 0; i < 100; i++)
  {
    Serial.print(F("."));
    delay(500);
  }
  Serial.println();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Woke up!"));
  Serial.println(F("Woke up!"));
  delay(2000);
  clearLine(0);
  lcd.print(F("Battery Test Chamber"));
  test();
}

void blink(int _delay = 50)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(_delay);
  digitalWrite(LED_BUILTIN, LOW);
  delay(_delay);
}

void debug();

void setup()
{
  // debug_init();
  Serial.begin(2000000);
  Serial.println(F("Starting Engine !!"));

  if (ISLOGENABLED)
  {
    Serial.print("Free SRAM ");
    Serial.print(getFreeSram());
    Serial.println(" Bytes");
  }

  blink();
  pinInit();
  calibrationInit();
  lcd_init();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Battery Test Chamber"));

  blink();
  while (!ads.begin())
  {
    Serial.println(F("ADS initialization failed."));
    delay(500);
    Serial.println(F("Trying to reinitialize."));
    delay(1000);
  }

  clearLine(2);
  lcd.print(F("ADS OK"));
  Serial.println(F("ADS initialization success."));

  blink();
  while (!sd.begin(SD_CONFIG))
  {
    Serial.println(F("SD initialization failed."));
    delay(1000);
    Serial.println(F("Trying to reinitialize."));
    delay(1000);
  }
  api.sizeCheck();
  Serial.println(F("SD initialization success."));
  clearLine(2);
  lcd.print(F("SD OK"));
  for (unsigned char i = 0; i < no_of_discharger_connected; i++)
  {
    MCP[i].begin(chip_select_pin_location_discharger[i]);
  }

  configureNoOfSensorConnected();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Battery Test Chamber"));
  lcd.setCursor(0, 2);
  lcd.print(F("Waiting for INS!"));
  Serial.println(F("Waiting for INS!"));
  unsigned long t = millis();
  while (millis() < t + 2000)
  {
    if (Serial.available())
    {
      String code = Serial.readStringUntil('\n');
      Serial.print(F("Received Code : "));
      Serial.println(code);
      if (code == "CAL")
      {
        // send the device in callibration mode
        // which is on chargin
        Serial.println(F("Gone into callibration."));
        clearLine(2);
        lcd.print(F("In Callibration"));
        for (uint8_t i = 0; i < N_CELL_CAPABLE; i++)
        {
          setCellChargeDischarge(i + 1, relay_cell_charge);
        }
        while (true)
        {
          /* code */
        }
      }
      else
      {
        // read the file name and send everycontetn of it
        clearLine(2);
        lcd.print(F("In Data Transfer"));
        api.sendDirectory(code);
      }
    }
  }

  // blink(2000);
  clearLine(2);
  test();
}

void loop()
{
  // put your main code here, to run repeatedly:
  runExp();
  updateLCDView();
  // debug();
}

void debug()
{
  // Serial.println("Doing");
  // bool address[4] = {true, false, false, false};
  // channelTheMux(address);
  // for (unsigned int i = 0; i < temp_average_sample_count; i++)
  // {
  //   float raw = measureFromADS(tem_sen_ads_location[0][1]);
  //   Serial.println(raw);
  // }
  Serial.println("Again");
  bool address[4] = {true, true, false, true};
  channelTheMux(address);
  Serial.println(measureFromADS(cur_sen_ads_location[3]));
  delay(4000);
}
