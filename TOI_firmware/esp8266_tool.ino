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

//#define ESP_DEBUG 1

int connectWiFi(char* ssid, char* pass)
{
  if(send_expect("AT+CWMODE=1","OK\r\n",1000)){
#ifdef ESP_DEBUG
    Serial.println("Initial client mode setting error ignored");
#endif
  }
 
  send("AT+CWJAP=\"");
  send(ssid);
  send("\",\"");
  send(pass);
  send("\"");
  send("\r\n");

  return expect("OK\r\n",20000);
}

int connectAP(char* ssid, char* pass, int chan, int enc)
{
  if(send_expect("AT+CWMODE=2","OK\r\n",1000)){
#ifdef ESP_DEBUG
    Serial.println("Initial AP mode setting error ignored");
#endif
  }
 
  send("AT+CWSAP=\"");
  send(ssid);
  send("\",\"");
  send(pass);
  send("\",");
  send(chan);
  send(",");
  send(enc);
  send("\r\n");
  
  return expect("OK\r\n",20000);
}

int setup_server(int port)
{
  send("AT+CIPSERVER=1,");
  send(port);
  send("\r\n");

  return expect("OK\r\n",1000);
}

int connect_host(int ch_id, char* host, int port)
{
  send("AT+CIPSTART=");
  send(ch_id);
  send(",\"TCP\",\"");
  send(host);
  send("\",");
  send(port);
  send("\r\n");

  return expect("OK\r\n",3000);
}

int close_channel(int ch_id)
{
  send("AT+CIPCLOSE=");
  send(ch_id);
  send("\r\n");

  return expect("OK\r\n",1000);
}

int stop_server(int port)
{
  send("AT+CIPSERVER=0,");
  send(port);
  send("\r\n");

  return expect("OK\r\n",1000);
}

int set_multicon()
{
  return send_expect("AT+CIPMUX=1","OK\r\n",1000);
}

int unset_multicon()
{
  return send_expect("AT+ CIPMUX=0","OK\r\n",1000);
} 

int send_ipdata_head(int ch_id,char* data, int length)
{
  send("AT+CIPSEND=");
  send(ch_id);
  send(",");
  send(length);
  send("\r\n");

  if ( !expect(">",1000) ) {
    send(data);
    return 0;
  }
  return 1;
}

void send_ipdata(char* data )
{
  send(data);
}

void send_ipdata(int data )
{
  send(data);
}

int send_ipdata_fin(char* data)
{
  return send_expect(data,"OK\r\n",5000);
}

int http_req_get(char* host,   char* URL, char* resp_buffer, int buffer_length)
{
  //open connection
  
  //send request
  
  //wait for response
  
  //receive return document
  
  return 0;
}

int buffer_position = 0;
int timeout_count = 0;

int esp_poll()
{
  /* While polling we assume to be talking to an ESP8266, so we
   * can make the assumption that each data chunk ends with an added OK 
   * and other such things. */
  
  int read = 0; 
  read = serial_read(input_buffer, buffer_position, BUFFER_SIZE-buffer_position-1);
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
  //String ip_data=String(input_buffer);
  
  /* Only get is supported, so we can stop reading the 
   * input buffer once the complete get request is read */
 
  if (strstr(input_buffer,"Link") && buffer_position == 6) {
    input_buffer[0]=0;
    buffer_position = 0;
    timeout_count = 0;
  }

  if (strstr(input_buffer,"Unlink") && buffer_position == 8) {
    input_buffer[0]=0;
    buffer_position = 0;
    timeout_count = 0;
  }
   
  if (strstr(input_buffer,"+IPD")) {
#ifdef ESP_DEBUG
    Serial.println("IPD found:");
#endif
    if (strstr(input_buffer,"GET")) {
#ifdef ESP_DEBUG
      Serial.println("GET found:");
#endif
      if (strstr(input_buffer,"HTTP")) {
#ifdef ESP_DEBUG
        Serial.println("HTTP found:");
#endif

        /* We got what we need for now, 
         * eat everything up to the next OK. */
        if (strstr(input_buffer,"OK")) {
#ifdef ESP_DEBUG
          Serial.println("OK found:");
#endif
        } else {
          expect("OK\r\n",200);
        }
        
        /* Handle the request */
        request_get(input_buffer, buffer_position);
        
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
