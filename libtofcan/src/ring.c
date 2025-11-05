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
#include "libtofcan-ring.h"

#include <stdlib.h>
#include <math.h>

void libtofcan_ring_config(struct libtofcan_ring *ring,
                           int16_t *diagram, int diagram_size,
                           int ring_radius) {
    ring->diagram = diagram;
    ring->diagram_size = diagram_size;
    ring->radius = ring_radius;
}

void libtofcan_ring_reset(struct libtofcan_ring *ring) {
    for(int i = 0; i < ring->diagram_size; i++)
        ring->diagram[i] = -1;
}

struct Polar {
    double angle;
    int distance;
};

static void get_absolute_polar(struct Polar *result,
                               const struct Polar *input,
                               int sensor_x, int sensor_y,
                               int sensor_angle) {
    // convert input from polar to cartesian
    int x = input->distance * cos(input->angle) + 0.5;
    int y = input->distance * sin(input->angle) + 0.5;

    // add sensor origin
    x += sensor_x;
    y += sensor_y;

    // convert back to polar
    result->angle = atan2(y, x);
    result->distance = sqrt(x * x + y * y);

    // add sensor angle
    result->angle += sensor_angle;
}

void libtofcan_ring_insert(struct libtofcan_ring *ring,
                           const struct libtofcan_batch *batch,
                           double angle) {
    const int x0 = ring->radius * cos(angle) + 0.5;
    const int y0 = ring->radius * sin(angle) + 0.5;

    double angle_per_sample = M_PI_4 / batch->data_length;
    double angle_per_cell = (2 * M_PI) / ring->diagram_size;

    for(int i = 0; i < batch->data_length; i++) {
        struct Polar input = {
            .angle = (i + 0.5) * angle_per_sample - M_PI_4 / 2,
            .distance = batch->data[i]
        };

        struct Polar point;
        get_absolute_polar(
            &point, &input,
            x0, y0, angle
        );

        int diagram_index = point.angle / angle_per_cell;
        int16_t *diagram_cell = &ring->diagram[diagram_index];
        if(*diagram_cell == -1 || *diagram_cell > point.distance)
            *diagram_cell = point.distance;
    }
}
