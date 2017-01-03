# XY-Scope
X Y axis Oscilloscope (Vector Scope) for Arduino

License: MIT

This code will work on any TFT LCD compatible with Arduino's TFT.h when wired using hardware SPI pins.
The resolution is auto-detected and the axis with the least pixels will be selected as the vector square resolution.
The analog input is expected to be between 0V and 5V. For AC signals going below 0V (like audio), you must add voltage divider resistors (VIN/2) and a coupling capacitor for each channels.

Simple input circuit (see in raw text):
                   Vin (5v)
                      |
                     10k
                      |
Audio Source - 10µF - + - Analog Input
                      |
                     10k
                      |
                   Ref (0V)
