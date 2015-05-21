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

/* Standard Arduino board with one Serial port
 *
 * Arduino Uno, Mini, Nano, 
 */
#ifdef BOARD_ARDUINO_STD

/* Software Serial Configuration */
#define M_SOFT_SERIAL 1
#define M_SOFT_TX 11
#define M_SOFT_RX 10

/* External Connections for indicators and inputs */
#define M_HEARTBEAT_LED 13
#define M_RESET_PIN     12

/* Input Buffer size for the web server */
#define M_BUFFER_SIZE 160

#endif


/* APRBROTHER Cactus board with one Serial port
 *
 */
#ifdef BOARD_ARDUINO_CACTUS

/* Software Serial Configuration */
//#define M_SOFT_SERIAL 1
#define M_SOFT_TX 12
#define M_SOFT_RX 11

#endif


