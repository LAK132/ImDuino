# ImDuino
ImGui on Arduino (ESP32) example

# Requirements

1. The TFT_22_ILI9225 screen and library
2. An Arduino with SPI (this example was designed for an ESP32)

# Setting up hardware

ImDuino is configured by default to use:

* Pin 32 for LED
* Pin 33 for reset
* Pin 27 for RS
* Pin 15 for CS
* Pin 15 for clock
* Pin 13 for SDI

These can be changed by modifying the constants at the top of ImDuino.ino
