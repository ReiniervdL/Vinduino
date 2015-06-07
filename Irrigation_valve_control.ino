// Irrigation valve control example
// Turns on a Hunter DC latching solenoid on for one second, then off for one second, repeatedly.
// Copyright (C) 2015, Reinier van der Lee
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
 
int a = 13;
int b = 12;

void setup() {                
  // initialize the digital pin as an output.
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);    
}

void loop() {
  digitalWrite(a, HIGH);   // turn the valve on (HIGH is the voltage level)
  delay(10);               // wait 10 milliseconds
  digitalWrite(a, LOW);    // end drive pulse by making the voltage LOW
   delay(1000);               // wait for a second
 digitalWrite(b, HIGH);   // turn the valve off (HIGH is the voltage level)
  delay(10);               // wait 10 milliseconds
  digitalWrite(b, LOW);    // end drive pulse by making the voltage LOW
  delay(1000);
}
