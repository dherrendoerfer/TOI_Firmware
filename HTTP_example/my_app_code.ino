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

//#define APP_DEBUG 1

// include the library code:
#include <LiquidCrystal.h>

LiquidCrystal lcd( A3, 2, 3, 4, 5, 6);


long sectics = 0;

/* My-App specific init */
void app_init()
{
   pinMode(A3, OUTPUT);
   lcd.begin(16, 2);
   
   lcd.print("TOI_fw v0.10a");
}

/* My-App specific URL handlers
   Put your web-pages in here. */
int myAppHandleURL(char* URL, int ch_id)
{
  if (strcmp(URL,"/index.html") == 0 || strcmp(URL,"/") == 0) {
    /* index page */
    page_index(ch_id, URL);
    return 0;
  }

  if (strcmp(URL,"/analog.html") == 0) {
    /* analog page */
    page_analog(ch_id, URL);
    return 0;
  }

  if (strcmp(URL,"/digital.html") == 0) {
    /* digital page */
    page_digital(ch_id, URL);
    return 0;
  }

  if (strcmp(URL,"/output.html") == 0) {
    /* temp page */
    page_output(ch_id, URL);
    return 0;
  }

  if (strstr(URL,"/oset.html?") == URL) {
    /* time save page */
    page_oset(ch_id, URL);
    return 0;
  }

  return 1;
}

/* My-App specific shutdown code 
   Note: a reboot or shutdown is in progress */
void shutdown_event()
{
  sectics = 0;
}

/* My-App specific eeprom code 
   Note: an eeprom-write was done */
void eeprom_event()
{

}

  
int tick()
{ 
  if ( seconds == 9 || seconds == 39) {
    char tmp[64];
    if (http_req_get("192.168.178.8", 80, "/test.html", tmp, 96)) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      return 0; 
    }

    if (tmp[strlen(tmp)-1] == '\n')
      tmp[strlen(tmp)-1] = 0;
      
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(tmp);
  }

  if ( millis() % 1000 == 0) {
    char tmp[64];
    sprintf(tmp, "%02d:%02d:%02d",hours,minutes,seconds);
    
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(tmp);
    
    if (sectics++ < 30) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(IP);
    }
  }
}


/*********************************************************************************************** 
*  index.html 
*
*  Simple html page stored in progmem (flash) and sent via
*  response_send_progmem()
*/
prog_char content_index[] PROGMEM =  "<HTML><BODY>index.html<hr>Arduino ESP8266 Web Server Sample<p>"
                          "Available options:<br> "
                          "Read analog inputs <a href=analog.html>analog.html</a><br>"
                          "Read digital inputs <a href=digital.html>digital.html</a><br>"
                          "Set digital outputs <a href=output.html>output.html</a><p>"
                          "To change the Wifi configuration please use<br>"
                          "the built-in config page <a href=arduino.html>arduino.html</a>"
                          "</BODY></HTML>\r\n";
  
void page_index(int ch_id, char* URL) {
  response_send_progmem(ch_id, content_index, sizeof(content_index));  
}


/*********************************************************************************************** 
*  analog.html 
*
*  Return a web page via the stream send functions. Uses very little
*  memory, but requires a lot of care.
*/
void page_analog(int ch_id, char* URL) {
  
  char buf[64];

  if (stream_head(ch_id)) {
    close_channel(ch_id);
    return;  
  }

  sprintf(buf,"Analog:");
  
  for (int i=0; i<8; i++) {
    // Skip A3 (Used by LCD)
    if (i==3)
      continue;
    if (i<7)
      sprintf(buf, "%s%d,",buf,analogRead(i));
    else
      sprintf(buf, "%s%d",buf,analogRead(i));
  }

  sprintf(buf, "%s\r\n", buf);

  stream_send(ch_id, buf);
  close_channel(ch_id);
  
  return;  
}

/*********************************************************************************************** 
*  digital.html 
*
*  Returns a web page via the response_send_simple() function.
*  Requires some memory, but is fast and easy to use.
*/
void page_digital(int ch_id, char* URL) {
  
  char buf[64];

  sprintf(buf,"Digital:");
  
  for (int i=7; i<10; i++) {
    if (i<9)
      sprintf(buf, "%s%d,",buf,digitalRead(i));
    else
      sprintf(buf, "%s%d",buf,digitalRead(i));
  }

  sprintf(buf, "%s\r\n", buf);

  response_send_simple(ch_id, buf, strlen(buf));
}

/*********************************************************************************************** 
*  ouput.html 
*
*  Send a generated page via the stream_send functions.
*  Although pages can become very large uses very little
*  resources.
*/
int outvalues[9] = {0,0,0,0,0,0,0,0,0};

void page_output(int ch_id, char* URL) {
  
  char buf[128];

  if (stream_head(ch_id)) {
    close_channel(ch_id);
    return;  
  }
  if (stream_send(ch_id,"<HTML><BODY>Outputs<hr><p>")) {
    close_channel(ch_id);
    return;  
  }

  for (int i=7; i<10; i++) {
    sprintf(buf, "PIN: %d : value: %d  <a href=oset.html?ch=%d&st=1>ON</a>;<a href=oset.html?ch=%d&st=0>OFF</a><br>",i,outvalues[i],i,i);
    if (stream_send(ch_id, buf)) {
      close_channel(ch_id);
      return;  
    }
  }

  stream_send(ch_id,"<hr></BODY></HTML>\r\n");
  close_channel(ch_id);
  
  return;  
}

/*********************************************************************************************** 
*  oset.html 
*
*  Page with parameter handling. Parameters are stored in the URL string
*  and can be retrieved, reponses are selected and send from progmem.
*/

prog_char content_oset[] PROGMEM = "<HTML><BODY>OUTPUT SET<hr><p><a href=output.html>Back</a></BODY></HTML>\r\n";
prog_char content_oset_err[] PROGMEM = "<HTML><BODY>ERROR<hr><p><a href=output.html>Back</a></BODY></HTML>\r\n";

void page_oset(int ch_id, char* URL)
{
  char* Params = strstr(URL,"?")+1;
  int chan = 0;
  int state = 0;

#ifdef APP_DEBUG  
  Serial.print("Params:" );
  Serial.println(Params);
#endif

  char* r_chan=strstr(Params,"ch=")+3;
  char* r_state=strstr(Params,"st=")+3;

  if (strstr(r_chan,"&"))
    *(strstr(r_chan,"&")) = 0;
  if (strstr(r_state,"&"))
    *(strstr(r_state,"&")) = 0;

  sscanf(r_chan, "%d", &chan );
  sscanf(r_state, "%d", &state );

#ifdef APP_DEBUG  
  Serial.print("Chan:" );
  Serial.println(chan);
  Serial.print("state:" );
  Serial.println(state);
#endif
  
  if (chan > 1 && chan < 10) {
    if (state < 2) {
      outvalues[chan]=state;
      digitalWrite(chan,state);
    } else {
      if (state < 256) {
        outvalues[chan]=state;
        analogWrite(chan,state);
      }
      else {
        response_send_progmem(ch_id, content_oset_err, sizeof(content_oset_err));
        return;
      }
    }
  }
  else {
    response_send_progmem(ch_id, content_oset_err, sizeof(content_oset_err));
    return;
  }

  response_send_progmem(ch_id, content_oset, sizeof(content_oset));
}

/*********************************************************************************************** 
*  Helpers 
*/


