// SD card datalogger
// based on libraries and examples from Adafruit

// Copyright (C) 2015,  Reinier van der Lee
// www.vanderleevineyard.com

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;
const int chipSelect = 10;
File dataFile;

void setup()
{
Serial.begin(9600);
Wire.begin();
// rtc.begin();
// rtc.begin(DateTime(F(__DATE__)
// This line sets the RTC with an explicit date & time, for example to set
// January 21, 2014 at 3am you would call:
// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
//rtc.adjust(DateTime(2015, 4, 25, 8, 11, 0));

Serial.print("Initializing SD card...");
// make sure that the default chip select pin is set to
// output, even if you don't use it:

pinMode(10, OUTPUT);
// see if the card is present and can be initialized:
if (!SD.begin(chipSelect)) {
Serial.println("Card failed, or not present");
// don't do anything more:
while (1) ;
}
Serial.println("card initialized.");
// Open up the file we're going to log to!
dataFile = SD.open("datalog.txt", FILE_WRITE);
if (! dataFile) {
Serial.println("error opening datalog.txt");
// Wait forever since we cant write data
while (1) ;
  }
}

void loop()
{
int sensorValue = analogRead(A0);
int psi = (sensorValue-97)*.24445;
DateTime now = rtc.now();

Serial.print(now.month(), DEC);
Serial.print('/');
Serial.print(now.day(), DEC);
Serial.print(' ');
Serial.print(now.hour(), DEC);
Serial.print(':');
Serial.print(now.minute(), DEC);
Serial.print(':');
Serial.print(now.second(), DEC);
Serial.print("\t");
Serial.print (psi);
Serial.println();

dataFile.print(now.month(), DEC);
dataFile.print('/');
dataFile.print(now.day(), DEC);
dataFile.print(' ');
dataFile.print(now.hour(), DEC);
dataFile.print(':');
dataFile.print(now.minute(), DEC);
dataFile.print(':');
dataFile.print(now.second(), DEC);
dataFile.print("\t");
dataFile.print (psi);
dataFile.println();

dataFile.flush();

// Take 1 measurement every minute
delay(60000); 
}
