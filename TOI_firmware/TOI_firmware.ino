/*
 * This file is part of esp_webserver.
 *
 * Copyright (C) 2015  D.Herrendoerfer
 *
 *   uCNC_controller is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   uCNC_controller is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with uCNC_controller.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#define HEARTBEAT_LED 13
#define RESET_PIN     12

#define SSID "Default"
#define PASS "MyGoodPasswd"

char e_SSID[32] = SSID;
char e_PASS[32] = PASS;
int  e_AP       = 0;
int  e_ENC      = 0;
int  e_CHAN     = 10;

#define APSSID "IOT_Default"
#define APPASS ""
#define APCHAN 10
#define APENC 0

#define BUFFER_SIZE 512
char input_buffer[BUFFER_SIZE];

int shutdown = 0;
int reboot   = 0;
int defaultAP = 1;

void setup()
{
  Serial1.begin(9600);
  Serial.begin(9600);
  
  pinMode(HEARTBEAT_LED, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);

  Serial.print("Reset pin state is:");
  Serial.println(digitalRead(RESET_PIN));
 
  if (check_eeprom() && digitalRead(RESET_PIN)) {
    Serial.println("Using config from EEPROM");
    read_eeprom();
    defaultAP = 0;
  }
}

void loop() {
  unsigned long time;
  unsigned long seconds = 0;

  /* Start up Wifi */
  if (!send_expect("AT","OK\r\n",500)){
    if (defaultAP)
      connectAP(APSSID, APPASS, APCHAN, APENC);
    else {
      if (!e_AP) {
        connectWiFi(e_SSID, e_PASS);
      } else {
        connectAP(e_SSID, e_PASS, e_CHAN, e_ENC);
      }
    }
  } else {
    Serial.println("No response from ESP8266 will retry in 60s.");
    delay(60000);
    return;
  }
  
  send_dump("AT+CIFSR");

  set_multicon();
  setup_server(80);
 
  while (!shutdown) {
    time = millis();

    if (time % 10 == 0) {
      /* Call the server service routine every 10ms*/
      esp_poll();
    }
    
    if (time % 1000 == 0) {
      seconds++;
      /*Hearteat LED*/
      digitalWrite(13, seconds % 2);
      delay(1);
    }
  }
  
  while (!reboot)
  {
    Serial.println("Shutdown. Cycle power or reset.");
    delay(60000);
  }
  
  Serial.println("Rebooting.");
  shutdown=0;
  reboot=0;
  stop_server(80);
  unset_multicon();
}
