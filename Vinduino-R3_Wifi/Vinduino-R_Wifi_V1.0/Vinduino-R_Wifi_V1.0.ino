
// Remote soil moisture 3-sensor node based on Vinduino-H V3
// Using ESP2866 Wifi module and PCF8563 RTC
// Using latest DHT libaray, allowing use of DHT11 or DHT 22
// Added galvanic voltage bias compensation
// Date May 24, 2015
// Copyright (C), 2015, Reinier van der Lee, 
// www.vanderleevineyard.com

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "LowPower.h"   // Needed for sleep mode
#include <Wire.h>       // I2C communication to RTC
#include <math.h>       // Conversion equation from resistance to %
#include <dht.h>

// Wifi connection data
#define SSID "insert wifi network name here"
#define PASS "insert your network password here"
#define IP "184.106.153.149" // thingspeak.com
String GET = "GET /update?key="VSBQNUSZ0RCICFRK" &1=";

//set up temp sensor
dht DHT;
#define DHT11_PIN 12

#define PCF8563address 0x51

// Use pin 2 as wake up pin
const int wakeUpPin = 2;

// set up variables for battery voltage measurement
int analoginput = 0; // our analog pin
int analogamount = 0; // stores incoming value
float voltage =0; // used to store voltage value

// Format data for RTC
//byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
//byte alarmMinute, alarmHour, alarmDay, alarmDayOfWeek;
byte timercontrol, timervalue; //  for interrupt timer
//String days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//byte bcdToDec(byte value)
//{
//  return ((value / 16) * 10 + value % 16);
//}

//byte decToBcd(byte value){
//  return (value / 10 * 16 + value % 10);
//}

// Setting up format for reading 3 soil sensors
#define NUM_READS 11    // Number of sensor reads for filtering

typedef struct {        // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
  int moisture;
  long resistance;
} values;


const long knownResistor = 4700;  // Constant value of known resistor in Ohms

int supplyVoltage;                // Measured supply voltage
int sensorVoltage;                // Measured sensor voltage

values valueOf[NUM_READS];        // Calculated  resistances to be averaged
long buffer[NUM_READS];
int index;

int i;                            // Simple index variable


long readVcc() { 
long result;                       // Read 1.1V reference against AVcc 
ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); 
delay(2); // Wait for Vref to settle 
ADCSRA |= _BV(ADSC); // Convert 
while (bit_is_set(ADCSRA,ADSC)); 
result = ADCL; 
result |= ADCH<<8; 
result = (1125300L / result); // Back-calculate AVcc in mV 
return result; 
}


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
  
    // initialize I2C communications
  Wire.begin();
  
  // Configure wake up pin as input.
  // This will consume few uA of current.
  pinMode(wakeUpPin, INPUT);   

  // initialize the digital pins as an output.
  // Pin 6,7 is for sensor 1
  pinMode(6, OUTPUT);    
  pinMode(7, OUTPUT); 
  // Pin 8,9 is for sensor 2
  pinMode(8, OUTPUT);    
  pinMode(9, OUTPUT);  
  // Pin 10,11 is for sensor 3
  pinMode(10, OUTPUT);    
  pinMode(11, OUTPUT);
  
  // Pin 13 is radio enable
 pinMode(13, OUTPUT);      
  
  // set the timer delay and alarm
  settimerPCF8563();
  
}

