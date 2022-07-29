#include <Arduino.h>

// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include "avr8-stub.h"

#ifndef CUSTOM_H
#define CUSTOM_H
#include "functionPrototype.h"
#include "config_atmega.h"
#endif


#ifndef ADAFRUIT_ADS1X15_H
#define ADAFRUIT_ADS1X15_H
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
Adafruit_ADS1115 ads;
#endif


#include <Adafruit_Sensor.h>
#include <DHT.h>

DHT  dht[] = {{DHTPIN_1, DHTTYPE_1},{DHTPIN_2, DHTTYPE_2},{DHTPIN_3, DHTTYPE_3},{DHTPIN_4, DHTTYPE_4}};

DHT sht(12,DHT22);

void setup() {
  // put your setup code here, to run once:
  //debug_init();
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  while(!ads.begin()){
    Serial.println("ADS initialization failed.");
  }
  doNothing();
  measureCellVoltage(2);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, true);
  delay(1000);
  digitalWrite(13, false);
  delay(1000);
}