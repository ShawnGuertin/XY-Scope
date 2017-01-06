/* Copyright 2017 Shawn Guertin
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Useful information about the ADC prescaler and interrupt configuration:
http://www.glennsweeney.com/tutorials/interrupt-driven-analog-conversion-with-an-atmega328p
http://www.microsmart.co.za/technical/2014/03/01/advanced-arduino-adc/
http://apcmag.com/arduino-analog-to-digital-converter-how-it-works.htm/

This code will work on any TFT LCD compatible with Arduino's TFT.h when wired using hardware SPI pins.
The resolution is auto-detected and the axis with the least pixels will be selected as the vector square resolution.
The analog input is expected to be between 0V and 5V. For AC signals going below 0V (like audio), you must add voltage divider resistors (VIN/2) and a coupling capacitor for each channels.

I use pin A0 for channel X and A1 for channel X, to change it you need to change all occurrence of analogRead and register ADMUX. The ADMUX bits are 0 ,1 ,2 and 3.

Note that the registries used are for the ATmega48A/PA/88A/PA/168A/PA/328/P
*/

#include <TFT.h>  // Arduino included LCD library
#include <SPI.h>  // Arduino included SPI library

// Plug the TFT to the hardware SPI pins on the Arduino, plus the following:
// pin definition for the Uno
#define cs   10
#define dc   9
#define rst  8

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
// Global variables to process the channels
unsigned int channelx,channely;

// Global variable for the divider
unsigned char divider;

// High when an analog value is ready to be read
volatile boolean readflag = 0;

// Variable used to store analog result
volatile unsigned int analogval;

void setup() {

  // initialize the display
  TFTscreen.begin();
  
  // clear the screen black background
  TFTscreen.background(0, 0, 0);
  
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
  
  // Since the loop only processes and display the previously captured sample, we need to set the initial sample. Don't reuse the analogRead() function after changing the registries.
  channelx = analogRead(0) / divider;
  channely = analogRead(1) / divider;
  
// ADC register manipulation:

/* ADMUX Register (Atmel Atmega328P manual page 248)
  
   BITS :  76543210 */
  ADMUX = B01000000;
  /* Bit description:
  7 and 6: 01 is to reference the ADC to VCC (11 would be for for 1,1V internal reference)
  5: 0 is to right-adjust the result ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  4 is unused
  3,2,1,0: 0000 is to select ADC0
  */
  
/* ADCSRA Register (Atmel Atmega328P manual page 249)

    BITS :  76543210 */
  ADCSRA = B10001100;
  /* Bit description:
  7: enables the ADC
  6: In Single Conversion mode, write this bit to one to start each conversion. We are not ready to start it yet.
  5: 0 to disable Auto Triggering
  4: This bit is set when an ADC conversion completes and the Data Registers are updated.
  3: 1 to activate ADC Interrupt Enable
  2,1,0: ADC Prescaler Select Bits. Binary 100 is to divide 16MHz by 16, to get an ADC clock of 1MHz.
    Note: Atmel recommends 50-200kHZ ADC clock for 10bit max precision, and states that up to 1MHz will output 8bit equivalent precision.
  */

/*Enable global interrupts
  AVR macro included in <avr/interrupts.h>, which the Arduino IDE
  supplies by default. */
  sei();
  
  // Set ADSC in ADCSRA (0x7A) to start the initial ADC conversion
  ADCSRA |=B01000000;
  // Now the ADC is working, and will interrupt when a result is ready.
}

void loop() {

  /* ATTENTION, it might be confusing to try to understand the code form here.
  The reason it starts here is to display the initial sample (from the setup() function). */

  // Set the stroke colour to Green (this is going to be the RGB colour of the scope)
  TFTscreen.stroke(0, 255, 0);
  // Draw the point to the screen
  TFTscreen.point(channelx, channely);

  // Set the stroke colour to black
  TFTscreen.stroke(0, 0, 0);
  // Erase the oldest sample in the sample memory arrays
  TFTscreen.point(samplememoryx[arrayposition], samplememoryy[arrayposition]);

  // Save the previous samples to the sample memory arrays
  samplememoryx[arrayposition] = channelx;
  samplememoryy[arrayposition] = channely;
  
  // Change array position to be ready for next loop iteration
  if (arrayposition == (samplenumber - 1))
  {
    // If the position is at the end of the array, then rollover to 0
    arrayposition = 0;
  }
  else
  {
    ++arrayposition;
  }
  
  // Getting prepared to switch to the next analog input:
  // According to the datasheet, changing the MUX will not go in effect until the current conversion is complete.

  // Analog conversion for Channel Y:
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog input
  ADMUX &= B11110000;
  // Change MUX to select analog input 1
  ADMUX |= B00000001;
  
  //Processing done. Waiting for the ADC to finish the analog conversion for Channel X.
  while(!readflag)
  {
	  // Waiting for Channel X...
  }
  
  // The ADC is done. Starting the ADC conversion for Channel Y:
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;
  // Now the ADC is working, and will interrupt when a result is ready.
  
  // Reseting the readflag
  readflag = 0;
  
  /* The Interrupt service routine for the ADC completion is simple on purpose
  We need to put the channel x ADC output in the channelx variable
  And let's convert the ADC's 10bit result to the screen's resolution */
  channelx = analogval / divider;
  
  // Getting prepared to switch to the next analog input:
  // According to the datasheet, changing the MUX will not go in effect until the current conversion is complete.

  // Analog conversion for Channel X:
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog input
  ADMUX &= B11110000;
  /* Since we use channel 0, the previous step already set the channel to the right channel (0000), to select another pin the following line would be: 
  ADMUX |= B0000XXXX;  // Where XXXX is the required channel*/
  
  //Processing done. Waiting for the ADC to finish the analog conversion for Channel Y.
  while(!readflag)
  {
	  // Waiting for Channel Y...
  }
  
  // The ADC is done. Starting the ADC conversion for Channel Y:
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;
  // Now the ADC is working, and will interrupt when a result is ready.
  
  // Reseting the readflag
  readflag = 0;
  
  /* The Interrupt service routine for the ADC completion is simple on purpose
  We need to put the channel x ADC output in the channelx variable
  And let's convert the ADC's 10bit result to the screen's resolution */
  channely = analogval / divider;
  
}

// Interrupt service routine for the ADC completion
ISR(ADC_vect){

  // Done reading
  readflag = 1;
 
  // Must read low first
  analogval = ADCL | (ADCH << 8);
}