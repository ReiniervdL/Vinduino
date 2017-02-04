// Remote soil moisture 4-sensor node based on Vinduino R3
// Using ESP8266 WiFi and PCF8563 RTC
// Added galvanic voltage bias compensation
// Date March 28, 2016
// Reinier van der Lee, www.vanderleevineyard.com

#include "LowPower.h"             // Needed for sleep mode
#include <Wire.h>                 // I2C communication to RTC
#include <math.h>                 // Conversion equation from resistance to %
#include <OneWire.h>
#include <DallasTemperature.h>

// Wifi connection data
#define SSID "your WiFi SSID here"
#define PASS "your WiFi password here"
#define IP "184.106.153.149" // thingspeak.com
String GET = "GET /update?key=ThingSpeak-key &1="; ? //enter you ThingSpeak key here

//set up temp sensor
#define ONE_WIRE_BUS 12 
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

#define PCF8563address 0x51       // Format data for RTC
byte timercontrol, timervalue;    // for interrupt timer

//Use pin 2 for wake up from sleep mode interrupt
const int wakeUpPin = 2;

typedef struct {                  // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
 // int moisture;
 long resistance;
} values;

// Setting up format for reading 4 soil sensors
#define NUM_READS 10    // Number of sensor reads for filtering

const long knownResistor = 4700;  // Value of R9 and R10 in Ohms, = reference value for soil sensor

int supplyVoltage;                // Measured supply voltage
int sensorVoltage;                // Measured sensor voltage
int zeroCalibration = 265;        // calibrate sensor resistace to zero when input is short circuited
                                  // basically this is compensating for the mux switch resistance

values valueOf[NUM_READS];        // Calculated  resistances to be averaged
long buffer[NUM_READS];
int index2;
int i;                            // Simple index variable



void setup() {
  
  Serial.begin(115200);            // initialize serial communications at 115200 bps:    
  Wire.begin();                   // initialize I2C communications

  pinMode(wakeUpPin, INPUT);      // Configure wake up pin as input.
  
  // setting up the soil sensor interface
  // initialize digital pins D5, D6 as an high impedance input.
  // Pin 5,6 are for driving the soil moisture sensor
  pinMode(5, INPUT);    
  pinMode(6, INPUT); 
  // Pin 7 is for enabling Mux switches
  pinMode(7, OUTPUT); 
  // Pin 8,9 are for selecting sensor 1-4
  pinMode(8, OUTPUT);  // Mux input A
  pinMode(9, OUTPUT);  // Mux input B 

   // set the timer delay and alarm
  settimerPCF8563();
 
  pinMode(13, OUTPUT);           // Pin 13 is used as WiFi radio and aux power enable
  digitalWrite(13, HIGH); 
  delay (1000);
  Serial.println("AT+RST");   
  delay (1000); 
  Serial.println("AT+CWMODE=1");
  delay(1000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  delay (5000); 
  readsensors();
  digitalWrite(13, LOW);   
}


void loop() 
{

  attachInterrupt(0, wakeUp, LOW); // Allow wake up pin to trigger interrupt on low.
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  
  detachInterrupt(0);           // Disable external pin interrupt on wake up pin.
  checkPCF8563alarm();          // If alarm, make pin 13 (radio enable) high
  
  delay(500);  
 
  readsensors();

  delay (1000);

}



void readsensors(){
  
// Select sensor 1, and enable MUX
  digitalWrite(8, LOW); 
  digitalWrite(9, LOW); 
  digitalWrite(7, LOW); 
  measureSensor();
  int read1 = average();

// Select sensor 2, and enable MUX
  digitalWrite(8, LOW); 
  digitalWrite(9, HIGH); 
  digitalWrite(7, LOW); 
  measureSensor();
  int read2 = average();

    // Select sensor 3, and enable MUX
  digitalWrite(8, HIGH); 
  digitalWrite(9, LOW); 
  digitalWrite(7, LOW); 
  measureSensor();
  int read3 = average();

  // Select sensor 4, and enable MUX
  digitalWrite(8, HIGH); 
  digitalWrite(9, HIGH); 
  digitalWrite(7, LOW); 
  measureSensor();
  int read4 = average();

  float Vsys = analogRead(3)*0.00647;   // read the battery voltage

  //Read DS18B20 sensor
  float temp;
  float humidity;
  sensors.requestTemperatures();      // Send the command to get temperatures

  temp = (sensors.getTempFByIndex(0)); // Degrees F
  // temp = (sensors.getTempCByIndex(0)); // Degrees C
  humidity = 0;


  // convert data to string
  char buf[16];   String strsensor1 = dtostrf(read1, 4, 0, buf);
  char sn2[16];   String strsensor2 = dtostrf(read2, 4, 0, buf);
  char sn3[16];   String strsensor3 = dtostrf(read3, 4, 0, buf);
  char sn4[16];   String strsensor4 = dtostrf(read4, 4, 0, buf);
  char Vbatt[16]; String strVcc = dtostrf(Vsys, 4, 2, buf);
  char tem[16];   String strTemp = dtostrf(temp, 2, 1, buf);
  char hum[16];   String strHumidity = dtostrf(humidity, 2, 0, buf);
  
  updateData( strsensor1, strsensor2, strsensor3, strsensor4, strVcc, strTemp, strHumidity );
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
Wire.write(0x0F); // sends delay time value (0F =15 minutes, 3C = 1 hour)
Wire.endTransmission();

// optional - turns on INT_ pin when timer activated
// will turn off once you run void PCF8563alarmOff()
Wire.beginTransmission(PCF8563address);
Wire.write(0x01); // select control status_register
Wire.write(B00000001); // set Timer INT enable bit
Wire.endTransmission();
} 

void updateData(String strsensor1, String strsensor2, String strsensor3, String strsensor4, String strVcc,String strTemp, String strHumidity){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  delay(1000);
  //if(Serial.find("Error")){
  //  return;
  
  cmd = GET; cmd += strsensor1; cmd += "&2="; cmd += strsensor2; cmd += "&3=";cmd += strsensor3; cmd += "&4="; cmd += strsensor4; 
  cmd += "&5="; cmd += strVcc; cmd += "&6="; cmd += strTemp; cmd += "&7="; cmd += strHumidity; cmd += "\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  delay (5000);
  Serial.print(cmd);
  delay(5000);     
  digitalWrite(13, LOW);  // turn off the radio module

}


