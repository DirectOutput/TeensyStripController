/********************************************************************************************************
 ** TeensyStrip Controller
 ** ----------------------
 ** 
 ** This Sketch turns a Teensy 3.1, 3.2 or later into a controller for WS2811/WS2812 based led strips.
 ** This strip controller was originally designed for use with the Direct Output Framework, but since 
 ** the communication protocol is simple and communication uses the virtual com port of the Teensy
 ** it should be easy to controll the strips from other applications as well.
 ** 
 ** The most important part of the code is a slightly hacked version (a extra method to set the length of the 
 ** strips dynamiccaly has been added) of Paul Stoffregens excellent OctoWS2811 LED Library.  
 ** For more information on the lib check out: 
 ** https://github.com/PaulStoffregen/OctoWS2811
 ** http://www.pjrc.com/teensy/td_libs_OctoWS2811.html 
 ** 
 *********************************************************************************************************/  
 /*  
    License:
    --------   
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  Required Connections
  --------------------
    pin 2:  LED Strip #1    OctoWS2811 drives 8 LED Strips.
    pin 14: LED strip #2    All 8 are the same length.
    pin 7:  LED strip #3
    pin 8:  LED strip #4    A 100 ohm resistor should used
    pin 6:  LED strip #5    between each Teensy pin and the
    pin 20: LED strip #6    wire to the LED strip, to minimize
    pin 21: LED strip #7    high frequency ringining & noise.
    pin 5:  LED strip #8
    pin 15 & 16 - Connect together, but do not use
    pin 4 - Do not use
    pin 3 - Do not use as PWM.  Normal use is ok.
     
*/
#include "arduino.h"
#include "OctoWS2811Ext.h" //A slightly hacked version of the OctoWS2811 lib which allows for dynamic setting of the number of leds is used.

//Definiton of Major and Minor part of the firmware version. This value can be received using the V command.
//If something is changed in the code the number should be increased.
#define FirmwareVersionMajor 1
#define FirmwareVersionMinor 4

//For Teensy 4.0 you can define the nuber of ouput Pins - 8 should be good (for 3.1/3.2 this is only used for calculation and schould not changed)
const int numPins = 8;
//for tesnsy 4.0 you can change the standard port (schould not be done) - Not used for 3.1/3.2
byte pinList[numPins] = {2, 14, 7, 8, 6, 20, 21, 5};

//Defines the max number of leds which is allowed per ledstrip.
//This number is fine for Teensy 3.2, 3.1. For newer Teensy versions (they dont exists yet) it might be possible to increase this number.
#define MaxLedsPerStrip 1100

//Defines the Pinnumber to which the built in led of the Teensy is connected.
//For Teensy 3.2, 3.1 this is pin 13, if you use a newer Teensy version (not available at the time of writing) you might need to change this number.
#define LedPin 13

// Defines the Pinnumber for the test button which is low when pressed
#define TestPin 17

//Memory buffers for the OctoWS2811 lib
DMAMEM int displayMemory[MaxLedsPerStrip * numPins * 3 / 4];
int drawingMemory[MaxLedsPerStrip * numPins * 3 / 4];

//Variable used to control the blinking and flickering of the led of the Teensy
elapsedMillis BlinkTimer;
int BlinkMode;
elapsedMillis BlinkModeTimeoutTimer;

//Config definition for the OctoWS2811 lib
const int config = WS2811_RGB | WS2811_800kHz; //Dont change the color order (even if your strip are GRB). DOF takes care of this issue (see config of ledstrip toy)

#if defined(__IMXRT1062__)
  OctoWS2811Ext leds(MaxLedsPerStrip, displayMemory, drawingMemory, config, numPins, pinList);
#else
  OctoWS2811Ext leds(MaxLedsPerStrip, displayMemory, drawingMemory, config);
#endif

word configuredStripLength=448;

