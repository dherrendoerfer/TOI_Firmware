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

/* HARDWARE DEFINITIONS FOR CACTUS BOARD */
#define SerialESP     Serial1
#define M_ESP_ENABLE  13
#define M_ESP_FLASH   15
#define M_RESET_PIN   10
#define M_FLASH_PIN   14

/* Input Buffer size for the web server */
#define M_BUFFER_SIZE 400

/* DEFAULT WIFI DEFINITIONS FOR THE ESP8266 */
#define SSID "DefaultSsid"
#define PASS "DefaultPass"
#define NTPIP "176.9.92.196"

/* log_mute will mute the next log of the
 * serial read. Use for time-critical handling
 * where logging would break time constraints. */
boolean log_mute = 0;

/* Pre-defined for the use of get_ip() */
char IP[16]="000.000.000.000";

/* Variable declarations for settings stored
 * to EEPROM */
char e_SSID[32] = SSID;
char e_PASS[32] = PASS;
int  e_AP       = 0;
int  e_ENC      = 0;
int  e_CHAN     = 10;

char e_NTP[16]  = NTPIP;
int  e_TZ       = 0;
int  e_DO_NTP   = 0;

/* Duration of a Second in milliseconds.
 * Modify this value if the oscillator of the 
 * Arduino is too far off. This only influences
 * software real time clock */
long t_usPerSec  = 1000000;

/* DEFINITIONS FOR THE INITIAL AP MODE */
#define APSSID "TOI_Default"
#define APPASS ""
#define APCHAN 10
#define APENC 0

/* Declarations for the incomming IP data buffer and 
 * packet storage */
char input_buffer[M_BUFFER_SIZE + 1]; //Adding one byte for zero-termination
int input_buffer_ptr = 0;
int num_pkt = 0;
char *pkt[5];
int   pktlen[5];
int   pktchan_id[5];

/* Declaration of the variables for the state machine */
boolean shutdown = 0;
boolean reboot   = 0;
boolean defaultAP = 1;
boolean reinit = 0;
boolean mode_flasher = 0;

void setup()
{
  /* At startup check if the pin for flash mode has
   * been connected to ground, in which case we go 
   * into flashing mode, and stay there */
  pinMode(M_FLASH_PIN, INPUT_PULLUP);
  delay(200);

  if (digitalRead(M_FLASH_PIN) == 0) {
    Serial.begin(115000);
    SerialESP.begin(115000);
    mode_flasher = 1;
    return;
  }

  /* Normal startup for the TOI firmware continues here */

#ifdef M_ESP_FLASH     
  pinMode(M_ESP_FLASH, INPUT);
#endif

  /* Setup serial */
  Serial.begin(9600);
  SerialESP.begin(9600);
  
#ifdef INFO_DEBUG
  while (!Serial) {
    ;
  }
#endif
  
  /* Make sure the input buffer ends at its end */
  input_buffer[M_BUFFER_SIZE] = 0; //just to be sure
  
#ifdef INFO_DEBUG
  Serial.print(F("Free RAM:"));
  Serial.println(freeRam());
#endif
    
   /* Various hardware dependent initializations */ 
#ifdef M_HEARTBEAT_LED     
  pinMode(M_HEARTBEAT_LED, OUTPUT);
#endif

#ifdef M_ESP_ENABLE     
  pinMode(M_ESP_ENABLE, OUTPUT);
  Serial.println(F("Setting ESP enable on."));
  digitalWrite(M_ESP_ENABLE,1);
  delay(500);
  serial_flush(); 
#endif

  /* Check if the RESET CONFIG pin is pulled low 
   * if it is, don't load EEPROM settings */
  pinMode(M_RESET_PIN, INPUT_PULLUP);

#ifdef INFO_DEBUG
  Serial.print(F("Reset pin state is:"));
  Serial.println(digitalRead(M_RESET_PIN));
#endif

  if (check_eeprom() && digitalRead(M_RESET_PIN)) {
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

/* Declaration of variables for the software realtime clock. */
unsigned long utime;
unsigned long next_us_seconds;
unsigned int seconds = 0;
unsigned int minutes = 0;
unsigned int hours = 0;
unsigned int days = 0;

/* Timeout handling */
unsigned long next_timeout = 0;

void init_wifi() 
{
  unset_echo();
  
#ifdef INFO_DEBUG
    Serial.println(F("Setting up AP"));
#endif
  if (defaultAP) {
    connectAP(APSSID, APPASS, APCHAN, APENC);
  }
  else {
    if (!e_AP) {
      connectWiFi(e_SSID, e_PASS);
    } else {
      connectAP(e_SSID, e_PASS, e_CHAN, e_ENC);
    }
  }

  get_ip(IP,sizeof(IP),( defaultAP | e_AP));

#ifdef INFO_DEBUG
    Serial.print(F("Local IP is:"));
    Serial.println(IP);
#endif

//  send_expect("AT+CWLAP","OK\r\n",10000);
//  send_expect("AT+CWJAP?","OK\r\n",10000);
  
#ifdef INFO_DEBUG  
  send_dump("AT+CIFSR");
#endif
  
  /* Enable multiple connection mode on ESP8266 */
  set_multicon();

  //get ntp day & time
  if (e_DO_NTP) {
#ifdef INFO_DEBUG
    Serial.println(F("Getting initial time via NTP"));
#endif
    ntp_time_get(e_NTP);  
  }
 
  /* Fire up the server socket on the ESP8266 
   * and wait for connections to come. */ 
  setup_server(80);
}

void loop() 
{
  /* Flash mode hook, if flash mode is selected we do 
   * nothing but that until reset */
  if (mode_flasher) {
    do_flasher();
    return;
  }
  
#ifdef INFO_DEBUG
    Serial.println(F("Entering main loop()"));
#endif

  if (send_expect("AT","OK\r\n",1000)) {
#ifdef INFO_DEBUG
    Serial.println(F("No response from ESP8266 will retry in 60s."));
#endif
    delay(60000);
    return;
  }

  /* Start up Wifi */
  init_wifi();

  // prepare the next clock tick
  next_us_seconds = micros() + t_usPerSec;
  next_timeout = micros() + 30*t_usPerSec;
  /* duty loop, running until shutdown is set */
  while (!shutdown) {
    utime = micros();

    if (SerialESP.available() > 0 || num_pkt > 0) {
      /* Call the server service routine if there is data */
      esp_poll();
    }
    
    /* The clock code:
     * This provides a simple running clock time with a 
     * day of the week counter. Because the Arduino quarz
     * clock is almost always off by a few percent. The clock
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
      
      /* Tell the application 1 sec has passed*/
      second_tick();
      
#ifdef HEARTBEAT_LED     
      /*Blink the hearteat LED*/
      digitalWrite(13, seconds % 2);
#endif

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

    /* Timeout/health check */
    if ((long)(utime - next_timeout) >= 0 ) {
      char c_ip[16]="000.000.000.000";

#ifdef INFO_DEBUG
      Serial.println(F("timeout_event() called"));
#endif

      if (!get_ip(c_ip,sizeof(c_ip),( defaultAP | e_AP))) {
#ifdef INFO_DEBUG
        Serial.println(F("get_ip() -> OK"));
#endif
      }
      
      if ( strncmp(c_ip,IP,sizeof(c_ip)) == 0) {
#ifdef INFO_DEBUG
        Serial.println(F("IP -> OK"));
#endif
      }

      timeout_event();
      next_timeout = utime + 30*t_usPerSec;
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

  /* Clean up, the main loop will be called again. */
  shutdown=0;
  reboot=0;
  stop_server(80);
  unset_multicon();
}
