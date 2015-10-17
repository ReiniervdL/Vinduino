// "Vinduino-H" portable conductivity meter code V2.00
// Date October 17, 2015
// Copyright (C), 2015, Reinier van der Lee
// www.vanderleevineyard.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.


// include the library code only for LCD display version
#include <LiquidCrystal.h>
#include <math.h>

#define NUM_READS 10   // Number of sensor reads for filtering

typedef struct {        // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
  float moisture;
  long resistance;
} values;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// define Ohm character 
byte ohm[8] = {
  B00000,
  B01110,
  B10001,
  B10001,
  B10001,
  B01010,
  B11011,
  B00000
};

const long knownResistor = 4665;  // Constant value of known resistor in Ohms

int supplyVoltage;                // Measured supply voltage
int sensorVoltage;                // Measured sensor voltage

values valueOf[NUM_READS];        // Calculated  resistances to be averaged
long buffer[NUM_READS];
int index;

int i;                            // Simple index variable

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 

  // set up the LCD's number of columns and rows: 
  lcd.createChar(1, ohm);
  lcd.begin(8, 2);
  lcd.print ("Vinduino");
  lcd.setCursor(0, 1); 
  lcd.print("Salinity"); 

  // initialize the digital pin as an output.
  // Pin 6 is sense resistor voltage supply 1
  pinMode(6, OUTPUT);    

  // initialize the digital pin as an output.
  // Pin 7 is sense resistor voltage supply 2
  pinMode(7, OUTPUT);   

  delay(200);   
}

void loop() {

float moisture;

 // read sensor 1-3, filter, and calculate resistance value
 // Noise filter: median filter
 
measure(1,6,7,1);
long read1 = average();
measure(1,7,6,0);
long read2= average();
long sensor1 = (read1 + read2)/2;

delay (100);

    //valueOf[i].moisture = long(pow(valueOf[i].resistance/1950), 1.0/-0.87 ));
moisture =  float(pow (sensor1/1950.0 , 1.0/-0.87)); 

  Serial.print("sensor resistance = ");
  Serial.print(sensor1);
  Serial.print(",");
  Serial.print (moisture, 1);
  Serial.println ();
  
    // set the cursor to column 0, line 0
  lcd.setCursor(0, 0);
  //Clear the LCD
  lcd.print("        "); 
  lcd.setCursor(0, 1); 
  lcd.print("        "); 

  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
  //lcd.print("Conductivity: ");
  lcd.print (moisture, 1);
  lcd.print(" dS/m");

  // set the cursor to column 0, line 1
 lcd.setCursor(0,0);
  // lcd.print("Probe: " );
 lcd.print(sensor1);
 lcd.print(" ");
 lcd.write(1);

  // delay until next measurement (msec)
  delay(3000);   

}

void measure (int sensor, int phase_b, int phase_a, int analog_input)
{
 
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  for (i=0; i<NUM_READS; i++) {

    // Read 1 pair of voltage values
    digitalWrite(phase_a, HIGH);                 // set the voltage supply on
    delayMicroseconds(25);
    supplyVoltage = analogRead(analog_input);   // read the supply voltage
    delayMicroseconds(25);
    digitalWrite(phase_a, LOW);                  // set the voltage supply off 
    delay(1);
     
    digitalWrite(phase_b, HIGH);                 // set the voltage supply on
    delayMicroseconds(25);
    sensorVoltage = analogRead(analog_input);   // read the sensor voltage
    delayMicroseconds(25);
    digitalWrite(phase_b, LOW);                  // set the voltage supply off 

    // Calculate resistance
    // Tip: no need to transform 0-1023 voltage value to 0-5 range, due to following fraction
    long resistance = (knownResistor * (supplyVoltage - sensorVoltage ) / sensorVoltage) ;
    
    delay(1); 
    addReading(resistance);
    
   }
  }


// Averaging algorithm
void addReading(long resistance){
  buffer[index] = resistance;
  index++;
  if (index >= NUM_READS) index = 0;
}

long average(){
  long sum = 0;
  for (int i = 0; i < NUM_READS; i++){
    sum += buffer[i];
  }
  return (long)(sum / NUM_READS);
}
