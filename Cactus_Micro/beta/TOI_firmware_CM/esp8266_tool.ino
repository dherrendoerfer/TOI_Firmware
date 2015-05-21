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

/* Free SRAM memory helper function.
 * This gives an approriate value of leftover free memory.
 */
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


int connectWiFi(char* ssid, char* pass)
{
  if(send_expect("AT+CWMODE=1","OK\r\n",1000)){
#ifdef ESP_DEBUG
    Serial.println(F("Initial client mode setting error ignored"));
#endif
  }
 
  send("AT+CWJAP=\"");
  send(ssid);
  send("\",\"");
  log_mute=1;
  send(pass);
  send("\"");
  send("\r\n");

  return expect("OK\r\n",20000);
}

int connectAP(char* ssid, char* pass, int chan, int enc)
{
  if(send_expect("AT+CWMODE=2","OK\r\n",1000)){
#ifdef ESP_DEBUG
    Serial.println(F("Initial AP mode setting error ignored"));
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

int udp_open_host(int ch_id, char* host, int port)
{
  send("AT+CIPSTART=");
  send(ch_id);
  send(",\"UDP\",\"");
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
    if (data)
      send(data);
    return 0;
  }
  return 1;
}

void send_ipdata(char* data )
{
  send(data);
}

void send_ipdata(char* data ,int length)
{
  send(data,length);
}

void send_ipdata(int data )
{
  send(data);
}

int send_ipdata_fin()
{
  return expect("SEND OK\r\n",5000);
}

int get_ip(char* ip, int len)
{
  if (send_expect("AT+CIFSR","STAIP,\"",1000))
    return 1; 

  if (!find("\"",1000,ip,len))
    return 1; 
  
  // strip the tailing "
  if (strstr(ip,"\""))
    *(strstr(ip,"\"")) = 0;
  
#ifdef ESP_DEBUG
  Serial.print("IP:");
  Serial.println(ip);
#endif

  return 0;
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
//  if (!find("Linked\r\n",5000,resp_buffer,buffer_length))
//    return 1;
  
  // send request
  send_ipdata_head(ch_id, "GET " , strlen(URL)+36);
  send_ipdata(URL);
  send_ipdata(" HTTP/1.0\r\n");
  send_ipdata("Connection: close\r\n\r\n");
  send_ipdata_fin();
  
  // wait for response IP data
  log_mute = 1;  
  if (expect("+IPD,",5000))
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
  if (!find("2,CLOSED\r\n",2000,resp_buffer,buffer_length))
    goto err_out;
  
  // strip of a trailing OK line from lighttpd i.e.
  *(strstr(resp_buffer,"OK\r\n")-2) = 0;

  // wait until the socket was closed.  
//  expect("Unlink\r\n",3000);
  
  return 0;
  
  // error out: eat up eventual trailing garbage.
  err_out:
  find("OK\r\n",500,0,0);
  expect("2,CLOSED\r\n",2000);
  return 1;
}

int ntp_time_get(char* hostip)
{
  int ch_id=3;
  int port=123;
  
  char req[48] = { 010,0,0,0,0,0,0,0,0 };
  
#ifdef ESP_DEBUG
  Serial.println(F("Setting time via NTP:"));
#endif

  // open connection
  if (udp_open_host(ch_id, hostip, port))
    return 1;

  send_ipdata_head(ch_id, NULL, 48);
  send_ipdata(req,48);
  send_ipdata_fin();

  if (num_pkt == 1 && pktlen[0] == 49)
  {
    // If the packet was caught by the auto parser
    memcpy(req,pkt[0],48);
    delete_packet(0); 
  } else {
    // wait for response IP data manually
    if (expect("+IPD,",2000))
      return 1;

    if (expect("3,48:",500))
      return 1;
    
    if (raw_read(req, 0, 48, 500) != 48)
      return 1;
  }

  find("OK\r\n",1000,0,0);
  close_channel(ch_id);
  find("3,CLOSED\r\n",2000,0,0);
      
#ifdef ESP_DEBUG
  Serial.println(F("Got:"));
#endif

  unsigned long secsSince1900 =  (unsigned long)req[43];
                secsSince1900 += ((unsigned long)req[42] << 8 )   & 0xff00;
                secsSince1900 += ((unsigned long)req[41] << 16 )  & 0xff0000;
                secsSince1900 += ((unsigned long)req[40] << 24 )  & 0xff000000;
                
  /* Now convert NTP time into everyday time.
   * Unix time starts on Jan 1 1970.
   * In seconds, that's 2208988800.
   * Subtract seventy years. */
  unsigned long epoch = secsSince1900 - 2208988800UL;
  
  /* Calculate the day, hour, minute and second.
   * UTC is the time at Greenwich Meridian (GMT). */
  
  /* Apply local timezone adjustment */
  epoch += e_TZ * 3600;
  
  days = ((epoch - 86400L*4)  % 604800L) / 86400L; // day (604800 equals secs per week)
  hours = (epoch  % 86400L) / 3600;                // hour (86400 equals secs per day)
  minutes = (epoch  % 3600) / 60;                  // minute (3600 equals secs per minute)
  seconds = epoch % 60;                            // second

#ifdef ESP_DEBUG
  Serial.println(F("Seconds since Jan 1 1900 = "));
  Serial.println(secsSince1900);
  Serial.println(F("Unix time = "));
  Serial.println(epoch);
  Serial.println(F("The local time is "));
  Serial.println(days);
  Serial.println(hours);   
  Serial.println(minutes);       
  Serial.println(seconds);                
#endif

  return 0;
}


/* New parser, for combined receive buffer 
 *  Parser reads an IPD packet either as a whole,
 *  (binary) or Strips relevant information and
 *  records only a http request for example.
 */
int ipd_parse()
{
  char tmpbuf[6];
  int ch_id;
  int length;
  int rread;

  if (num_pkt == 4) {
    goto err_out; //Bail out if packet buffer is full
  }

  log_mute=1;
  if (!find(":",100,tmpbuf,6))
    goto err_out;
    
    Serial.println(tmpbuf);

  ch_id = tmpbuf[0] - '0';

  length = tmpbuf[2] - '0';
  if (tmpbuf[3] != ':') {
    length *= 10;
    length += tmpbuf[3] - '0';
    if (tmpbuf[4] != ':') {
      length *= 10;
      length += tmpbuf[4] - '0';
    }
  }

  if (length > M_BUFFER_SIZE - input_buffer_ptr) {
    goto err_out; //Bail out if packet is too large for buffer
  }

  rread  = raw_read(input_buffer, input_buffer_ptr, length, 5000);

#ifdef ESP_DEBUG
  Serial.println(F("ipd_parse():"));
#endif
#ifdef ESP_DEBUG
  Serial.print(F("l:"));
  Serial.println(length);
#endif
#ifdef ESP_DEBUG
  Serial.print(F("r:"));
  Serial.println(rread);
#endif

  if (rread != length) {
    goto err_out; // Reading the packet failed
  }
  
  /*A packet was successfully read. Store and record. */
  input_buffer[input_buffer_ptr+length] = 0;
  length++;
  pkt[num_pkt] = input_buffer + input_buffer_ptr;
  pktlen[num_pkt] = length;
  pktchan_id[num_pkt] = ch_id;
  input_buffer_ptr += length;

  num_pkt++;

#ifdef ESP_DEBUG
  Serial.print(F("packet:"));
  Serial.println(num_pkt);
#endif
#ifdef ESP_DEBUG
  Serial.println(F("---------- pkt: ----------"));
  Serial.println(pkt[num_pkt -1]);
  Serial.println(F("--------------------------"));
#endif

  return 0;  
err_out:
  find("OK\r\n",2000,0,0);
  return 1;
}

void delete_packet(int num)
{
  char* p_start = pkt[num];
  char* p_end   = pkt[num]+pktlen[num];
  int   p_len   = pktlen[num];
  int   i = 0;
  
  if (num_pkt == 1 && num == 0) {
    num_pkt = 0;
    input_buffer_ptr = 0;
    return;
  }

  memcpy(p_start,p_end,M_BUFFER_SIZE-p_len);

  for (i=num ; i < num_pkt - 1; i++) {
    pkt[i] = pkt[i+1]-p_len;
    pktlen[i] = pktlen[i+1];
    pktchan_id[i] = pktchan_id[i+1];
  }

  pkt[i] = 0;
  pktlen[i] = 0;
  pktchan_id[i] = 0;

  input_buffer_ptr -= p_len;
  num_pkt--;
}

char tmpbuf[30];

static int esp_poll()
{
  tmpbuf[0]=0;
  
  /* While polling we assume to be talking to an ESP8266, so we
   * can make the assumption that each data chunk ends with an added OK 
   * and other such things. 
   * In the polling loop we just fish for some data, until a data packet
   * becomes available */

//  log_mute=1;
  find("\r\n",100, tmpbuf, sizeof(tmpbuf));

  if (num_pkt == 0)
    return 1;
  
  /* Only GET is supported, so we can stop reading the 
   * input buffer once the complete get request is read */

  // Select the 1st packet and try to analyze the protocol

  if (strstr(pkt[0],"GET")) {
#ifdef ESP_DEBUG
    Serial.println(F("GET found:"));
#endif
        
    /* Handle the request */
    request_get(pkt[0], pktlen[0],pktchan_id[0]);
    
    delete_packet(0);  
    return 0;
  }  
     
  return 1;
}
