// Simple Iambic Keyer v1.00
// by Ernest PA3HCM
// Reinier AI6CT added rotary encoder and OLED WPM indication

#include <Encoder.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#define I2C_ADDRESS 0x3C\
//#define RST_PIN -1
SSD1306AsciiAvrI2c oled;

Encoder myEnc(2, 3);

#define P_DOT    5   // Connects to the dot lever of the paddle
#define P_DASH   6   // Connects to the dash lever of the paddle
#define P_AUDIO 12   // Audio output
#define P_CW    13   // Output of the keyer, connect to your radio
#define P_SPEED A0   // Attached to center pin of potmeter, allows you
                     // to set the keying speed.

int oldPosition  = -999;
float speed;

// Initializing the Arduino
void setup()
{
  Serial.begin(9600);
  
  pinMode(P_DOT, INPUT_PULLUP);
  pinMode(P_DASH, INPUT_PULLUP); 
  pinMode(P_AUDIO, OUTPUT);
  pinMode(P_CW, OUTPUT);
  digitalWrite(P_CW, LOW);      // Start with key up

  oled.begin(&Adafruit128x32, I2C_ADDRESS);

  oled.setFont(Adafruit5x7);

}
  
// Main routine
void loop()
{
    uint32_t m = micros(); 
   
   speed = myEnc.read()+60;
   if (speed != oldPosition) {
    oldPosition = speed;
   // Serial.print("WPM: ");
   // Serial.println(1200/speed,1);

    oled.clear();
    

  // first row
  oled.setCursor (0,0);
  oled.set1X();
  oled.println("AI6CT morse keyer");

  // second row
  //oled.setCursor (0,1);
  //oled.set1X();
  //oled.println(" ");

      // third row
  oled.setCursor (0,2);
  oled.set2X();
  oled.print(1200/speed,1);
  oled.print(" WPM    ");
  oled.print(micros() - m);

  
   }
  
 // speed = analogRead(P_SPEED)/2; // Read the keying speed from potmeter
  if(!digitalRead(P_DOT))        // If the dot lever is presssed..
  {
    tone(P_AUDIO,800);
    keyAndBeep(speed);           // ... send a dot at the given speed
    noTone(P_AUDIO);
    delay(speed);                //     and wait before sending next
  }
  if(!digitalRead(P_DASH))       // If the dash lever is pressed...
  {
    tone(P_AUDIO,800);
    keyAndBeep(speed*3);         // ... send a dash at the given speed
    noTone(P_AUDIO);
    delay(speed);                //     and wait before sending next
  }
}

// Key the transmitter and sound a beep
void keyAndBeep(int speed)
{
  digitalWrite(P_CW, HIGH);            // Key down
  //tone (P_AUDIO,800);
  delay (speed);
 // noTone(P_AUDIO);
  digitalWrite(P_CW, LOW);             // Key up
}
