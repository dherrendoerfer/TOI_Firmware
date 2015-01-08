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
 
//#define SER_DEBUG 1

int expect(char* expectstr, int timeout) 
{
  Serial1.setTimeout(timeout); 
  
  if(Serial1.find(expectstr)){
#ifdef SER_DEBUG
    Serial.print("RECEIVED: ");
    Serial.println(expectstr);
#endif
    return 0;
  } 

#ifdef SER_DEBUG
  Serial.print("ERROR DID NOT RECEIVE: ");
  Serial.println(expectstr);
#endif
  return 1;
}

int send_expect(char* sendstr, char* expectstr, int timeout) 
{
  Serial1.println(sendstr);
#ifdef SER_DEBUG
  Serial.print("Sent: ");
  Serial.println(sendstr);  
#endif
  
  return expect(expectstr,timeout);
}

int send_expect(String sendstr, char* expectstr, int timeout) 
{
  Serial1.println(sendstr);
#ifdef SER_DEBUG
  Serial.print("Sent: ");
  Serial.println(sendstr);  
#endif
  
  return expect(expectstr, timeout);
}

void send(char* sendstr) 
{
  Serial1.print(sendstr);
#ifdef SER_DEBUG
  Serial.print("Sent: ");
  Serial.println(sendstr);  
#endif
}

void send(int val) 
{
  char sendstr[6];
  sprintf(sendstr,"%i",val);
  Serial1.print(sendstr);
#ifdef SER_DEBUG
  Serial.print("Sent: ");
  Serial.println(sendstr);  
#endif
}


void serial_flush()
{
  while(Serial1.available()>0) {
    Serial1.read();
    delay(10);
  }
}

int send_dump(char* sendstr)
{
  char tmp;
  
  Serial1.println(sendstr);
  Serial.println(F("--------------- DUMP ---------------"));
  Serial.print(F("Sent: "));
  Serial.println(sendstr);

  while (tmp != '\n') {
    if (Serial1.available()>0) {
      tmp=Serial1.read();
      Serial.write(tmp);
    }
  }
  tmp=0;
  while (tmp != '\n') {
    if (Serial1.available()>0) {
      tmp=Serial1.read();
      Serial.write(tmp);
    }
  }
  tmp=0;
  while (tmp != '\n') {
    if (Serial1.available()>0) {
      tmp=Serial1.read();
      Serial.write(tmp);
    }
  }
  Serial.println(F("--------------- DUMP ---------------"));
  serial_flush();
}

int serial_read(char* buffer, int offset, int length)
{
  int count=0;
  
//  if (Serial1.available() == 0)
//    return 0;
  
  while(Serial1.available() > 0) {
    buffer[offset]=Serial1.read();
//    Serial.write(buffer[offset]);
    buffer[offset+1]=0; // HACK !!!
    count++;
    /* Kind of a neat hack to keep mem consumption low:
     * bail out at the end of each line, because we may
     * safely ignore a lot of things                     */
    if (buffer[offset] == '\n')
      break;
    offset++;
    if (offset == length)
      break;
  }

//  if ( count ) {  
//    Serial.println();
//    Serial.print("At:" );
//    Serial.println(count);
//  }
  return count;
}
