// SPDX-License-Identifier: GPL-3.0-or-later

/* Copyright 2026 Rocco Rusponi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "main.h"

#include "tof2can.h"

struct SensorData {
    bool available;

    int     buffer_length;
    int16_t buffer[64];

    bool below_threshold;
    bool threshold_event;
};

struct SensorConfig {
    int matrix_width;

    struct Bounds { int x0, y0, x1, y1 } bounds;
    int result_selector;

    int threshold;
    int threshold_delay;
    int threshold_focus;

    int transmit_timing;
    int transmit_condition;
};

struct SensorState {
    int data_requests;

    struct SensorData   data;
    struct SensorConfig config;
} sensor_states[TOF2CAN_MAX_SENSOR_COUNT];
