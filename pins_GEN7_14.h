/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * Gen7 v1.4 pin assignments
 */

/**
 * Rev B    26 DEC 2016
 *
 * 1) added pointer to a current Arduino IDE extension
 * 2) added support for M3, M4 & M5 spindle control commands
 * 3) added case light pin definition
 *
 */

/**
 * A useable Arduino IDE extension (board manager) can be found at
 * https://github.com/Lauszus/Sanguino
 *
 * This extension has been tested on Arduino 1.6.12 & 1.8.0
 *
 * Here's the JSON path:
 * https://raw.githubusercontent.com/Lauszus/Sanguino/master/package_lauszus_sanguino_index.json
 *
 * When installing select 1.0.2
 *
 * Installation instructions can be found at https://learn.sparkfun.com/pages/CustomBoardsArduino
 * Just use the above JSON URL instead of Sparkfun's JSON.
 *
 * Once installed select the Sanguino board and then select the CPU.
 *
 */

#if !defined(__AVR_ATmega644P__) && !defined(__AVR_ATmega644__) && !defined(__AVR_ATmega1284P__)
  #error "Oops!  Make sure you have 'Sanguino' selected from the 'Tools -> Boards' menu."
#endif

#define BOARD_NAME "Gen7 v1.4"

#define GEN7_VERSION 14 // v1.4

//
// Limit switches
//
#define X_STOP_PIN          0
#define Y_STOP_PIN          1
#define Z_STOP_PIN          2

//
// Steppers
//
#define X_STEP_PIN         29
#define X_DIR_PIN          28
#define X_ENABLE_PIN       25

#define Y_STEP_PIN         27
#define Y_DIR_PIN          26
#define Y_ENABLE_PIN       25

#define Z_STEP_PIN         23
#define Z_DIR_PIN          22
#define Z_ENABLE_PIN       25

#define E0_STEP_PIN        19
#define E0_DIR_PIN         18
#define E0_ENABLE_PIN      25

//
// Temperature Sensors
//
#define TEMP_0_PIN          1   // Analog Input
#define TEMP_BED_PIN        0   // Analog Input

//
// Heaters
//
#define HEATER_0_PIN        4
#define HEATER_BED_PIN      3

//
// Misc. Functions
//
#define PS_ON_PIN          15
#define CASE_LIGHT_PIN     15    // MUST BE HARDWARE PWM

// A pin for debugging
#define DEBUG_PIN           0

// RS485 pins
#define TX_ENABLE_PIN      12
#define RX_ENABLE_PIN      13

//
// M3/M4/M5 - Spindle/Laser Control
//
#define SPINDLE_LASER_ENABLE_PIN 20  // Pin should have a pullup/pulldown!
#define SPINDLE_LASER_PWM_PIN    16  // MUST BE HARDWARE PWM
#define SPINDLE_DIR_PIN          21
