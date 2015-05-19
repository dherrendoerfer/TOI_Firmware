TOI_Firmware for the Cactus Micro from April Brother
====================================================

A Things-Of-Internet firmware for the Arduino and the ESP8266 module

Opposed to the popular Internet-of-things movement, where all sorts of
devices post their status to the net, here is the Things-Of-Internet 
firmware:

It turns an Arduino into a tiny integrated web server within its own
wifi network, or within an existing one.

"I don't want internet for all my things, look what it did to the children ..."



********************** BETA version ALERT !!!!!! **************************
THIS IS BETA CODE, IT MAY BE INCOMPLETE OR BROKEN. YOU HAVE BEEN WARNED.



Introduction:
-------------

This is a beta drop of the TOI_firmware for the Cactus Micro Arduino 
compatible controller board by April Brother (aprbrother.com).
The firmware implements an application back-end for microcontrollers using 
the ESP8266 Wifi controller module for internet communication.
The firmware features a small HTTP based Web-Server with an easy-to-program
application interface and a complete set of communication feature interfaces.
It also provides a software Real Time Clock with NTP client functions to
automatically set the time from the internet and keep it in sync.
Furthermore download and upload functions are provided to up and download
data to web servers.

Installation:
-------------

The firmware supports only the Cactus Micro Arduino board, with 
an ESP8266 module connected to the Serial1 port.
You are required to solder the HW Serial select pads on the 
bottom-side of the Cactus.
To make use of the flashing function you need to connect IO port 15
from the ATMEGA to the MODE PIN on the ESP8266 module.

The following Arduino IO Ports are used by the firmware:
- Serial1 TX and RX for ESP8266
- Digital IO pin 15 connected to ESP mode select
- Digital IO pin 10 connect to GND to bypass firmware settings from EEPROM
- Digital IO pin 14 connect to GND to activate ESP8266 Flash Programmer mode

Firmware ai-thinker-0.9.5.2-9600.bin is recommended on the ESP8266.

To compile and build this, the Arduino 1.0.6 IDE (or higher version) is
recommended (but no longer requred).


Usage:
------

After powering on, the device will become an Access Point with the SSID
"TOI_Default" and no encryption/passcode enabled. 
Once connected browse to "http://192.168.4.1/arduino.html".
Using the configuration page you may add the device to your existing
network, or create a new AP SSID. If you change the encryption settings
in AP mode you may need to reset the board.
The sketch now includes a primitive clock, acessible via the 'cdate.html'
URL and settable via the config page.


Currently implemented URLs:
---------------------------

http://192.168.4.1/arduino.html  -  Landing page for Arduino TOI firmware
http://192.168.4.1/config.html   -  Configuration page
http://192.168.4.1/c2ee.html     -  Page to set Wifi environment settings
http://192.168.4.1/c2ct.html     -  Page to set time and day
http://192.168.4.1/cntp.html     -  Page to set NTP client settings
http://192.168.4.1/cdate.html    -  Page to query the current time and day

Sample wget code to set time and day:
wget  -q -O - "http://192.168.4.1:80/c2ct.html?ch=$(date +'%H')&cm=$(date +'%M')&cd=$(date +'%w')"


Default Settings:
-----------------

You can reset the board to default settings if you restart the board with
IO pin 10 connected to GND. 