//Setup of the system. Is called once on startup.
void setup() {
  Serial.begin(9600); // This has no effect. USB bitrate (12MB) will be used anyway.

  //Initialize the lib for the leds
  leds.setStripLength(configuredStripLength);
  leds.begin();
  leds.show();

  //Initialize the led pin
  pinMode(LedPin,OUTPUT);

  //Initialize and find value of the test pin
  pinMode(TestPin,INPUT_PULLUP);  
  if (! digitalRead(TestPin)) { 
    // run test if button is grounded
    Test();
  }
  
  SetBlinkMode(0);
}

//Main loop of the programm gets called again and again.
void loop() {
  // put your main code here, to run repeatedly:

  //Check if data is available
  if (Serial.available()) {
    
    byte receivedByte = Serial.read();  
    switch (receivedByte) {
      case 'L':
        //Set length of strips
        SetLedStripLength();
        break;
      case 'F':
        //Fill strip area with color
        Fill();
        break;  
      case 'R':
        //receive data for strips
        ReceiveData();
        break;
      case 'O':
        //output data on strip
        OutputData();
        break;   
      case 'C':
        //Clears all previously received led data
        ClearAllLedData();
        break;
      case 'V':
        //Send the firmware version
        SendVersion();  
        break;
      case 'M':
        //Get max number of leds per strip  
        SendMaxNumberOfLeds();
        break;
      case 'T':
        //initiate Test over serial  
        Test();
        break;
      default:
        // no unknown commands allowed. Send NACK (N)
        Nack();
      break;
    }

   
    SetBlinkMode(1);
    
    
  } 
  Blink();
}


//Sets the mode for the blinking of the led
void SetBlinkMode(int Mode) {
  BlinkMode=Mode;
  BlinkModeTimeoutTimer=0;
}

//Controls the blinking of the led
void Blink() {
  switch(BlinkMode) {
    case 0:
      //Blinkmode 0 is only active after the start of the Teensy until the first command is received.
      if(BlinkTimer<1500) {
        digitalWrite(LedPin,0);
      } else if(BlinkTimer<1600) {
        digitalWrite(LedPin,1);
      } else {
        BlinkTimer=0;
        digitalWrite(LedPin,0);
      }
    break;
    case 1:
      //Blinkmode 1 is activated when the Teensy receives a command
      //Mode expires 500ms after the last command has been received resp. mode has been set
      if(BlinkTimer>30) {
        BlinkTimer=0;
        digitalWrite(LedPin,!digitalRead(LedPin));
      }
      if(BlinkModeTimeoutTimer>500) {
        SetBlinkMode(2);
      }
    break;   
    case 2:
      //Blinkmode 2 is active while the Teensy is waiting for more commands
      if(BlinkTimer<1500) {
        digitalWrite(LedPin,0);
      } else if(BlinkTimer<1600) {
        digitalWrite(LedPin,1);
      } else if(BlinkTimer<1700) {
        digitalWrite(LedPin,0);
      } else if(BlinkTimer<1800) {
        digitalWrite(LedPin,1);
      }else {
        BlinkTimer=0;
        digitalWrite(LedPin,0);
      }
      default:
      //This should never be active
      //The code is only here to make it easier to determine if a wrong Blinkcode has been set
      if(BlinkTimer>2000) {
        BlinkTimer=0;
        digitalWrite(LedPin,!digitalRead(LedPin));
      }
      break;
  }
  
}


//Outputs the data in the ram to the ledstrips
void OutputData() {
   leds.show();
   Ack();
}


//Fills the given area of a ledstrip with a color
void Fill() {
  word firstLed=ReceiveWord();

  word numberOfLeds=ReceiveWord();

  int ColorData=ReceiveColorData();

   if( firstLed<=configuredStripLength*8 && numberOfLeds>0 && firstLed+numberOfLeds-1<=configuredStripLength*8 ) {
       word endLedNr=firstLed+numberOfLeds;
       for(word ledNr=firstLed; ledNr<endLedNr;ledNr++) {
         leds.setPixel(ledNr,ColorData);
       }
       Ack();
   } else {
     //Number of the first led or the number of leds to receive is outside the allowed range
     Nack();    
   }
  
  
}


