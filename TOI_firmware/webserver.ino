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

#define SERV_DEBUG 1

int request_get( String ip_data)
{
  int ch_id;
  
  String URL = ip_data.substring(ip_data.indexOf("GET")+4, ip_data.indexOf("HTTP"));
  URL.trim();
  String ID  = ip_data.substring(ip_data.indexOf("IPD,")+4, ip_data.indexOf(":"));
  ch_id = String(ID.substring(0,ID.indexOf(","))).toInt();

#ifdef SERV_DEBUG  
  Serial.print("URL: ");
  Serial.println(URL);
  Serial.print("ID: ");
  Serial.println(ch_id);
#endif

  if (URL.equals("/index.html") || URL.equals("/")) {
    /* index page */
    return page_index(ch_id, URL);
  }
  
  if (URL.indexOf("/reboot.html?") == 0) {
    /* index page */
    return page_reboot(ch_id, URL);
  }

  if (URL.equals("/config.html")) {
    /* index page */
    return page_config(ch_id, URL);
  }

  if (URL.indexOf("/c2ee.html?") == 0) {
    /* index page */
    return page_c2ee(ch_id, URL);
  }

  response_error(ch_id,404);
    
  return 1;
}

int response_head(int ch_id, int length)
{
  String Header;

  Header = "HTTP/1.1 200 OK\r\n";
  Header += "Content-Type: text/html\r\n";
  Header += "Connection: close\r\n";
  Header += "Content-Length: ";
  Header += String(length);
  Header += "\r\n\r\n";

  return send_ipdata(ch_id, Header);
}

int response_send(int ch_id, String Content)
{
  if (response_head(ch_id,Content.length()))
    return 1;
  return send_ipdata(ch_id, Content);
}

int response_error(int ch_id, int error)
{
  String ErrMsg;

  ErrMsg = "HTTP/1.1 ";
  ErrMsg += String(error);
  ErrMsg += " ERROR\r\n";
  ErrMsg += "Content-Type: text/html\r\n";
  ErrMsg += "Connection: close\r\n";
  ErrMsg += "Content-Length:";
  ErrMsg += String(String(error).length()+6);
  ErrMsg += "\r\n\r\n";
  ErrMsg += "Error ";
  ErrMsg += String(error);

  return send_ipdata(ch_id, ErrMsg);
}

/* This is where the web pages go 
*
*/

int page_index(int ch_id, String URL) {
  String Content;
  Content =  "<HTML><BODY>Arduino index.html<hr>default page<p><a href=config.html>Configuration</a><br>";
  Content += "<a href=reboot.html?reboot>reboot</a><br><a href=reboot.html?shutdown>shutdown</a>";
  Content += "<br><a href=reboot.html?default>default settings</a></BODY></HTML>\r\n";
  
  return response_send(ch_id, Content);  
}

int page_reboot(int ch_id, String URL)
{
  String Content;
  Content = "<HTML><BODY>Arduino reboot.html<hr>will shutdown/reboot to selected settings</BODY></HTML>\r\n";
  
  response_send(ch_id, Content);

  if (URL.indexOf("?default") > 0) {
    // Also make EEPROM invalid.
    defaultAP=1;
  } else {
    defaultAP=0;
  }
  
  if (URL.indexOf("?shutdown") > 0) {
    reboot = 0;
  } else {
    reboot = 1;
  }
  
  shutdown=1;
}

int eeprom_unlock = 0;

int page_config(int ch_id, String URL)
{
  String Content;
  Content  = "<HTML><BODY>Arduino config.html<hr><h5><form action=\"c2ee.html\" method=\"get\">SSID:<input name=\"ssid\" value=\"";
  Content += e_SSID;
  Content +="\"><br>Password:<input name=\"pass\" value=\"";
  Content += e_PASS;
  Content += "\"><br><input type=\"checkbox\" name=\"ap\" value=\"on\">Run as AP<br><input type=\"checkbox\" name=\"enc\" value=\"on\">Run AP with encryption";
  Content += "<br>AP on channel:<input name=\"chan\" value=\"";
  Content += e_CHAN;
  Content += "\"><br><button>Save</button></form></BODY></HTML>\r\n";

  eeprom_unlock=1;  
  return response_send(ch_id, Content);
}

int page_c2ee(int ch_id, String URL)
{
  String Params = URL.substring(URL.indexOf("?")+1,URL.length());
  Params += "&";

#ifdef SERV_DEBUG  
  Serial.print("Params:" );
  Serial.println(Params);
#endif

  String r_ssid=Params.substring(Params.indexOf("ssid=")+5,Params.length());
  r_ssid=r_ssid.substring(0,r_ssid.indexOf("&"));

  String r_pass=Params.substring(Params.indexOf("pass=")+5,Params.length());
  r_pass=r_pass.substring(0,r_pass.indexOf("&"));

  String r_chan=Params.substring(Params.indexOf("chan=")+5,Params.length());
  r_chan=r_chan.substring(0,r_chan.indexOf("&"));
  
#ifdef SERV_DEBUG  
  Serial.print("Cred:" );
  Serial.print(r_ssid);
  Serial.print(",");
  Serial.println(r_pass);
#endif

  if (eeprom_unlock)
  {
    Serial.println("Updating EEPROM:" );
    r_ssid.toCharArray(e_SSID,32);
    r_pass.toCharArray(e_PASS,32);
    e_CHAN = r_chan.toInt();
    
    if (Params.indexOf("ap=on") >0 ) {
      e_AP=1;
    } else {
      e_AP=0;
    }

    if (Params.indexOf("enc=on") >0 ) {
      e_ENC=3;
    } else {
      e_ENC=0;
    }
    
    write_eeprom();
  }

  String Content;
  Content  = "<HTML><BODY>SAVE<hr>OK<p><a href=reboot.html?reboot>reboot</a></BODY></HTML>\r\n";
  
  return response_send(ch_id, Content);
}
