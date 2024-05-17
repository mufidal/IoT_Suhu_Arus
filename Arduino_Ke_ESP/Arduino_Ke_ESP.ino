#include <SoftwareSerial.h>
#include "ACS712.h"

SoftwareSerial TES(8,9);
SoftwareSerial TES2(6,7);
ACS712 sensor(ACS712_20A, A1);

int peak;
int peaksebelum;
float vmax;
float vrms; 
unsigned long counter;
unsigned long countersebelum;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  TES.begin(115200);
  TES2.begin(115200);
  sensor.calibrate();
}

void loop() {
  // put your main code here, to run repeatedly:
  float I = sensor.getCurrentAC();
  if(I < 0.08){
    I = 0;
    //delay(500);
  }
  //Serial.println("Current : ");
  Serial.println(I);
  TES.println(I);
  
  counter = millis();
  peak = analogRead(A0);
  //Serial.println(peak);
  if(peak > peaksebelum){
    peaksebelum = peak;
    if(peak >= 512){
      vmax = peak;
    }
  }
if(peak < peaksebelum){
    peaksebelum = peak;
    vrms = ((0.9395*vmax)-479.3);
    if(counter - countersebelum >= 500){
      countersebelum = counter;
      //Serial.print("vrms : ");
      if(vrms <= 7){
        vrms = 0;
        Serial.println(vrms);
      }
      Serial.println(vrms);
      TES2.println(vrms);
    }
  }
  /*Serial.println(analogRead(36));
  delay(1);*/
  
}
