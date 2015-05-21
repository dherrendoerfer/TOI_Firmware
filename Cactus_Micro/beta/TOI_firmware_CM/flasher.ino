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
 
/* Passthrough mode flasher to make flashing of the ESP8266 possible */

//#define FLASH_DEBUG 1

void do_flasher() 
{
#ifdef FLASH_DEBUG
  while (!Serial) {
    ;
  }
#endif

#ifdef M_ESP_ENABLE     
  pinMode(M_ESP_FLASH, OUTPUT);
  pinMode(M_ESP_ENABLE, OUTPUT);

  digitalWrite(M_ESP_ENABLE,0);
  delay(500);
  digitalWrite(M_ESP_FLASH,0);
  delay(200);
  digitalWrite(M_ESP_ENABLE,1);
  delay(500);
#endif

Serial.println("ESP8266 programmer ready.");

  while (1) {
      while (SerialESP.available())
        Serial.write(SerialESP.read());
      while (Serial.available())
        SerialESP.write(Serial.read());
  }
}

