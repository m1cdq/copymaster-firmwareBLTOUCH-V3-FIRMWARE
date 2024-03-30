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

//
// 200k ATC Semitec 204GT-2
// Verified by linagee. Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 4.7kohm pullup, voltage divider math, and manufacturer provided temp/resistance
//
const short temptable_2[][2] PROGMEM = {
  {   35.4003 * OVERSAMPLENR, 300 },
  {   38.1139 * OVERSAMPLENR, 295 },
  {   41.0738 * OVERSAMPLENR, 290 },
  {   44.2952 * OVERSAMPLENR, 285 },
  {   47.8124 * OVERSAMPLENR, 280 },
  {   51.6582 * OVERSAMPLENR, 275 },
  {   55.8635 * OVERSAMPLENR, 270 },
  {   60.4379 * OVERSAMPLENR, 265 },
  {   65.4658 * OVERSAMPLENR, 260 },
  {   70.9703 * OVERSAMPLENR, 255 },
  {   76.9899 * OVERSAMPLENR, 250 },
  {   83.5775 * OVERSAMPLENR, 245 },
  {   90.8166 * OVERSAMPLENR, 240 },
  {   98.7284 * OVERSAMPLENR, 235 },
  {   107.416 * OVERSAMPLENR, 230 },
  {   116.9 * OVERSAMPLENR, 225 },
  {   127.327 * OVERSAMPLENR, 220 },
  {   138.692 * OVERSAMPLENR, 215 },
  {   151.183 * OVERSAMPLENR, 210 },
  {  164.771 * OVERSAMPLENR, 205 },
  {  179.649 * OVERSAMPLENR, 200 },
  {  195.778 * OVERSAMPLENR, 195 },
  {  213.491 * OVERSAMPLENR, 190 },
  {  232.551 * OVERSAMPLENR, 185 },
  {  253.212 * OVERSAMPLENR, 180 },
  {  275.509 * OVERSAMPLENR, 175 },
  {  299.399 * OVERSAMPLENR, 170 },
  {  325.075 * OVERSAMPLENR, 165 },
  {  352.384 * OVERSAMPLENR, 160 },
  {  381.265 * OVERSAMPLENR, 155 },
  {  411.762 * OVERSAMPLENR, 150 },
  {  443.725 * OVERSAMPLENR, 145 },
  {  476.967 * OVERSAMPLENR, 140 },
  {  511.182 * OVERSAMPLENR, 135 },
  {  546.255 * OVERSAMPLENR, 130 },
  {  581.606 * OVERSAMPLENR, 125 },
  {  617.17 * OVERSAMPLENR, 120 },
  {  652.413 * OVERSAMPLENR, 115 },
  {  687.111 * OVERSAMPLENR, 110 },
  {  720.736 * OVERSAMPLENR, 105 },
  {  753.009 * OVERSAMPLENR, 100 },
  {  783.72 * OVERSAMPLENR,  95 },
  {  812.634 * OVERSAMPLENR,  90 },
  {  839.46 * OVERSAMPLENR,  85 },
  {  864.053 * OVERSAMPLENR,  80 },
  {  886.413 * OVERSAMPLENR,  75 },
  {  906.615 * OVERSAMPLENR,  70 },
  {  924.521 * OVERSAMPLENR,  65 },
  {  940.357 * OVERSAMPLENR,  60 },
  {  954.148 * OVERSAMPLENR,  55 },
  {  966.126 * OVERSAMPLENR,  50 },
  {  976.353 * OVERSAMPLENR,  45 },
  {  985.093 * OVERSAMPLENR,  40 },
  {  992.42 * OVERSAMPLENR,  35 },
  {  998.589 * OVERSAMPLENR,  30 },
  {  1003.68 * OVERSAMPLENR,  25 },
  {  1007.87 * OVERSAMPLENR,  20 },
  {  1011.27 * OVERSAMPLENR,  15 },
  {  1014.05 * OVERSAMPLENR,  10 },
  { 1016.27 * OVERSAMPLENR,   5 },
  { 1018.05 * OVERSAMPLENR,   0 },
  { 1019.45 * OVERSAMPLENR,  -5 },
  { 1020.55 * OVERSAMPLENR, -10 },
  { 1021.4 * OVERSAMPLENR, -15 }
  /*
  {    1 * OVERSAMPLENR, 848 },
  {   30 * OVERSAMPLENR, 300 }, // top rating 300C
  {   34 * OVERSAMPLENR, 290 },
  {   39 * OVERSAMPLENR, 280 },
  {   46 * OVERSAMPLENR, 270 },
  {   53 * OVERSAMPLENR, 260 },
  {   63 * OVERSAMPLENR, 250 },
  {   74 * OVERSAMPLENR, 240 },
  {   87 * OVERSAMPLENR, 230 },
  {  104 * OVERSAMPLENR, 220 },
  {  124 * OVERSAMPLENR, 210 },
  {  148 * OVERSAMPLENR, 200 },
  {  176 * OVERSAMPLENR, 190 },
  {  211 * OVERSAMPLENR, 180 },
  {  252 * OVERSAMPLENR, 170 },
  {  301 * OVERSAMPLENR, 160 },
  {  357 * OVERSAMPLENR, 150 },
  {  420 * OVERSAMPLENR, 140 },
  {  489 * OVERSAMPLENR, 130 },
  {  562 * OVERSAMPLENR, 120 },
  {  636 * OVERSAMPLENR, 110 },
  {  708 * OVERSAMPLENR, 100 },
  {  775 * OVERSAMPLENR,  90 },
  {  835 * OVERSAMPLENR,  80 },
  {  884 * OVERSAMPLENR,  70 },
  {  924 * OVERSAMPLENR,  60 },
  {  955 * OVERSAMPLENR,  50 },
  {  977 * OVERSAMPLENR,  40 },
  {  993 * OVERSAMPLENR,  30 },
  { 1004 * OVERSAMPLENR,  20 },
  { 1012 * OVERSAMPLENR,  10 },
  { 1016 * OVERSAMPLENR,   0 }
  */
};
