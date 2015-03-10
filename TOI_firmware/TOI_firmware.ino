/*
 * This file is part of TOI_firmware.
 *
 * Copyright (C) 2015  D.Herrendoerfer
 *
 *   TOI_firmware is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   TOI_firmware is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with TOI_firmware.  If not, see <http://www.gnu.org/licenses/>.
 */

//#define INFO_DEBUG 1

#include <stdlib.h>

/* Needed for Arduino UNO, Mini, Nano, etc...  
    comment out for Mega or ADK        */
#include <SoftwareSerial.h>
SoftwareSerial Serial1(10,11);

/* log_mute will mute the next log of the
   serial read. Use for time-critical handling
   where logging would break time constraints. */
int log_mute = 0;

#define HEARTBEAT_LED 13
#define RESET_PIN     12

#define SSID "DefaultSsid"
#define PASS "DefaultPass"
#define NTPIP "176.9.92.196"

/* Pre-defined for the use of get_ip()
   must be large enough to hold 2 IP strings
*/
char IP[32]="000.000.000.000";

char e_SSID[32] = SSID;
char e_PASS[32] = PASS;
int  e_AP       = 0;
int  e_ENC      = 0;
int  e_CHAN     = 10;

char e_NTP[16]  = NTPIP;
int  e_TZ       = 0;
int  e_DO_NTP   = 0;

long t_usPerSec  = 1000000;

#define APSSID "TOI_Default"
#define APPASS ""
#define APCHAN 10
#define APENC 0

#define BUFFER_SIZE 160
char input_buffer[BUFFER_SIZE];

int shutdown = 0;
int reboot   = 0;
int defaultAP = 1;
int reinit = 0;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  
  pinMode(HEARTBEAT_LED, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);

#ifdef INFO_DEBUG
  Serial.print(F("Reset pin state is:"));
  Serial.println(digitalRead(RESET_PIN));
#endif

  if (check_eeprom() && digitalRead(RESET_PIN)) {
#ifdef INFO_DEBUG
    Serial.println(F("Using config from EEPROM"));
#endif
    read_eeprom();
    defaultAP = 0;
  }
  
  /* Execute app-specific init code. */
#ifdef INFO_DEBUG
  Serial.println(F("Calling app_init()"));
#endif
  app_init();

}

unsigned long time;
unsigned long utime;
unsigned long next_us_seconds;
unsigned int seconds = 0;
unsigned int minutes = 0;
unsigned int hours = 0;
unsigned int days = 0;

void init_wifi() 
{
  unset_echo();
  
#ifdef INFO_DEBUG
    Serial.println(F("Setting up AP"));
#endif
  if (defaultAP)
    connectAP(APSSID, APPASS, APCHAN, APENC);
  else {
    if (!e_AP) {
      connectWiFi(e_SSID, e_PASS);
    } else {
      connectAP(e_SSID, e_PASS, e_CHAN, e_ENC);
    }
  }

//  send_expect("AT+CWLAP","OK\r\n",10000);
//  send_expect("AT+CWJAP?","OK\r\n",10000);

  get_ip(IP,sizeof(IP));
  
#ifdef INFO_DEBUG  
  send_dump("AT+CIFSR");
#endif
  
  set_multicon();

  //get ntp day & time
  if (e_DO_NTP) {
#ifdef INFO_DEBUG
    Serial.println(F("Getting initial time via NTP"));
#endif
    ntp_time_get(e_NTP);  
  }
  
  setup_server(80);
}

void loop() 
{
#ifdef INFO_DEBUG
    Serial.println(F("Entering main loop()"));
#endif
  /* Start up Wifi */
  if (!send_expect("AT","OK\r\n",500)){
    init_wifi();
  } else {
#ifdef INFO_DEBUG
    Serial.println(F("No response from ESP8266 will retry in 60s."));
#endif
    delay(60000);
    return;
  }

  // prepare the next clock tick
  next_us_seconds = micros()+t_usPerSec;
  
  while (!shutdown) {
    time = millis();
    utime = micros();

    if (Serial1.available() > 0) {
      /* Call the server service routine if there is data */
      esp_poll();
    }
    
    /* The clock code:
     * This provides a simple running clock time with a 
     * day of the week counter. Because the Arduino quarz
     * clock is almost always off by a few percent the clock
     * is synchronized with the micros() call and a presettable
     * value for elimination of drift. (t_usPerSec)  */
    if ( (long)(utime - next_us_seconds) >= 0 ) {
      next_us_seconds += t_usPerSec;
      seconds++;
      if (seconds > 59) {
        seconds = 0;
        minutes++;
        if (minutes > 59) {
          minutes = 0;
          hours++;
          if (hours > 23) {
            hours = 0;
            days++;
            if (days > 6) {
              days = 0;
            }
          }
        }
      }
      
      /*Hearteat LED*/
      digitalWrite(13, seconds % 2);

#ifdef INFO_DEBUG  
      if ( seconds == 59){
        send_dump("AT+CIFSR");
      }
#endif
    }
    
    if (reinit == 1) {
      reinit = 0;
      init_wifi();
    }
    
    /*Space for aditional code:
     * If you want to extend the code with your own functions
     * that should run periodically, below here is the place to
     * put it. */
  
    tick();

  }  /* End of the main loop */
  
  /* Call app-specific shutdown code */
  shutdown_event();
  
  while (!reboot)
  {
 #ifdef INFO_DEBUG
    Serial.println(F("Shutdown. Cycle power or reset."));
 #endif
    delay(60000);
  }

#ifdef INFO_DEBUG  
  Serial.println(F("Rebooting."));
#endif

  shutdown=0;
  reboot=0;
  stop_server(80);
  unset_multicon();
}
