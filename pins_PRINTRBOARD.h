/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2017 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
 *  Rev B  2 JUN 2017
 *
 *  Converted to Arduino pin numbering
 */

/**
 *  There are two Arduino IDE extensions that are compatible with this board
 *  and with the mainstream Marlin software.
 *
 *  Teensyduino - http://www.pjrc.com/teensy/teensyduino.html
 *    Select Teensy++ 2.0 in Arduino IDE from the 'Tools -> Boards' menu
 *
 *    Installation instructions are at the above URL.  Don't bother loading the
 *    libraries - they are not used with the Marlin software.
 *
 *  Printrboard - https://github.com/scwimbush/Printrboard-HID-Arduino-IDE-Support
 *
 *    Installation:
 *
 *       1. Go to the above URL, click on the "Clone or Download" button and then
 *          click on "Download ZIP" button.
 *       2. Unzip the file, find the "printrboard" directory and then copy it to the
 *          hardware directory in Arduino.  The Arduino hardware directory will probably
 *          be located in a path similar to this: C:\Program Files (x86)\Arduino\hardware.
 *       3. Restart Arduino.
 *       4. Select "Printrboard" from the 'Tools -> Boards' menu.
 *
 *  Teensyduino is the most popular option. Printrboard is used if your board doesn't have
 *  the Teensyduino bootloader on it.
 */

/**
 *  To burn the bootloader that comes with Printrboard:
 *
 *   1. Connect your programmer to the board.
 *   2. In the Arduino IDE select "Printrboard" and then select the programmer.
 *   3. In the Arduino IDE click on "burn bootloader". Don't worry about the "verify failed at 1F000" error message.
 *   4. The programmer is no longer needed. Remove it.
 */

#ifndef __AVR_AT90USB1286__
  #error "Oops!  Make sure you have 'Teensy++ 2.0' or 'Printrboard' selected from the 'Tools -> Boards' menu."
#endif

#define BOARD_NAME         "Printrboard"

#define LARGE_FLASH        true

// Disable JTAG pins so they can be used for the Extrudrboard
#define DISABLE_JTAG       true

//
// Limit Switches
//
#define X_STOP_PIN         47   // E3
#if ENABLED(SDSUPPORT)
  #define Y_STOP_PIN       37   // E5 - Move Ystop to Estop socket
#else
  #define Y_STOP_PIN       20   // B0 SS - Ystop in Ystop socket
#endif
#define Z_STOP_PIN         36   // E4

//
// Steppers
//
#define X_STEP_PIN         28   // A0
#define X_DIR_PIN          29   // A1
#define X_ENABLE_PIN       19   // E7

#define Y_STEP_PIN         30   // A2
#define Y_DIR_PIN          31   // A3
#define Y_ENABLE_PIN       18   // E6

#define Z_STEP_PIN         32   // A4
#define Z_DIR_PIN          33   // A5
#define Z_ENABLE_PIN       17   // C7

#define E0_STEP_PIN        34   // A6
#define E0_DIR_PIN         35   // A7
#define E0_ENABLE_PIN      13   // C3

//
// Temperature Sensors
//
#define TEMP_0_PIN          1   // Analog Input
#define TEMP_BED_PIN        0   // Analog Input

//
// Heaters / Fans
//
#define HEATER_0_PIN       15   // C5 PWM3B - Extruder
#define HEATER_1_PIN       44   // F6
#define HEATER_2_PIN       45   // F7
#define HEATER_BED_PIN     14   // C4 PWM3C


#define FAN_PIN            16   // C6 PWM3A

//
// Misc. Functions
//
#define SDSS               20   // B0 SS
#define FILWIDTH_PIN        2   // Analog Input

//
// LCD / Controller
//
#if ENABLED(ULTRA_LCD) && ENABLED(NEWPANEL)
  // we have no buzzer installed
  #define BEEPER_PIN       -1

  // LCD Pins
  #if ENABLED(LCD_I2C_PANELOLU2)
    #define BTN_EN1         3   // D3 RX1   JP2-7
    #define BTN_EN2         2   // D2 TX1   JP2-5
    #define BTN_ENC        41   // F3       JP2-4
    #define SDSS           38   // F0       B-THERM connector - use SD card on Panelolu2
  #else
    #define BTN_EN1        10   // C0       JP11-12
    #define BTN_EN2        11   // C1       JP11-13
    #define BTN_ENC        12   // C2       JP11-14
  #endif

  // not connected
  #define SD_DETECT_PIN    -1

  #define LCD_PINS_RS       9   // E1       JP11-11
  #define LCD_PINS_ENABLE   8   // E0       JP11-10
  #define LCD_PINS_D4       7   // D7       JP11-8
  #define LCD_PINS_D5       6   // D6       JP11-7
  #define LCD_PINS_D6       5   // D5       JP11-6
  #define LCD_PINS_D7       4   // D4       JP11-5

#endif // ULTRA_LCD && NEWPANEL

#if ENABLED(VIKI2) || ENABLED(miniVIKI)
  #define BEEPER_PIN        8   // E0       JP11-10
  // Pins for DOGM SPI LCD Support
  #define DOGLCD_A0        40   // F2       JP2-2
  #define DOGLCD_CS        41   // F3       JP2-4
  #define LCD_SCREEN_ROT_180

  // The encoder and click button
  #define BTN_EN1           2   // D2 TX1   JP2-5
  #define BTN_EN2           3   // D3 RX1   JP2-7
  #define BTN_ENC          45   // F7 TDI   JP2-12

  #define SDSS             43   // F5 TMS   JP2-8
  #define SD_DETECT_PIN    -1

  #define STAT_LED_RED_PIN  12  // C2       JP11-14
  #define STAT_LED_BLUE_PIN 10  // C0       JP11-12

#endif