void loop() {
  
   
 // Allow wake up pin to trigger interrupt on low.
 attachInterrupt(0, wakeUp, LOW);
    
   // Enter power down state with ADC and BOD module disabled.
   // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
   // Disable external pin interrupt on wake up pin.
  detachInterrupt(0); 
   

   // If alarm, make pin 13 (radio enable) high
  checkPCF8563alarm();
   delay(500);
   
   //Restart radio module
      Serial.println("AT+RST");
  delay(5000);
  if(Serial.find("ready")){
    connectWiFi();
  }
 // read sensor 1-3, filter, and calculate resistance value
 // Noise filter: median filter
 
measure(1,6,7,1);
long read1 = average();
measure(1,7,6,0);
long read2= average();
long sensor1 = (read1 + read2)/2;

delay (100);


measure(2,8,9,2);
long read3 = average();
measure(2,9,8,6);
long read4= average();
long sensor2 = (read3 + read4)/2;

delay (100);

measure(3,10,11,3);
long read5 = average();
measure(3,11,10,7);
long read6= average();
long sensor3 = (read5 + read6)/2;


 // measure and print battery voltage
 float Vcc = (readVcc());
 Vcc =Vcc/1000;
 //Serial.print("Battery voltage: ");
 //Serial.print( Vcc,2);
 //Serial.println (" V");
 //Serial.println();


  int temp;
  int humidity;
  int chk = DHT.read11(DHT11_PIN);    // READ DATA

    temp = DHT.temperature,1;
    humidity = DHT.humidity,1;
    
   // convert data to string
  char buf[16];
  String strTemp = dtostrf(temp, 4, 1, buf);
  
  char hum[16];
  String strHumidity = dtostrf(humidity, 4, 1, buf);
  
  char sn1[16];
  String strsensor1 = dtostrf(sensor1, 4, 1, buf);
  
  char sn2[16];
  String strsensor2 = dtostrf(sensor2, 4, 1, buf);
  
  char sn3[16];
  String strsensor3 = dtostrf(sensor3, 4, 1, buf);
  
  char Vbatt[16];
  String strVcc = dtostrf(Vcc, 4, 1, buf);
  
  updateTemp(strTemp, strHumidity, strsensor1, strsensor2, strsensor3, strVcc );
  

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
    // the 0.5 add-term is used to round to the nearest integer
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


void wakeUp()
{
    // Just a handler for the pin interrupt.
}

// RTC routines below

void PCF8563alarmOff()
// turns off alarm enable bits and wipes alarm registers. 
{
  byte test;
  // first retrieve the value of control register 2
  Wire.beginTransmission(PCF8563address);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.requestFrom(PCF8563address, 1);
  test = Wire.read();

  // set bit 2 "alarm flag" to 0
  test = test - B00000100;

  // now write new control register 2  
  Wire.beginTransmission(PCF8563address);
  Wire.write(0x01);
  Wire.write(test);
  Wire.endTransmission();
}


void checkPCF8563alarm()
// checks if the alarm has been activated
{
  byte test;
  // get the contents from control register #2 and place in byte test;
  Wire.beginTransmission(PCF8563address);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.requestFrom(PCF8563address, 1);
  test = Wire.read();
  test = test & B00000100 ;// read the timer alarm flag bit
  if (test == B00000100)// alarm on?
  {
    digitalWrite(13, HIGH);   // turn pin 13 and Radio module on (HIGH is the voltage level)
    
    delay(50);
    
    // alarm! Do something to tell the user
    //Serial.println();
    //Serial.println("** Timer alarm **");
   //delay(200);

    // turn off the alarm
    PCF8563alarmOff();
    
    }
}

void settimerPCF8563()
// this sets the timer from the PCF8563
{
Wire.beginTransmission(PCF8563address);
Wire.write(0x0E); // move pointer to timer control address
// enable timer,timer clock set to 1 Hz
Wire.write(0x82); // sends 0x82 Hex 010000010 (binary)
Wire.endTransmission();
 
Wire.beginTransmission(PCF8563address);
Wire.write(0x0F); // move pointer to timer value address
Wire.write(0xFF); // sends delay time value (0F =16 seconds, 3C = 60 seconds)
Wire.endTransmission();

// optional - turns on INT_ pin when timer activated
// will turn off once you run void PCF8563alarmOff()
Wire.beginTransmission(PCF8563address);
Wire.write(0x01); // select control status_register
Wire.write(B00000001); // set Timer INT enable bit
Wire.endTransmission();
} 

void updateTemp(String strTemp, String strHumidity, String strsensor1, String strsensor2, String strsensor3, String strVcc ){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  delay(1000);
  if(Serial.find("Error")){
    return;
  }
  cmd = GET;
  cmd += strTemp;
  cmd += "&2=";
  cmd += strHumidity; 
  cmd += "&3=";
  cmd += strsensor1; 
  cmd += "&4=";
  cmd += strsensor2; 
  cmd += "&5=";
  cmd += strsensor3;
  cmd += "&6=";
  cmd += strVcc;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  if(Serial.find(">")){
    Serial.print(cmd);
  }else{
    Serial.println("AT+CIPCLOSE");
  }
  
  delay(1000);     
  digitalWrite(13, LOW);  // turn off the radio module
}

 
boolean connectWiFi(){
  Serial.println("AT+CWMODE=1");
  delay(1000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  delay(1000);
  if(Serial.find("OK")){
    return true;
  }else{
    return false;
  }
}
