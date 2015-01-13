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

#define SERV_DEBUG 1

int request_get( char* ip_buffer, int length)
{
  int ch_id;
  
  char *URL;
  URL = strstr(ip_buffer,"GET")+4;
  
  *(strstr(ip_buffer,"HTTP")-1) = 0;

  char *ID = strstr(ip_buffer,"IPD")+4;
  sscanf(ID, "%d", &ch_id );

#ifdef SERV_DEBUG  
  Serial.print("URL: ");
  Serial.println(URL);
  Serial.print("ID: ");
  Serial.println(ch_id);
  Serial.print("buff_len: ");
  Serial.println(length);
#endif

  if (strcmp(URL,"/index.html") == 0 || strcmp(URL,"/") == 0) {
    /* index page */
    return page_index(ch_id, URL);
  }

  /* 
   * Built-in pages for configuration and test 
   */
  if (strcmp(URL,"/arduino.html") == 0 ){
    /* arduino page */
    return page_arduino(ch_id, URL);
  }

  if (strstr(URL,"/reboot.html?") == URL) {
    /* reboot page */
    return page_reboot(ch_id, URL);
  }

  if (strcmp(URL,"/config.html") == 0) {
    /* config page */
    return page_config(ch_id, URL);
  }

  if (strstr(URL,"/c2ee.html?") == URL) {
    /* connfig save page */
    return page_c2ee(ch_id, URL);
  }

  if (strstr(URL,"/c2ct.html?") == URL) {
    /* time save page */
    return page_c2ct(ch_id, URL);
  }

  if (strcmp(URL,"/cdate.html") == 0) {
    /* show time page */
    return page_cdate(ch_id, URL);
  }

  response_404(ch_id);
    
  return 1;
}

int intLen(unsigned int x) {
    if(x>=10000) return 5;
    if(x>=1000) return 4;
    if(x>=100) return 3;
    if(x>=10) return 2;
    return 1;
}

prog_char header[] PROGMEM = "HTTP/1.1 200 OK\r\n" 
                        "Content-Type: text/html\r\n"
                        "Connection: close\r\n"
                        "Content-Length: ";

int response_head(int ch_id, int length)
{

  int len = sizeof(header) + intLen(length) + 4;

  send_ipdata_head(ch_id, "", len);
  send_progmem_data((char*)header, sizeof(header));
  send_ipdata(length);
  return  send_ipdata_fin("\r\n\r\n");
}

int response_send_simple(int ch_id, char* content, int length)
{
  if (response_head(ch_id,length))
    return 1;
    
  send_ipdata_head(ch_id, content, length);
  return send_ipdata_fin("");
}

void send_progmem_data(PGM_P content, int length) 
{
  char tmp[] = "a"; 
  for ( int i=0; i<length; i++ ) {
    tmp[0] =  pgm_read_byte(content++);
    send_ipdata(tmp);
  }
}

int response_send_progmem(int ch_id, PGM_P content, int length)
{
  if (response_head(ch_id,length))
    return 1;
  send_ipdata_head(ch_id, "", length);
  send_progmem_data(content, length);
  
  return send_ipdata_fin("");
}

prog_char error_404[] PROGMEM = "HTTP/1.1 404 ERROR\r\n"
                        "Content-Type: text/html\r\n"
                        "Connection: close\r\n"
                        "Content-Length: 9\r\n\r\n"
                        "Error 404";

int response_404(int ch_id)
{
  send_ipdata_head(ch_id,"" , sizeof(error_404));
  send_progmem_data((char*)error_404, sizeof(error_404));

  return  send_ipdata_fin("");
}

/*********************************************************************************************** 
*  arduino.html 
*/
prog_char content_arduino[] PROGMEM =  "<HTML><BODY>arduino.html<hr>Built-in pages:<p>"
                          "<a href=config.html>Configuration</a><br>"
                          "<a href=reboot.html?reboot>reboot</a><br>"
                          "<a href=reboot.html?shutdown>shutdown</a><br>"
                          "<a href=reboot.html?default>default settings</a>"
                          "</BODY></HTML>\r\n";
  
int page_arduino(int ch_id, char* URL) {
  return response_send_progmem(ch_id, content_arduino, sizeof(content_arduino));  
}

/*********************************************************************************************** 
*  reboot.html 
*/
prog_char content_reboot[] PROGMEM = "<HTML><BODY>Arduino reboot.html<hr>will shutdown/reboot to selected settings</BODY></HTML>\r\n";

int page_reboot(int ch_id, char* URL)
{
  response_send_progmem(ch_id, (char*)content_reboot, sizeof(content_reboot));  

  if (strstr(URL,"?default")) {
    invalidate_eeprom();
    defaultAP=1;
  } else {
    defaultAP=0;
  }
  
  if (strstr(URL,"?shutdown")) {
    reboot = 0;
  } else {
    reboot = 1;
  }
  
  shutdown=1;
}

