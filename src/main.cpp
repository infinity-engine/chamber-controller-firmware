#include <Arduino.h>

// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include "avr8-stub.h"

#include "functionPrototype.h"
#include "config_atmega.h"
#include "config_const.h"

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
}

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(13, true);
  delay(1000);
  digitalWrite(13, false);
  delay(1000);
}