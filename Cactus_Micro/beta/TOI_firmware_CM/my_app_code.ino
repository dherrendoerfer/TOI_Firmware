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

#define strlenPGM(str) (sizeof(str)-1)

/* My-App specific init */
void app_init()
{

}

/* My-App specific loop code 
   Note: keep this code fast, do not block */
void tick()
{

}

/* My-App specific timed code 
   Note: keep this code fast, do not block */
void second_tick()
{

}

/* My-App specific timeout code 
   Note: keep this code fast, do not block */
void timeout_event()
{

}

/* My-App specific shutdown code 
   Note: a reboot or shutdown is in progress */
void shutdown_event()
{

}

/* My-App specific eeprom code 
   Note: an eeprom-write was done */
void eeprom_event()
{

}

/* My-App specific URL handlers
   Put your web-pages in here. */
int myAppHandleURL(char* URL, int ch_id)
{
  if (strcmp(URL,"/index.html") == 0 || strcmp(URL,"/") == 0) {
    /* index page */
    page_index(ch_id, URL);
    return 0;
  }

  return 1;
}


/*********************************************************************************************** 
*  index.html 
*/
prog_char content_index[] PROGMEM =  "<HTML><BODY>index.html<hr>the default page<p>"
                          "</BODY></HTML>\r\n";
  
void page_index(int ch_id, char* URL) {
  response_send_progmem(ch_id, content_index, strlenPGM(content_index));  
}
