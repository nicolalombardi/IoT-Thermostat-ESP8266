# IoT-Thermostat-ESP8266
This is the firmware for a thermostat I made using and ESP8266 and some additional hardware. Take a look at [this repository](https://github.com/nicolalombardi/IoT-Thermostat-WebApp) for the web interface.

## Hardware
The required hardware is shown and listed below:
<p><img src="https://i.imgur.com/jbL3K4J.png" style="display:block;" width="80%"></p>

- ESP8266
- Rotary encoder with a button
- 128x64 OLED display based on the SSD1306 driver
- RTC module (I used DS1307)
- Temperature sensor
- Relay module

## Using this project
### Installing the necessary libraries
Copy the libraries folder to you Arduino libraries folder and then change the lib_dir property inside of the file platformio.ini to your folder.
### Compiling
This project was created using Platform.io so it includes a platformio.ini with all the necessary configuration for compiling the project. 
