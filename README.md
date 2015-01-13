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

The firmware supports Arduino MEGA, UNO, Mini, Nano (and more) boards, with 
an ESP8266 module connected to the Serial1 port (I/O 18,19),  I/O pin
10 and 11 for SoftwareSerial.
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

Default Settings:
-----------------

You can reset the board to default settings if you restart the board with
IO pin 12 connected to GND. 

Version History
---------------
2015/01/13 no version     - clean up license statements, add license file
                          - remove day 7 (blernsday) of the week now back to 0-6
                          - save clock micros per second value to eeprom
                          - start work on real http-get function
2015/01/08 no version     - big rework, all static docs now in progmem
                          - removed all use of String class
                          - also works with Uno, Nano, and Mini (needs 3.3V supply)
                          - added clock. More comments
2015/01/05 no version yet - first commit of alpha