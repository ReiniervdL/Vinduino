
// Remote soil moisture 4-sensor node based on Vinduino R3
// Using LM-513H1 LoRa module, PCF8563 RTC,
// sensor 2,3: Watermark sensor @ 65F
// Date February, 2020
// Reinier van der Lee, www.vinduino.com

#include <Wire.h>                 // I2C communication to RTC
#include "LowPower.h"             // Needed for sleep mode, developed by "Rocketscream"
#include <math.h>                 // Conversion equation from resistance to %
#include <dht.h>                  // Rob Tillaart's library

//set up temp sensor
dht DHT;
#define DHT22_PIN 12



#define PCF8563address 0x51       // Format data for RTC
byte timercontrol, timervalue;    // for interrupt timer

//Use pin 2 for wake up from sleep mode interrupt
const int wakeUpPin = 2;

typedef struct {        // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
  int moisture;
  int resistance;
} values;

// Setting up format for reading 4 soil sensors
#define NUM_READS 11   // Number of sensor reads for filtering

const long knownResistor = 4750;  // Value of R9 and R10 in Ohms, = reference for sensor

unsigned long supplyVoltage;      // Measured supply voltage
unsigned long sensorVoltage;      // Measured sensor voltage
int zeroCalibration = 95;        // calibrate sensor resistace to zero when input is short circuited
                                  // basically this is compensating for the mux switch resistance

values valueOf[NUM_READS];        // Calculated  resistances to be averaged
long buffer[NUM_READS];
int index2;
int i;                            // Simple index variable
int j=0;                          // Simple index variable

int tension1;
int tension2;
int tension3;
int tension4;

int Vsys;
int temp;
int humidity;

boolean joinAccept = false; 

String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete

int n =10;


void setup() {
  
  // initialize serial communications at 57600 bps:
  Serial.begin(57600);            // initialize LoRa module communications
  Wire.begin();                   // initialize I2C communications
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
  // Configure wake up pin as input.
  // This will consume few uA of current.
  pinMode(wakeUpPin, INPUT);  

  // setting up the LM-210 LoRa module control pins:
  pinMode(3, OUTPUT); // module P1 pin
  pinMode(4, OUTPUT); // module P2 pin

  // P1=0 and P2=0 : module active mode (Mode 1)
  // P1=0 and P2=1 : module wake-up mode (Mode 2)
  // P1=1 and P2=0 : module power saving mode (Mode 3)
  // P1=1 and P2=1 : module set-up mode (Mode 4)
  
// setting up the sensor interface
  // initialize digital pins D5, D6 as an high impedance input.
  // Pin 5,6 are for driving the soil moisture sensor
  pinMode(5, INPUT);    
  pinMode(6, INPUT); 
  // Pin 7 is for enabling Mux switches
  pinMode(7, OUTPUT); 
  // Pin 8,9 are for selecting sensor 1-4
  pinMode(8, OUTPUT);  // Mux input A
  pinMode(9, OUTPUT);  // Mux input B 
 
  // Pin 13 is used as WiFi radio and aux power enable, as well as activity indicator
  pinMode(13, OUTPUT);  


  digitalWrite(3, LOW); // module P1 pin
  digitalWrite(4, LOW); // module P2 pin

   Serial.println (255, HEX); // wake-up LM-130 from sleep mode

  for (i = 0; i < n; i++) 
  {serialEvent();
  Serial.println (i);
  if (joinAccept == 1) i=n;
  delay (1000);
  }
  Serial.print ("joinAccept = ");
  Serial.println (joinAccept);
  if (joinAccept == 0) { loraconfig(); }
  if (joinAccept) { Serial.println ("no lora config needed");}

  soilsensors(); 
}


void loop() 
{
 digitalWrite(13, LOW); //  LED off;
 
  for (i = 0; i < n; i++) 
  {serialEvent();
  Serial.println (i);
  delay (300);
  }

   Serial.println("AAT1 SLEEP"); delay (200);

   settimerPCF8563();      // set the timer delay and alarm

   // Allow wake up pin to trigger interrupt on low.
   attachInterrupt(0, wakeUp, LOW);
    
   // Enter power down state with ADC and BOD module disabled.
   // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);     

   // Disable external pin interrupt on wake up pin.
  detachInterrupt(0); 
 Serial.println(255,HEX);

  delay (100);

  soilsensors(); 

    

delay (1000);

}

