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

//#define ESP_DEBUG 1

int connectWiFi(char* ssid, char* pass)
{
  if(send_expect("AT+CWMODE=1","OK\r\n",1000)){
#ifdef ESP_DEBUG
    Serial.println("Initial client mode setting error ignored");
#endif
  }
 
  String cmd="AT+CWJAP=\"";
  cmd+=ssid;
  cmd+="\",\"";
  cmd+=pass;
  cmd+="\"";
  
  return send_expect(cmd,"OK\r\n",20000);
}

int connectAP(char* ssid, char* pass, int chan, int enc)
{
  if(send_expect("AT+CWMODE=2","OK\r\n",1000)){
#ifdef ESP_DEBUG
    Serial.println("Initial AP mode setting error ignored");
#endif
  }
 
  String cmd="AT+CWSAP=\"";
  cmd+=ssid;
  cmd+="\",\"";
  cmd+=pass;
  cmd+="\",";
  cmd+=String(chan);
  cmd+=",";
  cmd+=String(enc);
  
  return send_expect(cmd,"OK\r\n",20000);
}

int setup_server(int port)
{
  String cmd = "AT+CIPSERVER=1,";
         cmd += String(port);

  return send_expect(cmd,"OK\r\n",1000);
}

int stop_server(int port)
{
  String cmd = "AT+CIPSERVER=0,";
         cmd += String(port);

  return send_expect(cmd,"OK\r\n",1000);
}

int set_multicon()
{
  return send_expect("AT+CIPMUX=1","OK\r\n",1000);
}

int unset_multicon()
{
  return send_expect("AT+ CIPMUX=0","OK\r\n",1000);
} 

int send_ipdata(int ch_id,String data)
{
  String cmd="AT+CIPSEND=";
  cmd+=String(ch_id);
  cmd+=",";
  cmd+=String(data.length());

  if(send_expect(cmd,">",1000))
    return 1;
  return send_expect(data,"OK\r\n",5000);
}

int close_channel(int ch_id)
{
  String cmd="AT+CIPCLOSE=";
  cmd+=String(ch_id);

  return send_expect(cmd,"OK\r\n",1000);
}

int buffer_position = 0;
int timeout_count = 0;

int esp_poll()
{
  /* While polling we assume to be talking to an ESP8266, so we
   * can make the assumption that each data chunk ends with an added OK 
   * and other such things. */
  
  int read = 0; 
  read = serial_read(input_buffer, buffer_position, BUFFER_SIZE-buffer_position);
  buffer_position+=read;
  
  if (read)
    timeout_count = 0;

  if (buffer_position==0) {
    return 1;
  }

#ifdef ESP_DEBUG
   Serial.print(input_buffer);
   Serial.print(",");
   Serial.print(buffer_position);
   Serial.println();
#endif

  /* Receive IP data */
  String ip_data=String(input_buffer);
  
  /* Only get is supported, so we can stop reading the 
   * inpu buffer once the complete get request is read */
   
  if (ip_data.indexOf("+IPD") >= 0) {
#ifdef ESP_DEBUG
    Serial.println("IPD found:");
#endif
    if (ip_data.indexOf("GET") >= 0) {
#ifdef ESP_DEBUG
      Serial.println("GET found:");
#endif
      if (ip_data.indexOf("HTTP") >= 0) {
#ifdef ESP_DEBUG
        Serial.println("HTTP found:");
#endif

        /* We got what we need for now, 
         * eat everything up to the next OK. */
        if (ip_data.indexOf("OK") >= 0) {
#ifdef ESP_DEBUG
          Serial.println("OK found:");
#endif
        } else {
          expect("OK\r\n",200);
        }
        
        /* Handle the request */
        request_get(ip_data);
        
        input_buffer[0]=0;
        buffer_position = 0;
        timeout_count = 0;
        return 0;
      }
    }
  }  
  
  timeout_count++;
  if (timeout_count == 300)
  {
#ifdef ESP_DEBUG
    Serial.println("timeout, cleared buffer.");
#endif
    input_buffer[0]=0;
    buffer_position = 0;
    timeout_count = 0;
  }
   
  return 1;
}