//Receives data for the ledstrips
void ReceiveData() {
  word firstLed=ReceiveWord();

  word numberOfLeds=ReceiveWord();

  if( firstLed<=configuredStripLength*8 && numberOfLeds>0 && firstLed+numberOfLeds-1<=configuredStripLength*8 ) {
    //FirstLedNr and numberOfLeds are valid. 
    //Receive and set color data
    
    word endLedNr=firstLed+numberOfLeds;
    for(word ledNr=firstLed; ledNr<endLedNr;ledNr++) {
      leds.setPixel(ledNr,ReceiveColorData());
    }

    Ack();

  } else {
    //Number of the first led or the number of leds to receive is outside the allowed range
    Nack();
  }
}

//Sets the length of the longest connected ledstrip. Length is restricted to the max number of allowed leds
void SetLedStripLength() {
   word stripLength=ReceiveWord();
   if(stripLength<1 || stripLength>MaxLedsPerStrip) {
     //stripLength is either to small or above the max number of leds allowed
     Nack();
   } else {
     //stripLength is in the valid range
     configuredStripLength=stripLength;
     leds.setStripLength(stripLength);
     leds.begin();  //Reinitialize the OctoWS2811 lib (not sure if this is needed)

     Ack();
   }
}

//Clears the data for all configured leds
void  ClearAllLedData() {
  for(word ledNr=0;ledNr<configuredStripLength*8;ledNr++) {
    leds.setPixel(ledNr,0);
  }
  Ack();
}


//Sends the firmware version
void SendVersion() {
  Serial.write(FirmwareVersionMajor);
  Serial.write(FirmwareVersionMinor);
  Ack();
}

//Sends the max number of leds per strip
void SendMaxNumberOfLeds() {
  byte B=MaxLedsPerStrip>>8;
  Serial.write(B);
  B=MaxLedsPerStrip&255;
  Serial.write(B);
  Ack();
}


//Sends a ack (A)
void Ack() {
  Serial.write('A');
}

//Sends a NACK (N)
void Nack() {
  Serial.write('N');
}

//Receives 3 bytes of color data.
int ReceiveColorData() {
  while(!Serial.available()) {};
  int colorValue=Serial.read(); 
  while(!Serial.available()) {};
  colorValue=(colorValue<<8)|Serial.read();
  while(!Serial.available()) {};
  colorValue=(colorValue<<8)|Serial.read();
  
  return colorValue;


}

//Receives a word value. High byte first, low byte second
word ReceiveWord() {
  while(!Serial.available()) {};
  word wordValue=Serial.read()<<8; 
  while(!Serial.available()) {};
  wordValue=wordValue|Serial.read();
  
  return wordValue;
}

// Colors for testing - assumes WS2812 color order of G, R, B
/*
#define RED    0x00FF00
#define GREEN  0xFF0000
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0x10FF88
#define ORANGE 0x45FF00
#define WHITE  0xFFFFFF
#define BLACK  0x000000
*/

// Less intense colors for testing - assumes WS2812 color order of G, R, B
#define RED    0x001600
#define GREEN  0x160000
#define BLUE   0x000016
#define YELLOW 0x141000
#define PINK   0x001209
#define ORANGE 0x041000
#define WHITE  0x101010
#define BLACK  0x000000


void Test() {
  int microsec = 250000;  // change color every 1/4 second

  ColorWipe(RED, microsec);
  ColorWipe(GREEN, microsec);
  ColorWipe(BLUE, microsec);
  ColorWipe(YELLOW, microsec);
  ColorWipe(PINK, microsec);
  ColorWipe(ORANGE, microsec);
  ColorWipe(WHITE, microsec);
  ColorWipe(BLACK, microsec);
}

void ColorWipe(int color, int wait)
{   
  for (int i=0; i < leds.numPixels(); i++) {
    leds.setPixel(i, color);
  }

  // wait before turning on LEDs and also turn on indicator LED
  delayMicroseconds(100000);
  digitalWrite(LedPin,1);
  leds.show();

  // wait for desginated timeout and then turn off indicator LED
  delayMicroseconds(wait);
  digitalWrite(LedPin,0);
}   
