/* Copyright 2017 Shawn Guertin
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Source of the ADC prescaler configuration:
http://www.microsmart.co.za/technical/2014/03/01/advanced-arduino-adc/

This code will work on any TFT LCD compatible with Arduino's TFT.h when wired using hardware SPI pins.
The resolution is auto-detected and the axis with the least pixels will be selected as the vector square resolution.
The analog input is expected to be between 0V and 5V. For AC signals going below 0V (like audio), you must add voltage divider resistors (VIN/2) and a coupling capacitor for each channels.
*/

#include <TFT.h>  // Arduino included LCD library
#include <SPI.h>  // Arduino included SPI library

// Define analog pins for X and Y axis
#define pinx A0
#define piny A1

// Plug the TFT to the hardware SPI pins on the Arduino, plus the following:
// pin definition for the Uno
#define cs   10
#define dc   9
#define rst  8

// Define various ADC prescaler
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

// Define TFT pins
TFT TFTscreen = TFT(cs, dc, rst);

// Global array for the sample retention (number of points to leave on the screen before erasing)
// Number of samples required (change as free sram is available, 1 sample = 2 bytes):
#define samplenumber 64
// Current position in the array to read and save samples
unsigned int arrayposition = 0;
// Arrays for channel X and channel Y sample retention (memory)
unsigned int samplememoryx[samplenumber] = { 0 };
unsigned int samplememoryy[samplenumber] = { 0 };

// Global variable vor the divider
unsigned char divider;

void setup() {

  // initialize the display
  TFTscreen.begin();
  
  // clear the screen black background
  TFTscreen.background(0, 0, 0);

  // set up the ADC
  ADCSRA &= ~PS_128;  // remove bits set by Arduino library
  // you can choose a prescaler from above.
  // PS_16, PS_32, PS_64 or PS_128
  ADCSRA |= PS_16;    // set our own prescaler to 16 (1Mhz)
  
  // Define the divider to convert the 10bit (1024) ADC to the TFT's screen resolution
  // Example for a TFT of 128x128: 1024/128=8
  if (TFTscreen.height() <= TFTscreen.width())
  {
  divider = (1024/TFTscreen.height());
  }
  else
  {
    divider = 1024/TFTscreen.width();
  }

}

void loop() {
  // Set variables
  unsigned int channelx,channely;

  // Get analog inputs ASAP and process them after (to reduce the offset between X and Y)
  channelx = analogRead(pinx);
  channely = analogRead(piny);
  // Convert the ADC's 10bits to the screen's resolution
  channelx = channelx / divider;
  channely = channely / divider;

  // Set the stroke colour to Green (this is going to be the RGB colour of the scope)
  TFTscreen.stroke(0, 255, 0);
  // Draw the point to the screen
  TFTscreen.point(channelx, channely);

  // Set the stroke colour to black
  TFTscreen.stroke(0, 0, 0);
  // Erase the oldest sample in the array
  TFTscreen.point(samplememoryx[arrayposition], samplememoryy[arrayposition]);

  // Save the channels to the current sample positions
  samplememoryx[arrayposition] = channelx;
  samplememoryy[arrayposition] = channely;
  
  if (arrayposition == (samplenumber - 1))
  {
    // If the position is at the end of the array, then rollover to 0
    arrayposition = 0;
  }
  else
  {
    ++arrayposition;
  }
}
