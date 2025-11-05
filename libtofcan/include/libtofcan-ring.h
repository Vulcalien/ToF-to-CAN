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

#include <stdint.h>
#include <stdbool.h>

#include "libtofcan.h"

struct libtofcan_ring {
    int16_t *diagram;
    int diagram_size;
    int radius;
};

extern void libtofcan_ring_config(struct libtofcan_ring *ring,
                                  int16_t *diagram, int diagram_size,
                                  int ring_radius);

extern void libtofcan_ring_reset(struct libtofcan_ring *ring);

extern void libtofcan_ring_insert(struct libtofcan_ring *ring,
                                  const struct libtofcan_batch *batch,
                                  double angle);