void soilsensors(){
  
// Select sensor 1, and enable MUX
  digitalWrite(8, LOW); 
  digitalWrite(9, LOW); 
  digitalWrite(7, LOW); 
  measureSensor();
 // double tension1 = average();  //use this line to display uncalibrated sensor resistance
 unsigned long read1 = average();
  if (read1 >35000){tension1=255;}
  if (read1 >1000 && read1 <=35000){tension1=-0.000000000003*pow(read1,3)+0.0000002*pow(read1,2)+(0.0037*read1)+6.73;}
  if (read1<=1000){tension1=(read1-400)/60;}
  if (read1<400){tension1=0;}

// Select sensor 2, and enable MUX
  digitalWrite(8, LOW); 
  digitalWrite(9, HIGH); 
  digitalWrite(7, LOW); 
  measureSensor();
  //double tension2 = average();  //use this line to display uncalibrated sensor resistance
  unsigned long read2 = average();
  if (read2 >35000){tension2=255;}
  if (read2 >1000 && read2 <=35000){tension2=-0.000000000003*pow(read2,3)+0.0000002*pow(read2,2)+(0.0037*read2)+6.73;}
  if (read2<=1000){tension2=(read2-400)/60;}
  if (read2<400){tension2=0;}

    // Select sensor 3, and enable MUX
  digitalWrite(8, HIGH); 
  digitalWrite(9, LOW); 
  digitalWrite(7, LOW); 
  measureSensor();
  //double tension3 = average();  //use this line to display uncalibrated sensor resistance
  unsigned long read3 = average();
  if (read3 >35000){tension3=255;}
  if (read3 >1000 && read3 <=35000){tension3=-0.000000000003*pow(read3,3)+0.0000002*pow(read3,2)+(0.0037*read3)+6.73;}
  if (read3<=1000){tension3=(read3-400)/60;}
  if (read3<400){tension3=0;}

  // Select sensor 4, and enable MUX
  digitalWrite(8, HIGH); 
  digitalWrite(9, HIGH); 
  digitalWrite(7, LOW); 
  measureSensor();
  //double tension4 = average();  //use this line to display uncalibrated sensor resistance
  unsigned long read4 = average();
  if (read4 >35000){tension4=255;}
  if (read4 >1000 && read4 <=35000){tension4=-0.000000000003*pow(read4,3)+0.0000002*pow(read4,2)+(0.0037*read4)+6.73;}
  if (read4<=1000){tension4=(read4-400)/60;}
  if (read4<400){tension4=0;}

  int Vsys = analogRead(3)*0.00647*50;   // read the battery voltage


 //digitalWrite(4, LOW); // module P2 pin
 //digitalWrite(3, LOW); // module P1 pin
 digitalWrite(13, HIGH); // LED ON
   delay (50);

  Vsys = analogRead(3)*0.00647*50;   // read the battery voltage
  delay (10);

  //Read DHT-22 sensor
  int chk = DHT.read22(DHT22_PIN);      // temperature sensor data

  temp = DHT.temperature;
  humidity = DHT.humidity;
  temp = temp +100 ; //use this line when sensor installed
  //temp = 100; humidity = 0; //use this line when sensor NOT installed

  TxData();
}

  void TxData(){
  Serial.print("AAT2 Tx=2,uncnf,");
  
  if (Vsys<16) //add leading zero
  {Serial.print(0, HEX);}
  if (Vsys>255) //handle overflow
  {Vsys = 255;}
  Serial.print(Vsys, HEX); 
  
  if (temp<16) //add leading zero fix
  {Serial.print(0, HEX);}
  if (temp>255) //handle overflow
  {temp = 255;}
  Serial.print(temp, HEX); 

  if (humidity<16) //add leading zero fix
  {Serial.print(0, HEX);}
  if (humidity>255) //handle overflow
  {humidity = 255;}
  Serial.print(humidity, HEX);

  
  if (tension1<16) //add leading zero fix
  {Serial.print(0, HEX);}
  if (tension1>255) //handle overflow
  {tension1 = 255;}
  Serial.print(tension1,HEX);  

  
  if (tension2<16) //add leading zero fix
  {Serial.print(0, HEX);}
  if (tension2>255) //handle overflow
  {tension2 = 255;}
  Serial.print(tension2,HEX); 

  
  if (tension3<16) //add leading zero fix
  {Serial.print(0, HEX);}
  if (tension3>255) //handle overflow
  {tension3 = 255;}
  Serial.print(tension3,HEX); 

  
  if (tension4<16) //add leading zero fix
  {Serial.print(0, HEX);}
  if (tension4>255) //handle overflow
  {tension4 = 255;}
  Serial.println(tension4,HEX);

  delay (500);
  

  digitalWrite(13, LOW); //  LED off

  return;
}

void measureSensor()
{

  for (i=0; i<NUM_READS; i++) 
      {
  
    pinMode(5, OUTPUT); 
    digitalWrite(5, LOW);  
    digitalWrite(5, HIGH); 
    delayMicroseconds(250);
    sensorVoltage = analogRead(0);   // read the sensor voltage
    supplyVoltage = analogRead(1);   // read the supply voltage
    digitalWrite(5, LOW); 
    pinMode(5, INPUT);
    long resistance = (knownResistor * (supplyVoltage - sensorVoltage ) / sensorVoltage)-zeroCalibration ;
    
   addReading(resistance);
   delayMicroseconds(250);

    pinMode(6, OUTPUT); 
    digitalWrite(6, LOW);  
    digitalWrite(6, HIGH); 
    delayMicroseconds(250);
    sensorVoltage = analogRead(1);   // read the sensor voltage
    supplyVoltage = analogRead(0);   // read the supply voltage
    digitalWrite(6, LOW); 
    pinMode(6, INPUT);
    resistance = (knownResistor * (supplyVoltage - sensorVoltage ) / sensorVoltage)-zeroCalibration ;
    
   addReading(resistance);
   delay(100);

      } 
}


