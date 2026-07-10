// SPDX-License-Identifier: GPL-3.0-or-later

/* Copyright 2025 Rocco Rusponi
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

extern int tof_matrix_width; // width = height = sqrt(resolution)

extern int tof_init(void);

extern void tof_start_ranging(void);
extern void tof_stop_ranging(void);

extern int tof_read_data(int16_t **matrix, uint8_t **status_matrix);

extern int tof_set_resolution(int resolution);
extern int tof_set_frequency(int frequency_hz);
extern int tof_set_sharpener(int sharpener_percent);
