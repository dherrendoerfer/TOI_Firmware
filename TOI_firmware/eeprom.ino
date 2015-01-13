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

#include <EEPROM.h>

int check_eeprom()
{
  if ( EEPROM.read(0) == 42 && EEPROM.read(1) == 42) {
    return 1;
  }
  return 0;
}

int invalidate_eeprom()
{
  EEPROM.write(1,0);
  
  return 0;
}

void read_eeprom()
{
  int byte = 2; // We start reading at byte 2 of eeprom
  int i; 
  
  for(i=0; i<32 ;i++) {
    e_SSID[i] = EEPROM.read(byte++);
  }

  for(i=0; i<32 ;i++) {
    e_PASS[i] = EEPROM.read(byte++);
  }

  e_AP   = EEPROM.read(byte++);
  e_ENC  = EEPROM.read(byte++);
  e_CHAN = EEPROM.read(byte++);

  t_usPerSec  = EEPROM.read(byte++);
  t_usPerSec += (EEPROM.read(byte++) << 8) & 0xff00;
  t_usPerSec += (long)(EEPROM.read(byte++) << 16) & 0xff0000;
  t_usPerSec += (long)(EEPROM.read(byte++) << 24) & 0xff000000;
  
  Serial.println(t_usPerSec);

}

void write_eeprom()
{
  int byte = 2; // We start reading at byte 2 of eeprom
  int i; 
  
  for(i=0; i<32 ;i++) {
     EEPROM.write(byte++,e_SSID[i]);
  }

  for(i=0; i<32 ;i++) {
    EEPROM.write(byte++,e_PASS[i]);
  }

  EEPROM.write(byte++,e_AP & 0xff);
  EEPROM.write(byte++,e_ENC & 0xff);
  EEPROM.write(byte++,e_CHAN & 0xff);
  
  EEPROM.write(byte++,t_usPerSec & 0xff);
  EEPROM.write(byte++,(t_usPerSec >>  8) & 0xff);
  EEPROM.write(byte++,(t_usPerSec >> 16) & 0xff);
  EEPROM.write(byte++,(t_usPerSec >> 24) & 0xff);
  
  EEPROM.write(0,42);
  EEPROM.write(1,42);
}

