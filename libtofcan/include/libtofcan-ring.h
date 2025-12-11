/* Copyright 2025 Vulcalien
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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "libtofcan.h"

struct libtofcan_ring_point {
    int16_t distance;
    int16_t age;
};

struct libtofcan_ring {
    struct libtofcan_ring_point *diagram;
    int diagram_size;
    int radius;
    int max_age;
};

// * diagram_size: number of data points (size of 'diagram' array)
// * ring_radius: distance from ring center to sensors
// * max_age: number of calls to 'libtofcan_ring_insert' points survive
extern void libtofcan_ring_config(struct libtofcan_ring *ring,
                                  struct libtofcan_ring_point *diagram,
                                  int diagram_size, int ring_radius,
                                  int max_age);

extern void libtofcan_ring_reset(struct libtofcan_ring *ring);

extern void libtofcan_ring_insert(struct libtofcan_ring *ring,
                                  const struct libtofcan_batch *batch,
                                  double angle);

#ifdef __cplusplus
}
#endif
