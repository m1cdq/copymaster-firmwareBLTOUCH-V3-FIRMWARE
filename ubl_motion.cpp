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
#include "MarlinConfig.h"

#if ENABLED(AUTO_BED_LEVELING_UBL)

  #include "Marlin.h"
  #include "ubl.h"
  #include "planner.h"
  #include "stepper.h"
  #include <avr/io.h>
  #include <math.h>

  extern float destination[XYZE];

  #if AVR_AT90USB1286_FAMILY  // Teensyduino & Printrboard IDE extensions have compile errors without this
    inline void set_current_to_destination() { COPY(current_position, destination); }
  #else
    extern void set_current_to_destination();
  #endif

#if ENABLED(DELTA)

  extern float delta[ABC],
               endstop_adj[ABC];

  extern float delta_radius,
               delta_tower_angle_trim[ABC],
               delta_tower[ABC][2],
               delta_diagonal_rod,
               delta_calibration_radius,
               delta_diagonal_rod_2_tower[ABC],
               delta_segments_per_second,
               delta_clip_start_height;

  extern float delta_safe_distance_from_top();

#endif


  static void debug_echo_axis(const AxisEnum axis) {
    if (current_position[axis] == destination[axis])
      SERIAL_ECHOPGM("-------------");
    else
      SERIAL_ECHO_F(destination[X_AXIS], 6);
  }

  void debug_current_and_destination(const char *title) {

    // if the title message starts with a '!' it is so important, we are going to
    // ignore the status of the g26_debug_flag
    if (*title != '!' && !ubl.g26_debug_flag) return;

    const float de = destination[E_AXIS] - current_position[E_AXIS];

    if (de == 0.0) return; // Printing moves only

    const float dx = destination[X_AXIS] - current_position[X_AXIS],
                dy = destination[Y_AXIS] - current_position[Y_AXIS],
                xy_dist = HYPOT(dx, dy);

    if (xy_dist == 0.0) return;

    SERIAL_ECHOPGM("   fpmm=");
    const float fpmm = de / xy_dist;
    SERIAL_ECHO_F(fpmm, 6);

    SERIAL_ECHOPGM("    current=( ");
    SERIAL_ECHO_F(current_position[X_AXIS], 6);
    SERIAL_ECHOPGM(", ");
    SERIAL_ECHO_F(current_position[Y_AXIS], 6);
    SERIAL_ECHOPGM(", ");
    SERIAL_ECHO_F(current_position[Z_AXIS], 6);
    SERIAL_ECHOPGM(", ");
    SERIAL_ECHO_F(current_position[E_AXIS], 6);
    SERIAL_ECHOPGM(" )   destination=( ");
    debug_echo_axis(X_AXIS);
    SERIAL_ECHOPGM(", ");
    debug_echo_axis(Y_AXIS);
    SERIAL_ECHOPGM(", ");
    debug_echo_axis(Z_AXIS);
    SERIAL_ECHOPGM(", ");
    debug_echo_axis(E_AXIS);
    SERIAL_ECHOPGM(" )   ");
    SERIAL_ECHO(title);
    SERIAL_EOL();

  }

  void unified_bed_leveling::line_to_destination_cartesian(const float &feed_rate, uint8_t extruder) {
    /**
     * Much of the nozzle movement will be within the same cell. So we will do as little computation
     * as possible to determine if this is the case. If this move is within the same cell, we will
     * just do the required Z-Height correction, call the Planner's buffer_line() routine, and leave
     */
    const float start[XYZE] = {
                  current_position[X_AXIS],
                  current_position[Y_AXIS],
                  current_position[Z_AXIS],
                  current_position[E_AXIS]
                },
                end[XYZE] = {
                  destination[X_AXIS],
                  destination[Y_AXIS],
                  destination[Z_AXIS],
                  destination[E_AXIS]
                };

    const int cell_start_xi = get_cell_index_x(RAW_X_POSITION(start[X_AXIS])),
              cell_start_yi = get_cell_index_y(RAW_Y_POSITION(start[Y_AXIS])),
              cell_dest_xi  = get_cell_index_x(RAW_X_POSITION(end[X_AXIS])),
              cell_dest_yi  = get_cell_index_y(RAW_Y_POSITION(end[Y_AXIS]));

    if (g26_debug_flag) {
      SERIAL_ECHOPAIR(" ubl.line_to_destination(xe=", end[X_AXIS]);
      SERIAL_ECHOPAIR(", ye=", end[Y_AXIS]);
      SERIAL_ECHOPAIR(", ze=", end[Z_AXIS]);
      SERIAL_ECHOPAIR(", ee=", end[E_AXIS]);
      SERIAL_CHAR(')');
      SERIAL_EOL();
      debug_current_and_destination(PSTR("Start of ubl.line_to_destination()"));
    }

    if (cell_start_xi == cell_dest_xi && cell_start_yi == cell_dest_yi) { // if the whole move is within the same cell,
      /**
       * we don't need to break up the move
       *
       * If we are moving off the print bed, we are going to allow the move at this level.
       * But we detect it and isolate it. For now, we just pass along the request.
       */

      if (!WITHIN(cell_dest_xi, 0, GRID_MAX_POINTS_X - 1) || !WITHIN(cell_dest_yi, 0, GRID_MAX_POINTS_Y - 1)) {

        // Note: There is no Z Correction in this case. We are off the grid and don't know what
        // a reasonable correction would be.

        planner._buffer_line(end[X_AXIS], end[Y_AXIS], end[Z_AXIS] + state.z_offset, end[E_AXIS], feed_rate, extruder);
        set_current_to_destination();

        if (g26_debug_flag)
          debug_current_and_destination(PSTR("out of bounds in ubl.line_to_destination()"));

        return;
      }

      FINAL_MOVE:

      /**
       * Optimize some floating point operations here. We could call float get_z_correction(float x0, float y0) to
       * generate the correction for us. But we can lighten the load on the CPU by doing a modified version of the function.
       * We are going to only calculate the amount we are from the first mesh line towards the second mesh line once.
       * We will use this fraction in both of the original two Z Height calculations for the bi-linear interpolation. And,
       * instead of doing a generic divide of the distance, we know the distance is MESH_X_DIST so we can use the preprocessor
       * to create a 1-over number for us. That will allow us to do a floating point multiply instead of a floating point divide.
       */

      const float xratio = (RAW_X_POSITION(end[X_AXIS]) - mesh_index_to_xpos(cell_dest_xi)) * (1.0 / (MESH_X_DIST));

      float z1 = z_values[cell_dest_xi    ][cell_dest_yi    ] + xratio *
                (z_values[cell_dest_xi + 1][cell_dest_yi    ] - z_values[cell_dest_xi][cell_dest_yi    ]),
            z2 = z_values[cell_dest_xi    ][cell_dest_yi + 1] + xratio *
                (z_values[cell_dest_xi + 1][cell_dest_yi + 1] - z_values[cell_dest_xi][cell_dest_yi + 1]);

      if (cell_dest_xi >= GRID_MAX_POINTS_X - 1) z1 = z2 = 0.0;

      // we are done with the fractional X distance into the cell. Now with the two Z-Heights we have calculated, we
      // are going to apply the Y-Distance into the cell to interpolate the final Z correction.

      const float yratio = (RAW_Y_POSITION(end[Y_AXIS]) - mesh_index_to_ypos(cell_dest_yi)) * (1.0 / (MESH_Y_DIST));
      float z0 = cell_dest_yi < GRID_MAX_POINTS_Y - 1 ? (z1 + (z2 - z1) * yratio) * fade_scaling_factor_for_z(end[Z_AXIS]) : 0.0;

      /**
       * If part of the Mesh is undefined, it will show up as NAN
       * in z_values[][] and propagate through the
       * calculations. If our correction is NAN, we throw it out
       * because part of the Mesh is undefined and we don't have the
       * information we need to complete the height correction.
       */
      if (isnan(z0)) z0 = 0.0;

      planner._buffer_line(end[X_AXIS], end[Y_AXIS], end[Z_AXIS] + z0 + state.z_offset, end[E_AXIS], feed_rate, extruder);

      if (g26_debug_flag)
        debug_current_and_destination(PSTR("FINAL_MOVE in ubl.line_to_destination()"));

      set_current_to_destination();
      return;
    }

    /**
     * If we get here, we are processing a move that crosses at least one Mesh Line. We will check
     * for the simple case of just crossing X or just crossing Y Mesh Lines after we get all the details
     * of the move figured out. We can process the easy case of just crossing an X or Y Mesh Line with less
     * computation and in fact most lines are of this nature. We will check for that in the following
     * blocks of code:
     */

    const float dx = end[X_AXIS] - start[X_AXIS],
                dy = end[Y_AXIS] - start[Y_AXIS];

    const int left_flag = dx < 0.0 ? 1 : 0,
              down_flag = dy < 0.0 ? 1 : 0;

    const float adx = left_flag ? -dx : dx,
                ady = down_flag ? -dy : dy;

    const int dxi = cell_start_xi == cell_dest_xi ? 0 : left_flag ? -1 : 1,
              dyi = cell_start_yi == cell_dest_yi ? 0 : down_flag ? -1 : 1;

    /**
     * Compute the scaling factor for the extruder for each partial move.
     * We need to watch out for zero length moves because it will cause us to
     * have an infinate scaling factor. We are stuck doing a floating point
     * divide to get our scaling factor, but after that, we just multiply by this
     * number. We also pick our scaling factor based on whether the X or Y
     * component is larger. We use the biggest of the two to preserve precision.
     */

    const bool use_x_dist = adx > ady;

    float on_axis_distance = use_x_dist ? dx : dy,
          e_position = end[E_AXIS] - start[E_AXIS],
          z_position = end[Z_AXIS] - start[Z_AXIS];

    const float e_normalized_dist = e_position / on_axis_distance,
                z_normalized_dist = z_position / on_axis_distance;

    int current_xi = cell_start_xi,
        current_yi = cell_start_yi;

    const float m = dy / dx,
                c = start[Y_AXIS] - m * start[X_AXIS];

    const bool inf_normalized_flag = (isinf(e_normalized_dist) != 0),
               inf_m_flag = (isinf(m) != 0);
    /**
     * This block handles vertical lines. These are lines that stay within the same
     * X Cell column. They do not need to be perfectly vertical. They just can
     * not cross into another X Cell column.
     */
    if (dxi == 0) {       // Check for a vertical line
      current_yi += down_flag;  // Line is heading down, we just want to go to the bottom
      while (current_yi != cell_dest_yi + down_flag) {
        current_yi += dyi;
        const float next_mesh_line_y = LOGICAL_Y_POSITION(mesh_index_to_ypos(current_yi));

        /**
         * if the slope of the line is infinite, we won't do the calculations
         * else, we know the next X is the same so we can recover and continue!
         * Calculate X at the next Y mesh line
         */
        const float x = inf_m_flag ? start[X_AXIS] : (next_mesh_line_y - c) / m;

        float z0 = z_correction_for_x_on_horizontal_mesh_line(x, current_xi, current_yi);

        z0 *= fade_scaling_factor_for_z(end[Z_AXIS]);

        /**
         * If part of the Mesh is undefined, it will show up as NAN
         * in z_values[][] and propagate through the
         * calculations. If our correction is NAN, we throw it out
         * because part of the Mesh is undefined and we don't have the
         * information we need to complete the height correction.
         */
        if (isnan(z0)) z0 = 0.0;

        const float y = LOGICAL_Y_POSITION(mesh_index_to_ypos(current_yi));

        /**
         * Without this check, it is possible for the algorithm to generate a zero length move in the case
         * where the line is heading down and it is starting right on a Mesh Line boundary. For how often that
         * happens, it might be best to remove the check and always 'schedule' the move because
         * the planner._buffer_line() routine will filter it if that happens.
         */
        if (y != start[Y_AXIS]) {
          if (!inf_normalized_flag) {
            on_axis_distance = use_x_dist ? x - start[X_AXIS] : y - start[Y_AXIS];
            e_position = start[E_AXIS] + on_axis_distance * e_normalized_dist;
            z_position = start[Z_AXIS] + on_axis_distance * z_normalized_dist;
          }
          else {
            e_position = end[E_AXIS];
            z_position = end[Z_AXIS];
          }

          planner._buffer_line(x, y, z_position + z0 + state.z_offset, e_position, feed_rate, extruder);
        } //else printf("FIRST MOVE PRUNED  ");
      }

      if (g26_debug_flag)
        debug_current_and_destination(PSTR("vertical move done in ubl.line_to_destination()"));

      //
      // Check if we are at the final destination. Usually, we won't be, but if it is on a Y Mesh Line, we are done.
      //
      if (current_position[X_AXIS] != end[X_AXIS] || current_position[Y_AXIS] != end[Y_AXIS])
        goto FINAL_MOVE;

      set_current_to_destination();
      return;
    }

    /**
     *
     * This block handles horizontal lines. These are lines that stay within the same
     * Y Cell row. They do not need to be perfectly horizontal. They just can
     * not cross into another Y Cell row.
     *
     */

    if (dyi == 0) {             // Check for a horizontal line
      current_xi += left_flag;  // Line is heading left, we just want to go to the left
                                // edge of this cell for the first move.
      while (current_xi != cell_dest_xi + left_flag) {
        current_xi += dxi;
        const float next_mesh_line_x = LOGICAL_X_POSITION(mesh_index_to_xpos(current_xi)),
                    y = m * next_mesh_line_x + c;   // Calculate Y at the next X mesh line

        float z0 = z_correction_for_y_on_vertical_mesh_line(y, current_xi, current_yi);

        z0 *= fade_scaling_factor_for_z(end[Z_AXIS]);

        /**
         * If part of the Mesh is undefined, it will show up as NAN
         * in z_values[][] and propagate through the
         * calculations. If our correction is NAN, we throw it out
         * because part of the Mesh is undefined and we don't have the
         * information we need to complete the height correction.
         */
        if (isnan(z0)) z0 = 0.0;

        const float x = LOGICAL_X_POSITION(mesh_index_to_xpos(current_xi));

        /**
         * Without this check, it is possible for the algorithm to generate a zero length move in the case
         * where the line is heading left and it is starting right on a Mesh Line boundary. For how often
         * that happens, it might be best to remove the check and always 'schedule' the move because
         * the planner._buffer_line() routine will filter it if that happens.
         */
        if (x != start[X_AXIS]) {
          if (!inf_normalized_flag) {
            on_axis_distance = use_x_dist ? x - start[X_AXIS] : y - start[Y_AXIS];
            e_position = start[E_AXIS] + on_axis_distance * e_normalized_dist;  // is based on X or Y because this is a horizontal move
            z_position = start[Z_AXIS] + on_axis_distance * z_normalized_dist;
          }
          else {
            e_position = end[E_AXIS];
            z_position = end[Z_AXIS];
          }

          planner._buffer_line(x, y, z_position + z0 + state.z_offset, e_position, feed_rate, extruder);
        } //else printf("FIRST MOVE PRUNED  ");
      }

      if (g26_debug_flag)
        debug_current_and_destination(PSTR("horizontal move done in ubl.line_to_destination()"));

      if (current_position[X_AXIS] != end[X_AXIS] || current_position[Y_AXIS] != end[Y_AXIS])
        goto FINAL_MOVE;

      set_current_to_destination();
      return;
    }

    /**
     *
     * This block handles the generic case of a line crossing both X and Y Mesh lines.
     *
     */

    int xi_cnt = cell_start_xi - cell_dest_xi,
        yi_cnt = cell_start_yi - cell_dest_yi;

    if (xi_cnt < 0) xi_cnt = -xi_cnt;
    if (yi_cnt < 0) yi_cnt = -yi_cnt;

    current_xi += left_flag;
    current_yi += down_flag;

    while (xi_cnt > 0 || yi_cnt > 0) {

      const float next_mesh_line_x = LOGICAL_X_POSITION(mesh_index_to_xpos(current_xi + dxi)),
                  next_mesh_line_y = LOGICAL_Y_POSITION(mesh_index_to_ypos(current_yi + dyi)),
                  y = m * next_mesh_line_x + c,   // Calculate Y at the next X mesh line
                  x = (next_mesh_line_y - c) / m; // Calculate X at the next Y mesh line
                                                  // (No need to worry about m being zero.
                                                  //  If that was the case, it was already detected
                                                  //  as a vertical line move above.)

      if (left_flag == (x > next_mesh_line_x)) { // Check if we hit the Y line first
        // Yes!  Crossing a Y Mesh Line next
        float z0 = z_correction_for_x_on_horizontal_mesh_line(x, current_xi - left_flag, current_yi + dyi);

        z0 *= fade_scaling_factor_for_z(end[Z_AXIS]);

        /**
         * If part of the Mesh is undefined, it will show up as NAN
         * in z_values[][] and propagate through the
         * calculations. If our correction is NAN, we throw it out
         * because part of the Mesh is undefined and we don't have the
         * information we need to complete the height correction.
         */
        if (isnan(z0)) z0 = 0.0;

        if (!inf_normalized_flag) {
          on_axis_distance = use_x_dist ? x - start[X_AXIS] : next_mesh_line_y - start[Y_AXIS];
          e_position = start[E_AXIS] + on_axis_distance * e_normalized_dist;
          z_position = start[Z_AXIS] + on_axis_distance * z_normalized_dist;
        }
        else {
          e_position = end[E_AXIS];
          z_position = end[Z_AXIS];
        }
        planner._buffer_line(x, next_mesh_line_y, z_position + z0 + state.z_offset, e_position, feed_rate, extruder);
        current_yi += dyi;
        yi_cnt--;
      }
      else {
        // Yes!  Crossing a X Mesh Line next
        float z0 = z_correction_for_y_on_vertical_mesh_line(y, current_xi + dxi, current_yi - down_flag);

        z0 *= fade_scaling_factor_for_z(end[Z_AXIS]);

        /**
         * If part of the Mesh is undefined, it will show up as NAN
         * in z_values[][] and propagate through the
         * calculations. If our correction is NAN, we throw it out
         * because part of the Mesh is undefined and we don't have the
         * information we need to complete the height correction.
         */
        if (isnan(z0)) z0 = 0.0;

        if (!inf_normalized_flag) {
          on_axis_distance = use_x_dist ? next_mesh_line_x - start[X_AXIS] : y - start[Y_AXIS];
          e_position = start[E_AXIS] + on_axis_distance * e_normalized_dist;
          z_position = start[Z_AXIS] + on_axis_distance * z_normalized_dist;
        }
        else {
          e_position = end[E_AXIS];
          z_position = end[Z_AXIS];
        }

        planner._buffer_line(next_mesh_line_x, y, z_position + z0 + state.z_offset, e_position, feed_rate, extruder);
        current_xi += dxi;
        xi_cnt--;
      }

      if (xi_cnt < 0 || yi_cnt < 0) break; // we've gone too far, so exit the loop and move on to FINAL_MOVE
    }

    if (g26_debug_flag)
      debug_current_and_destination(PSTR("generic move done in ubl.line_to_destination()"));

    if (current_position[X_AXIS] != end[X_AXIS] || current_position[Y_AXIS] != end[Y_AXIS])
      goto FINAL_MOVE;

    set_current_to_destination();
  }

  #if UBL_DELTA

    // macro to inline copy exactly 4 floats, don't rely on sizeof operator
    #define COPY_XYZE( target, source ) { \
                target[X_AXIS] = source[X_AXIS]; \
                target[Y_AXIS] = source[Y_AXIS]; \
                target[Z_AXIS] = source[Z_AXIS]; \
                target[E_AXIS] = source[E_AXIS]; \
            }

    #if IS_SCARA // scale the feed rate from mm/s to degrees/s
      static float scara_feed_factor, scara_oldA, scara_oldB;
    #endif

    // We don't want additional apply_leveling() performed by regular buffer_line or buffer_line_kinematic,
    // so we call _buffer_line directly here.  Per-segmented leveling and kinematics performed first.

    inline void _O2 ubl_buffer_segment_raw( float rx, float ry, float rz, float le, float fr ) {

      #if ENABLED(DELTA)  // apply delta inverse_kinematics

        const float delta_A = rz + SQRT( delta_diagonal_rod_2_tower[A_AXIS]
                                         - HYPOT2( delta_tower[A_AXIS][X_AXIS] - rx,
                                                   delta_tower[A_AXIS][Y_AXIS] - ry ));

        const float delta_B = rz + SQRT( delta_diagonal_rod_2_tower[B_AXIS]
                                         - HYPOT2( delta_tower[B_AXIS][X_AXIS] - rx,
                                                   delta_tower[B_AXIS][Y_AXIS] - ry ));

        const float delta_C = rz + SQRT( delta_diagonal_rod_2_tower[C_AXIS]
                                         - HYPOT2( delta_tower[C_AXIS][X_AXIS] - rx,
                                                   delta_tower[C_AXIS][Y_AXIS] - ry ));

        planner._buffer_line(delta_A, delta_B, delta_C, le, fr, active_extruder);

      #elif IS_SCARA  // apply scara inverse_kinematics (should be changed to save raw->logical->raw)

        const float lseg[XYZ] = { LOGICAL_X_POSITION(rx),
                                  LOGICAL_Y_POSITION(ry),
                                  LOGICAL_Z_POSITION(rz)
                                };

        inverse_kinematics(lseg); // this writes delta[ABC] from lseg[XYZ]
                                  // should move the feedrate scaling to scara inverse_kinematics

        const float adiff = FABS(delta[A_AXIS] - scara_oldA),
                    bdiff = FABS(delta[B_AXIS] - scara_oldB);
        scara_oldA = delta[A_AXIS];
        scara_oldB = delta[B_AXIS];
        float s_feedrate = max(adiff, bdiff) * scara_feed_factor;

        planner._buffer_line(delta[A_AXIS], delta[B_AXIS], delta[C_AXIS], le, s_feedrate, active_extruder);

      #else // CARTESIAN

        // Cartesian _buffer_line seems to take LOGICAL, not RAW coordinates

        const float lx = LOGICAL_X_POSITION(rx),
                    ly = LOGICAL_Y_POSITION(ry),
                    lz = LOGICAL_Z_POSITION(rz);

        planner._buffer_line(lx, ly, lz, le, fr, active_extruder);

      #endif

    }


    /**
     * Prepare a segmented linear move for DELTA/SCARA/CARTESIAN with UBL and FADE semantics.
     * This calls planner._buffer_line multiple times for small incremental moves.
     * Returns true if did NOT move, false if moved (requires current_position update).
     */

    bool _O2 unified_bed_leveling::prepare_segmented_line_to(const float ltarget[XYZE], const float &feedrate) {

      if (!position_is_reachable_xy(ltarget[X_AXIS], ltarget[Y_AXIS]))  // fail if moving outside reachable boundary
        return true; // did not move, so current_position still accurate

      const float tot_dx = ltarget[X_AXIS] - current_position[X_AXIS],
                  tot_dy = ltarget[Y_AXIS] - current_position[Y_AXIS],
                  tot_dz = ltarget[Z_AXIS] - current_position[Z_AXIS],
                  tot_de = ltarget[E_AXIS] - current_position[E_AXIS];

      const float cartesian_xy_mm = HYPOT(tot_dx, tot_dy);  // total horizontal xy distance

      #if IS_KINEMATIC
        const float seconds = cartesian_xy_mm / feedrate;                                  // seconds to move xy distance at requested rate
        uint16_t segments = lroundf(delta_segments_per_second * seconds),                  // preferred number of segments for distance @ feedrate
                 seglimit = lroundf(cartesian_xy_mm * (1.0 / (DELTA_SEGMENT_MIN_LENGTH))); // number of segments at minimum segment length
        NOMORE(segments, seglimit);                                                        // limit to minimum segment length (fewer segments)
      #else
        uint16_t segments = lroundf(cartesian_xy_mm * (1.0 / (DELTA_SEGMENT_MIN_LENGTH))); // cartesian fixed segment length
      #endif

      NOLESS(segments, 1);                        // must have at least one segment
      const float inv_segments = 1.0 / segments;  // divide once, multiply thereafter

      #if IS_SCARA // scale the feed rate from mm/s to degrees/s
        scara_feed_factor = cartesian_xy_mm * inv_segments * feedrate;
        scara_oldA = stepper.get_axis_position_degrees(A_AXIS);
        scara_oldB = stepper.get_axis_position_degrees(B_AXIS);
      #endif

      const float seg_dx = tot_dx * inv_segments,
                  seg_dy = tot_dy * inv_segments,
                  seg_dz = tot_dz * inv_segments,
                  seg_de = tot_de * inv_segments;

      // Note that E segment distance could vary slightly as z mesh height
      // changes for each segment, but small enough to ignore.

      float seg_rx = RAW_X_POSITION(current_position[X_AXIS]),
            seg_ry = RAW_Y_POSITION(current_position[Y_AXIS]),
            seg_rz = RAW_Z_POSITION(current_position[Z_AXIS]),
            seg_le = current_position[E_AXIS];

      const bool above_fade_height = (
        #if ENABLED(ENABLE_LEVELING_FADE_HEIGHT)
          planner.z_fade_height != 0 && planner.z_fade_height < RAW_Z_POSITION(ltarget[Z_AXIS])
        #else
          false
        #endif
      );

      // Only compute leveling per segment if ubl active and target below z_fade_height.

      if (!state.active || above_fade_height) {   // no mesh leveling

        const float z_offset = state.active ? state.z_offset : 0.0;

        do {

          if (--segments) {     // not the last segment
            seg_rx += seg_dx;
            seg_ry += seg_dy;
            seg_rz += seg_dz;
            seg_le += seg_de;
          } else {              // last segment, use exact destination
            seg_rx = RAW_X_POSITION(ltarget[X_AXIS]);
            seg_ry = RAW_Y_POSITION(ltarget[Y_AXIS]);
            seg_rz = RAW_Z_POSITION(ltarget[Z_AXIS]);
            seg_le = ltarget[E_AXIS];
          }

          ubl_buffer_segment_raw( seg_rx, seg_ry, seg_rz + z_offset, seg_le, feedrate );

        } while (segments);

        return false; // moved but did not set_current_to_destination();
      }

      // Otherwise perform per-segment leveling

      #if ENABLED(ENABLE_LEVELING_FADE_HEIGHT)
        const float fade_scaling_factor = fade_scaling_factor_for_z(ltarget[Z_AXIS]);
      #endif

      // increment to first segment destination
      seg_rx += seg_dx;
      seg_ry += seg_dy;
      seg_rz += seg_dz;
      seg_le += seg_de;

      for(;;) {  // for each mesh cell encountered during the move

        // Compute mesh cell invariants that remain constant for all segments within cell.
        // Note for cell index, if point is outside the mesh grid (in MESH_INSET perimeter)
        // the bilinear interpolation from the adjacent cell within the mesh will still work.
        // Inner loop will exit each time (because out of cell bounds) but will come back
        // in top of loop and again re-find same adjacent cell and use it, just less efficient
        // for mesh inset area.

        int8_t cell_xi = (seg_rx - (UBL_MESH_MIN_X)) * (1.0 / (MESH_X_DIST)),
               cell_yi = (seg_ry - (UBL_MESH_MIN_Y)) * (1.0 / (MESH_X_DIST));

        cell_xi = constrain(cell_xi, 0, (GRID_MAX_POINTS_X) - 1);
        cell_yi = constrain(cell_yi, 0, (GRID_MAX_POINTS_Y) - 1);

        const float x0 = mesh_index_to_xpos(cell_xi),   // 64 byte table lookup avoids mul+add
                    y0 = mesh_index_to_ypos(cell_yi);

        float z_x0y0 = z_values[cell_xi  ][cell_yi  ],  // z at lower left corner
              z_x1y0 = z_values[cell_xi+1][cell_yi  ],  // z at upper left corner
              z_x0y1 = z_values[cell_xi  ][cell_yi+1],  // z at lower right corner
              z_x1y1 = z_values[cell_xi+1][cell_yi+1];  // z at upper right corner

        if (isnan(z_x0y0)) z_x0y0 = 0;              // ideally activating state.active (G29 A)
        if (isnan(z_x1y0)) z_x1y0 = 0;              //   should refuse if any invalid mesh points
        if (isnan(z_x0y1)) z_x0y1 = 0;              //   in order to avoid isnan tests per cell,
        if (isnan(z_x1y1)) z_x1y1 = 0;              //   thus guessing zero for undefined points

        float cx = seg_rx - x0,   // cell-relative x and y
              cy = seg_ry - y0;

        const float z_xmy0 = (z_x1y0 - z_x0y0) * (1.0 / (MESH_X_DIST)),   // z slope per x along y0 (lower left to lower right)
                    z_xmy1 = (z_x1y1 - z_x0y1) * (1.0 / (MESH_X_DIST));   // z slope per x along y1 (upper left to upper right)

              float z_cxy0 = z_x0y0 + z_xmy0 * cx;            // z height along y0 at cx (changes for each cx in cell)

        const float z_cxy1 = z_x0y1 + z_xmy1 * cx,            // z height along y1 at cx
                    z_cxyd = z_cxy1 - z_cxy0;                 // z height difference along cx from y0 to y1

              float z_cxym = z_cxyd * (1.0 / (MESH_Y_DIST));  // z slope per y along cx from y0 to y1 (changes for each cx in cell)

        //    float z_cxcy = z_cxy0 + z_cxym * cy;            // interpolated mesh z height along cx at cy (do inside the segment loop)

        // As subsequent segments step through this cell, the z_cxy0 intercept will change
        // and the z_cxym slope will change, both as a function of cx within the cell, and
        // each change by a constant for fixed segment lengths.

        const float z_sxy0 = z_xmy0 * seg_dx,                                     // per-segment adjustment to z_cxy0
                    z_sxym = (z_xmy1 - z_xmy0) * (1.0 / (MESH_Y_DIST)) * seg_dx;  // per-segment adjustment to z_cxym

        for(;;) {  // for all segments within this mesh cell

          float z_cxcy = z_cxy0 + z_cxym * cy;      // interpolated mesh z height along cx at cy

          #if ENABLED(ENABLE_LEVELING_FADE_HEIGHT)
            z_cxcy *= fade_scaling_factor;          // apply fade factor to interpolated mesh height
          #endif

          z_cxcy += state.z_offset;                 // add fixed mesh offset from G29 Z

          if (--segments == 0) {                    // if this is last segment, use ltarget for exact
            seg_rx = RAW_X_POSITION(ltarget[X_AXIS]);
            seg_ry = RAW_Y_POSITION(ltarget[Y_AXIS]);
            seg_rz = RAW_Z_POSITION(ltarget[Z_AXIS]);
            seg_le = ltarget[E_AXIS];
          }

          ubl_buffer_segment_raw( seg_rx, seg_ry, seg_rz + z_cxcy, seg_le, feedrate );

          if (segments == 0 )                       // done with last segment
            return false;                           // did not set_current_to_destination()

          seg_rx += seg_dx;
          seg_ry += seg_dy;
          seg_rz += seg_dz;
          seg_le += seg_de;

          cx += seg_dx;
          cy += seg_dy;

          if (!WITHIN(cx, 0, MESH_X_DIST) || !WITHIN(cy, 0, MESH_Y_DIST)) {  // done within this cell, break to next
            break;
          }

          // Next segment still within same mesh cell, adjust the per-segment
          // slope and intercept to compute next z height.

          z_cxy0 += z_sxy0;   // adjust z_cxy0 by per-segment z_sxy0
          z_cxym += z_sxym;   // adjust z_cxym by per-segment z_sxym

        } // segment loop
      } // cell loop
    }

  #endif // UBL_DELTA

#endif // AUTO_BED_LEVELING_UBL
