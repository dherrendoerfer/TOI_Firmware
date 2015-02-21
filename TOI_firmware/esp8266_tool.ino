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

  return !find("OK\r\n",1000,0,0);
}

int stop_server(int port)
{
  send("AT+CIPSERVER=0,");
  send(port);
  send("\r\n");

  return expect("OK\r\n",1000);
}

int get_ip(char* ip, int len)
{
  if (!send_expect_read("AT+CIFSR","OK\r\n",1000,ip,len))
    return 1; 
  
  if (strstr(ip,"\r\n"))
    *(strstr(ip,"\r\n")) = 0;
  
#ifdef ESP_DEBUG
  Serial.print("IP:");
  Serial.println(ip);
#endif

  return 0;
}

int reset()
{
  return send_expect("AT+RST","ready\r\n",2000);
}

int set_echo()
{
  return send_expect("ATE1","OK\r\n",1000);
}

int unset_echo()
{
  return send_expect("ATE0","OK\r\n",1000);
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

int send_ipdata_fin()
{
  return expect("SEND OK\r\n",5000);
}

/* Note: This function is beta, it may not perform well with all
*        available web servers. 
*/
int http_req_get(char* host, int port, char* URL, char* resp_buffer, int buffer_length)
{
  int ch_id=2;
  
  // open connection
  if (connect_host(ch_id, host, port))
    return 1;  
  // wait until the connection is established
  if (!find("Linked\r\n",5000,resp_buffer,buffer_length))
    return 1;
  
  // send request
  send_ipdata_head(ch_id, "GET " , strlen(URL)+17);
  send_ipdata(URL);
  send_ipdata(" HTTP/1.0\r\n\r\n");
  send_ipdata_fin();
  
  // wait for response IP data
  log_mute = 1;  
  if (expect("+IPD",5000))
    return 1;

  // capture the : after IPD
  log_mute = 1;  
  find(":",500, 0, 0);

  // capture the 1st response line
  log_mute = 1;  
  if (!find("\r\n",500,resp_buffer,buffer_length))
    goto err_out;
  
  // bail out if no response code 200 is found
  if (!strstr(resp_buffer,"200"))
    goto err_out;
  
  // ignore everything up to the body separator  
  log_mute = 1;  
  if (expect("\r\n\r\n",500))
    goto err_out;
    
  // capture the response body  
  if (!find("OK\r\n",2000,resp_buffer,buffer_length))
    goto err_out;
  
  // strip of a trailing OK line from lighttpd i.e.
  *(strstr(resp_buffer,"OK\r\n")-2) = 0;

  // wait until the socket was closed.  
  expect("Unlink\r\n",3000);
  
  return 0;
  
  // error out: eat up eventual trailing garbage.
  err_out:
  find("OK\r\n",1000,0,0);
  expect("Unlink\r\n",2000);
  return 1;
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