// Averaging algorithm
void addReading(long resistance){
  buffer[index2] = resistance;
  index2++;
  if (index2 >= NUM_READS) index2 = 0;
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


void checkPCF8563alarm()    // checks if the alarm has been activated
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
  {digitalWrite(13, HIGH);   // turn pin 13 and Radio module on (HIGH is the voltage level)
   delay(50);
   PCF8563alarmOff(); }      // turn off the alarm
  
}

void settimerPCF8563()
// this sets the timer from the PCF8563
{
Wire.beginTransmission(PCF8563address);
Wire.write(0x0E); // move pointer to timer control address

Wire.write(0x83); // sends 0x83 Hex 010000011 (binary)
// (0x82 = enable timer,timer clock set to 1 second)
// (0x83 = enable timer,timer clock set to 1 minute)
Wire.endTransmission();
 
Wire.beginTransmission(PCF8563address);
Wire.write(0x0F); // move pointer to timer value address
Wire.write(0x0F); // sends delay time value (0F =15 minutes, 1E = 0.5 hour, 3C = 1 hour)
Wire.endTransmission();

// optional - turns on INT_ pin when timer activated
// will turn off once you run void PCF8563alarmOff()
Wire.beginTransmission(PCF8563address);
Wire.write(0x01); // select control status_register
Wire.write(B00000001); // set Timer INT enable bit
Wire.endTransmission();
}

void loraconfig()
// this initializes the LM-513 LoRa module to TTN LoRaWAN OTAA
{
  
  //Serial.println("AAT1 Reset"); delay (5000); //assume successful join to TTN network
  // set join mode = OTAA, ADR=0, DutyCycle=on)
  Serial.println("AAT2 JoinMode=1"); delay (50);
  Serial.println("AAT2 ADR=0"); delay (50);
  Serial.println("AAT2 DutyCycle=1"); delay (50);

  
  //Set 8 channel TX frequencies for TTN USA, for EU version do not set frequencies
  Serial.println("AAT2 Tx_Channel=0,903900000,30,1,0"); delay (50);
  Serial.println("AAT2 Tx_Channel=1,904100000,30,1,0"); delay (50);
  Serial.println("AAT2 Tx_Channel=2,904300000,30,1,0"); delay (50);
  Serial.println("AAT2 Tx_Channel=3,904500000,30,1,0"); delay (50);
  Serial.println("AAT2 Tx_Channel=4,904700000,30,1,0"); delay (50);
  Serial.println("AAT2 Tx_Channel=5,904900000,30,1,0"); delay (50); 
  Serial.println("AAT2 Tx_Channel=6,905100000,30,1,0"); delay (50);
  Serial.println("AAT2 Tx_Channel=7,905300000,30,1,0"); delay (50);


  // set AppEui & AppKey
  Serial.println("AAT2 AppEui=70B3D57ED002E2EE"); delay (50); // get the AppEUI from The Things Network console

  Serial.println("AAT2 AppKey=7C3AF9DC87E6A361A17BD1F555CF8CEC"); delay (50); // get the Appkey from The Things Network console

  
  Serial.println("AAT1 Save"); delay (5000);

  // save and reset;
  Serial.println("AAT1 Reset"); delay (1000); //assume successful join to TTN network

}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      SerialParse ();
    }
  }
}

void SerialParse () {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString);
    if(inputString.indexOf("JOIN_NOT_ACCEPT") >= 0) {Serial.println("JOIN_NOT_ACCEPT action");signal_Led(1);};
    if(inputString.indexOf("JOIN_ACCEPT") >= 0) {Serial.println("JOIN_ACCEPT action"); joinAccept = true; signal_Led(2);};
    if(inputString.indexOf("Tx_ok") >= 0) {Serial.println("Tx_ok action"); signal_Led(3); i=10;};
    if(inputString.indexOf("Tx_no_free_ch") >= 0) {Serial.println("Tx_no_free_ch action"); signal_Led(4);};
    if(inputString.indexOf("Tx_not_joined") >= 0) {Serial.println("Tx_not_joined action"); signal_Led(5);};
    if(inputString.indexOf("Tx_noACK") >= 0) {Serial.println("Tx_noACK action"); signal_Led(6);};
    if(inputString.indexOf("invalid_param") >= 0) {Serial.println("invalid_param action"); signal_Led(7);};
    
    //INPUT DATA WAS PARSED, SO WE GET BACK TO INITIAL STATE.
    inputString = "";
    stringComplete = false;
}}

void signal_Led(int n) {

  for (i = 0; i < n; i++) 
  {
    digitalWrite(13, HIGH);
    delay (500);
    digitalWrite(13, LOW); //  LED off;
    delay (500);
  }
}
