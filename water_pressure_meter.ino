// "Vinduino" portable PSI sensor V1.00
// Date March 7, 2015
// Copyright (C), 2015, Reinier van der Lee
// www.vanderleevineyard.com

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// include the library code only for LCD display version
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(8, 2);
  lcd.print ("Pressure");  
}

void loop() {

float sensorVoltage = analogRead(0);   // read the sensor voltage
int psi = ((sensorVoltage-95)/204)*50;
lcd.setCursor (0,1);
lcd.print (psi);
lcd.print (" PSI");
Serial.println (psi);

delay (1000);
}
