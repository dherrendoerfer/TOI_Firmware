TOI_Firmware
============

A Things-Of-Internet firmware for the Arduino and the ESP8266 module

Opposed to the popular Internet-of-things movement, where all sorts of
devices post their status to the net, here is the Things-Of-Internet 
firmware:

It turns an Arduino into a tiny integrated web server within its own
wifi network, or within an existing one.

"I don't want internet for all my things, look what it did to the children ..."

Installation:
-------------

The firmware is curently made for the Arduino MEGA board, with the ESP8266
connected to the Serial1 port (I/O 18,19).
To compile and build this, the Arduino 1.0.6 IDE (or higher version) is
required. 

Usage:
------

After powering on, the device will become an Access Point with the SSID
"Default" and no encryption enabled. 
Once connected browse to "http://192.168.4.1".
Using the configuration page you may add the device to your existing
network, or create a new AP SSID. If you change the encryption settings
in AP mode you may need to reset the board.

Default Settings:
-----------------

You can reset the board to default settings if you restart the board with
IO pin 12 connected to GND. 

