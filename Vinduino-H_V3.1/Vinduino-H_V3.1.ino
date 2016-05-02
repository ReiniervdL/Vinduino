// "Vinduino-H" portable soil moisture sensor code V3.10
// Date November 16, 2014
// Copyright (C), 2015, Reinier van der Lee and Theodore Kaskalis
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

#define NUM_READS 11    // Number of sensor reads for filtering

typedef struct {        // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
  int moisture;
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

const long knownResistor = 4700;  // Constant value of known resistor in Ohms

int activeDigitalPin = 6;         // 6 or 7 interchangeably
int supplyVoltageAnalogPin;       // 6-ON: A0, 7-ON: A1
int sensorVoltageAnalogPin;       // 6-ON: A1, 7-ON: A0

int supplyVoltage;                // Measured supply voltage
int sensorVoltage;                // Measured sensor voltage

values valueOf[NUM_READS];        // Calculated moisture percentages and resistances to be sorted and filtered

int i;                            // Simple index variable

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 

  // set up the LCD's number of columns and rows: 
  lcd.createChar(1, ohm);
  lcd.begin(8, 2);
  lcd.print ("Vinduino");
  lcd.setCursor(0, 1); 
  lcd.print("  V3.2"); 

  // initialize the digital pin as an output.
  // Pin 6 is sense resistor voltage supply 1
  pinMode(6, OUTPUT);    

  // initialize the digital pin as an output.
  // Pin 7 is sense resistor voltage supply 2
  pinMode(7, OUTPUT);   

  delay(200);   
}

void loop() {

  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  for (i=0; i<NUM_READS; i++) {

    setupCurrentPath();      // Prepare the digital and analog pin values

    // Read 1 pair of voltage values
    digitalWrite(activeDigitalPin, HIGH);                 // set the voltage supply on
    delay(10);
    supplyVoltage = analogRead(supplyVoltageAnalogPin);   // read the supply voltage
    sensorVoltage = analogRead(sensorVoltageAnalogPin);   // read the sensor voltage
    digitalWrite(activeDigitalPin, LOW);                  // set the voltage supply off  
    delay(10); 

    // Calculate resistance and moisture percentage without overshooting 100
    // the 0.5 add-term is used to round to the nearest integer
    // Tip: no need to transform 0-1023 voltage value to 0-5 range, due to following fraction
    valueOf[i].resistance = long( float(knownResistor) * ( supplyVoltage - sensorVoltage ) / sensorVoltage + 0.5 );
    valueOf[i].moisture = min( int( pow( valueOf[i].resistance/31.65 , 1.0/-1.695 ) * 400 + 0.5 ) , 100 );
//  valueOf[i].moisture = min( int( pow( valueOf[i].resistance/331.55 , 1.0/-1.695 ) * 100 + 0.5 ) , 100 );

  }

  // end of multiple read loop

  // Sort the moisture-resistance vector according to moisture
  sortMoistures();

  // Print out median values
  //Serial.print("sensor resistance = ");
  Serial.print(valueOf[NUM_READS/2].resistance);
  Serial.print(",");
  Serial.print(valueOf[NUM_READS/2].moisture);
 Serial.println ();
  
    // set the cursor to column 0, line 0
  lcd.setCursor(0, 0);
  //Clear the LCD
  lcd.print("        "); 
  lcd.setCursor(0, 1); 
  lcd.print("        "); 

  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
  //lcd.print("Moisture: ");
  lcd.print(valueOf[NUM_READS/2].moisture);
  lcd.print(" %");

  // set the cursor to column 0, line 1
 lcd.setCursor(0,0);
  // lcd.print("Sensor: " );
 lcd.print(valueOf[NUM_READS/2].resistance);
 lcd.print(" Ohm");
 //lcd.write(1);

  // delay until next measurement (msec)
  delay(3000);   

}

void setupCurrentPath() {
  if ( activeDigitalPin == 6 ) {
    activeDigitalPin = 7;
    supplyVoltageAnalogPin = A1;
    sensorVoltageAnalogPin = A0;
  }
  else {
    activeDigitalPin = 6;
    supplyVoltageAnalogPin = A0;
    sensorVoltageAnalogPin = A1;
  }
}

// Selection sort algorithm
void sortMoistures() {
  int j;
  values temp;
  for(i=0; i<NUM_READS-1; i++)
    for(j=i+1; j<NUM_READS; j++)
      if ( valueOf[i].moisture > valueOf[j].moisture ) {
        temp = valueOf[i];
        valueOf[i] = valueOf[j];
        valueOf[j] = temp;
      }
}
