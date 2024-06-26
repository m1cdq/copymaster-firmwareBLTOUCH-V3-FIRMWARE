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
 * Geeetech GT2560 Revision A board pin assignments, based on the work of
 * George Robles (https://georges3dprinters.com) and
 * Richard Smith <galorin@gmail.com>
 */

#if !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__)
  #error "Oops!  Make sure you have 'Arduino Mega' selected from the 'Tools -> Boards' menu."
#endif

#define BOARD_NAME           "GT2560 Rev.A"
#define DEFAULT_MACHINE_NAME "Prusa i3 Pro B"
#define LARGE_FLASH true

//
// Limit Switches
//
#define X_MIN_PIN          22
#define X_MAX_PIN          24
#define Y_MIN_PIN          26
#define Y_MAX_PIN          28
#define Z_MIN_PIN          30
#define Z_MAX_PIN          32

//
// Steppers
//
#define X_STEP_PIN         25
#define X_DIR_PIN          23
#define X_ENABLE_PIN       27

#define Y_STEP_PIN         31
#define Y_DIR_PIN          33
#define Y_ENABLE_PIN       29

#define Z_STEP_PIN         37
#define Z_DIR_PIN          39
#define Z_ENABLE_PIN       35

#define E0_STEP_PIN        43
#define E0_DIR_PIN         45
#define E0_ENABLE_PIN      41

#define E1_STEP_PIN        49
#define E1_DIR_PIN         47
#define E1_ENABLE_PIN      48

//
// Temperature Sensors
//
#define TEMP_0_PIN          8
#define TEMP_1_PIN          9
#define TEMP_BED_PIN       10

//
// Heaters / Fans
//
#define HEATER_0_PIN        2
#define HEATER_1_PIN        3
#define HEATER_BED_PIN      4
#define FAN_PIN             7

//
// Misc. Functions
//
#define SDPOWER            -1
#define SDSS               53
#define LED_PIN            13
#define PS_ON_PIN          12
#define SUICIDE_PIN        54  // Must be enabled at startup to keep power flowing
#define KILL_PIN           -1

#if ENABLED(ULTRA_LCD)

  #define BEEPER_PIN       18

  #if ENABLED(NEWPANEL)

    #define LCD_PINS_RS    20
    #define LCD_PINS_ENABLE 17
    #define LCD_PINS_D4    16
    #define LCD_PINS_D5    21
    #define LCD_PINS_D6     5
    #define LCD_PINS_D7     6

    // Buttons are directly attached
    #define BTN_EN1        42
    #define BTN_EN2        40
    #define BTN_ENC        19

    #define SD_DETECT_PIN  38

  #else // !NEWPANEL

    #define SHIFT_CLK      38
    #define SHIFT_LD       42
    #define SHIFT_OUT      40
    #define SHIFT_EN       17

    #define LCD_PINS_RS    16
    #define LCD_PINS_ENABLE 5
    #define LCD_PINS_D4     6
    #define LCD_PINS_D5    21
    #define LCD_PINS_D6    20
    #define LCD_PINS_D7    19

    #define SD_DETECT_PIN  -1

  #endif // !NEWPANEL

#endif // ULTRA_LCD
