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

/*Uncomment this to see all serial traffic*/ 
#define SER_DEBUG 1
/*Uncomment this to see serial errors*/ 
//#define SER_ERROR 1

/* IP Data search string, for automatic packet reception */
int ip = 0;
int iptr = 0;
char ipd[] = "+IPD,";

int event_ipd = 0;

int _find(char* str, unsigned long timeout, char* buffer, int buflen)
{
  unsigned long end;
  int sp = 0;
  int splen = strlen(str);
  int bufptr = 0; 
  char tmp;
  
#ifdef SER_DEBUG
      if (!log_mute) {        
        Serial.print(F("Find \""));
        Serial.print(str);
        Serial.print(F("\" len:"));
        Serial.print(splen);
        Serial.print(F(" buffer:@"));
        Serial.print((int)buffer);
        Serial.print(F(" buflen:"));
        Serial.println(buflen);
      }
#endif      

  end = millis() + timeout;

  while ( (long)(millis() - end ) < 0) {
    if (SerialESP.available() > 0){
      tmp = SerialESP.read();
      
      if (buffer) {
        buffer[bufptr++] = tmp;
        if (bufptr != buflen ) {
          buffer[bufptr] = 0;
        }
      }

#ifdef SER_DEBUG
      if (!log_mute)
        Serial.print(tmp);
#endif      
      if (str[sp] == tmp){
        sp++;
        if (sp == splen){
          log_mute = 0;
          // found the search string
          next_timeout = utime + 30000*t_usPerSec;
          return 1;
        }
      } else {
        sp=0;
      }
      
      if (ipd[ip] == tmp){
        ip++;
        if (ip == 5){
          // raise an ipd event
          event_ipd = 1;
          return 0;
          ip=0;
        }
      } else {
        ip=0;
      }

      if (buffer && bufptr == buflen ) {
        return 0; // buffer full
      }
    }
  }
#ifdef SER_DEBUG
  Serial.print(F("\r\nTimeout\r\n"));
#endif      
  log_mute = 0;
  return 0;
}

int find(char* str, unsigned long timeout, char* buffer, int buflen)
{
  int ret;
  try_again:
  ret = _find( str, timeout, buffer, buflen);
  
  if (ret)
    return ret;
    
  if (event_ipd) {
    ipd_parse();
    event_ipd = 0;
    ip=0;
    goto try_again;
  }
  else 
    return ret;
}

int expect(char* expectstr, unsigned int timeout) 
{
  if(find(expectstr, timeout, 0, 0)){
#ifdef SER_DEBUG
    Serial.print(F("RECEIVED: "));
    Serial.println(expectstr);
#endif
    return 0;
  } 

#if defined SER_DEBUG || SER_ERROR
  Serial.print(F("ERROR DID NOT RECEIVE: "));
  Serial.println(expectstr);
#endif

  /*Check if the ESP8266 is stuck */
  SerialESP.print("AT\r\n");
  if (!find("OK\r\n",2000,0,0)){
#if defined SER_DEBUG || SER_ERROR
    Serial.println(F("ERROR, ESP STUCK: "));
#endif
    SerialESP.print("\r\nAT\r\n");
    if (!find("OK\r\n",2000,0,0)){
#if defined SER_DEBUG || SER_ERROR
      Serial.println(F("ERROR, ESP still STUCK: "));
#endif
    } else {
      reinit = 1;
#if defined SER_DEBUG || SER_ERROR
      Serial.println(F("RECOVER, ESP DID RESPOND. "));
#endif
    }
  }
  return 1;
}

int send_expect(char* sendstr, char* expectstr, int timeout) 
{
  SerialESP.println(sendstr);
#ifdef SER_DEBUG
  Serial.print(F("Sent: "));
  Serial.println(sendstr);  
#endif
  
  return expect(expectstr,timeout);
}

int send_expect(String sendstr, char* expectstr, int timeout) 
{
  SerialESP.println(sendstr);
#ifdef SER_DEBUG
  Serial.print(F("Sent: "));
  Serial.println(sendstr);  
#endif
  
  return expect(expectstr, timeout);
}

int send_expect_read(char* sendstr, char* expectstr, int timeout, char* buffer, int buflen) 
{
  SerialESP.println(sendstr);
#ifdef SER_DEBUG
  Serial.print(F("Sent: "));
  Serial.println(sendstr);  
  Serial.println(F("Read<-"));
#endif
  
  return find(expectstr,timeout,buffer,buflen);
}

void send(char sendstr) 
{
  SerialESP.write(sendstr);
#ifdef SER_DEBUG
  if (!log_mute) {
    Serial.print(F("Sent: "));
    Serial.println(sendstr);
    log_mute = 0;
  }  
#endif
}

void send(char* sendstr) 
{
  SerialESP.print(sendstr);
#ifdef SER_DEBUG
  if (!log_mute) {
    Serial.print(F("Sent: "));
    Serial.println(sendstr);
    log_mute = 0;
  }  
#endif
}

void send(char* sendstr, int length) 
{
  int ret;
  ret = SerialESP.write((uint8_t*)sendstr, length);
#ifdef SER_DEBUG
  Serial.print(F("Sent "));
  Serial.print(ret);
  Serial.println(F(" bytes"));  
#endif
}

void send(int val) 
{
  char sendstr[6];
  sprintf(sendstr,"%i",val);
  SerialESP.print(sendstr);
#ifdef SER_DEBUG
  Serial.print(F("Sent: "));
  Serial.println(sendstr);  
#endif
}

void send_progmem_data(PGM_P content, int length) 
{
  int i;
  char tmp; 
  
  for (i=0; i<length; i++ ) {
    tmp =  pgm_read_byte(content++);
    log_mute=1;
    send(tmp);
    Serial.print(tmp);
    serial_flush();
  }
}

void serial_flush()
{
  SerialESP.flush();
  while(SerialESP.available()>0) {
    SerialESP.read();
    delay(5);
  }
}

int send_dump(char* sendstr)
{
  char tmp;
  
  SerialESP.println(sendstr);
  Serial.println(F("--------------- DUMP ---------------"));
  Serial.print(F("Sent: "));
  Serial.println(sendstr);

  while (tmp != '\n') {
    if (SerialESP.available()>0) {
      tmp=SerialESP.read();
      Serial.write(tmp);
    }
  }
  tmp=0;
  while (tmp != '\n') {
    if (SerialESP.available()>0) {
      tmp=SerialESP.read();
      Serial.write(tmp);
    }
  }
  tmp=0;
  while (tmp != '\n') {
    if (SerialESP.available()>0) {
      tmp=SerialESP.read();
      Serial.write(tmp);
    }
  }
  Serial.println(F("--------------- DUMP ---------------"));
  serial_flush();
}

int raw_read(char* buffer, int offset, int length, int timeout)
{
  unsigned long end = millis() + timeout;
  int count=0;
  while ( (long)(millis() - end ) < 0) {
    if(SerialESP.available() > 0) {
      buffer[offset++]=SerialESP.read();
      count++;
      if (count == length)
        return count;
    }
  }
  // Timeout
  return count; 
}