/*********************************************************************************************** 
*  config.html 
*/
int eeprom_unlock = 0;
prog_char content_config[] PROGMEM = "<HTML><BODY>Arduino config.html<hr>"
                         "<h5>Wifi Setup<br>"
                         "<form action=\"c2ee.html\" method=\"get\">"
                         "SSID:<input name=\"ssid\" value=\"MySSID\"><br>"
                         "Password:<input name=\"pass\" value=\"MyPASSWORD\"><br>"
                         "<input type=\"checkbox\" name=\"ap\" value=\"on\">Run as AP<br>"
                         "<input type=\"checkbox\" name=\"enc\" value=\"on\">Run as AP with encryption"
                         "<br>AP on channel:<input name=\"chan\" value=\"10\" size=\"2\"><br>"
                         "<button>Save</button></form>"
                         "<hr><form action=\"c2ct.html\" method=\"get\">"
                         "Set time:<input name=\"ch\" value=\"12\" size=\"2\">"
                         ":<input name=\"cm\" value=\"0\" size=\"2\"><br>"
                         "Day(0-7):<input name=\"cd\" value=\"0\" size=\"2\"><br>"
                         "Timebase:<input name=\"cb\" value=\"1000000\" size=\"7\"><br>"
                         "<button>Set</button></form>"
                         "</BODY></HTML>\r\n";

int page_config(int ch_id, char* URL)
{
  eeprom_unlock=1;  
  return response_send_progmem(ch_id, content_config, sizeof(content_config));  
}

/*********************************************************************************************** 
*  c2ee.html 
*/
prog_char content_c2ee[] PROGMEM = "<HTML><BODY>SAVE<hr>OK<p><a href=reboot.html?reboot>reboot</a></BODY></HTML>\r\n";

int page_c2ee(int ch_id, char* URL)
{
  char* Params = strstr(URL,"?")+1;

#ifdef SERV_DEBUG  
  Serial.print("Params:" );
  Serial.println(Params);
#endif

  char* r_ssid=strstr(Params,"ssid=")+5;
  char* r_pass=strstr(Params,"pass=")+5;
  char* r_chan=strstr(Params,"chan=")+5;

    if (strstr(Params,"ap=on")) {
      e_AP=1;
    } else {
      e_AP=0;
    }

    if (strstr(Params,"enc=on")) {
      e_ENC=3;
    } else {
      e_ENC=0;
    }

  if (strstr(r_ssid,"&"))
    *(strstr(r_ssid,"&")) = 0;
  if (strstr(r_pass,"&"))
    *(strstr(r_pass,"&")) = 0;
  if (strstr(r_chan,"&"))
    *(strstr(r_chan,"&")) = 0;

  sscanf(r_chan, "%d", &e_CHAN );
  
#ifdef SERV_DEBUG  
  Serial.println("Cred:" );
  Serial.print("SSID:" );
  Serial.println(r_ssid);
  Serial.print("PASS:");
  Serial.println(r_pass);
  Serial.print("CHAN:");
  Serial.println(e_CHAN);
  Serial.print("AP:");
  Serial.println(e_AP);
  Serial.print("AP_ENC:");
  Serial.println(e_ENC);
#endif

  if (eeprom_unlock)
  {
#ifdef SERV_DEBUG  
    Serial.println("Updating EEPROM:" );
#endif 
    strncpy(e_SSID,r_ssid,32);
    strncpy(e_PASS,r_pass,32);
    
    write_eeprom();
    defaultAP = 0;
  }
  
  return response_send_progmem(ch_id, content_c2ee, sizeof(content_c2ee));  
}

/*********************************************************************************************** 
*  c2ct.html 
*/
prog_char content_c2ct[] PROGMEM = "<HTML><BODY>TIME SET<hr><p><a href=index.html>Home</a></BODY></HTML>\r\n";

int page_c2ct(int ch_id, char* URL)
{
  char* Params = strstr(URL,"?")+1;

#ifdef SERV_DEBUG  
  Serial.print("Params:" );
  Serial.println(Params);
#endif

  char* r_hour=strstr(Params,"ch=")+3;
  char* r_min=strstr(Params,"cm=")+3;
  char* r_day=strstr(Params,"cd=")+3;
  char* r_base=strstr(Params,"cb=")+3;

  if (strstr(r_min,"&"))
    *(strstr(r_min,"&")) = 0;
  if (strstr(r_hour,"&"))
    *(strstr(r_hour,"&")) = 0;
  if (strstr(r_day,"&"))
    *(strstr(r_day,"&")) = 0;
  if (strstr(r_base,"&"))
    *(strstr(r_base,"&")) = 0;

  if (r_base) {
    sscanf(r_base, "%ld", &t_usPerSec );
    if (eeprom_unlock)
      write_eeprom();
  }
  
  sscanf(r_min, "%d", &minutes );
  sscanf(r_hour, "%d", &hours );
  sscanf(r_day, "%d", &days );

  seconds=0;
  
#ifdef SERV_DEBUG  
  Serial.print("New time:" );
  Serial.print(hours);
  Serial.print(":");
  Serial.println(minutes);
  Serial.print("Day:");
  Serial.println(days);
  Serial.print("Base:");
  Serial.println(t_usPerSec);
#endif

  return response_send_progmem(ch_id, content_c2ct, sizeof(content_c2ct));  
}

/*********************************************************************************************** 
*  cdate.html 
*/
int page_cdate(int ch_id, char* URL)
{
  char buffer[80];
  
  sprintf(buffer,"<HTML><BODY>%02i:%02i:%02i on day %i</BODY></HTML>\r\n",hours,minutes,seconds,days);
    
  return response_send_simple(ch_id, buffer, strlen(buffer)+1);  
}


/*********************************************************************************************** 
*        END OF BUILT-IN Functions and html pages 
*/
/*********************************************************************************************** 
*  index.html 
*/
prog_char content_index[] PROGMEM =  "<HTML><BODY>index.html<hr>the default page<p>"
                          "</BODY></HTML>\r\n";
  
int page_index(int ch_id, char* URL) {
  return response_send_progmem(ch_id, content_index, sizeof(content_index));  
}


