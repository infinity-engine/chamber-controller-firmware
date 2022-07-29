#include <Arduino.h>
#include "config_atmega.h"

void channelTheMux(bool address[]){
  bool s0 = address[0];
  bool s1 = address[1];
  bool s2 = address[2];
  bool s3 = address[3];
  digitalWrite(S0,s0);
  digitalWrite(S1,s1);
  digitalWrite(S2,s2);
  digitalWrite(S3,s3);
}
