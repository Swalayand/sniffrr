/*
   -------------------------------------------------------------------------------------
   HX711_ADC
   Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
   Olav Kallhovd sept2017
   -------------------------------------------------------------------------------------
*/

/*
   Settling time (number of samples) and data filtering can be adjusted in the config.h file
   For calibration and storing the calibration value in eeprom, see example file "Calibration.ino"

   The update() function checks for new data and starts the next conversion. In order to acheive maximum effective
   sample rate, update() should be called at least as often as the HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS.
   If you have other time consuming code running (i.e. a graphical LCD), consider calling update() from an interrupt routine,
   see example file "Read_1x_load_cell_interrupt_driven.ino".

   This is an example sketch on how to use this library
*/
/* SevSeg Counter Example
 
 Copyright 2020 Dean Reading
 
 This example demonstrates a very simple use of the SevSeg library with a 4
 digit display. It displays a counter that counts up, showing deci-seconds.
 */
 
// Code ini jalan di timbangan Faaiz dengan 1 seven segment
#include <HX711_ADC.h>
#include <EEPROM.h>


#include "SevSeg.h"
SevSeg sevseg; //Instantiate a seven segment controller object


//pins:
const int HX711_dout = 3; //mcu > HX711 dout pin
const int HX711_sck = 6; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;
const int btn = A0;

void setup() {
  Serial.begin(57600);
  Serial.println();
  Serial.println("Starting...");
  pinMode(btn, INPUT_PULLUP);
  pinMode(A1, OUTPUT);

  // sevseg
  byte numDigits = 5;
  byte digitPins[] = {A2, A4, A5, 12,13};
  byte segmentPins[] = {2, 5, 4, 7, 8, 9, 10, 11};
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected
  
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(10);
// 

  LoadCell.begin();
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 81.47;
  
//  EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
    LoadCell.tare();
  
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity
//  digitalWrite(bzr, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);                       // wait for a second
//  digitalWrite(bzr, LOW);

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
   if (millis() > t + serialPrintInterval) {
      const int i = LoadCell.getData();

      Serial.print("Load_cell output val: ");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
//      int number = i;
      sevseg.setNumber(i,3);
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (digitalRead(btn) == LOW){
    digitalWrite(A1, LOW);
//    tone(A1, 20000);
    LoadCell.tareNoDelay();
    }else {digitalWrite(A1, HIGH);}

  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tare();
  }

//  // check if last tare operation is complete:
//  if (LoadCell.getTareStatus() == true) {
//    Serial.println("Tare complete");
//  }
sevseg.refreshDisplay();
}
