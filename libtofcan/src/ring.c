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
                           struct libtofcan_ring_point *diagram,
                           int diagram_size, int ring_radius,
                           int max_age) {
    ring->diagram = diagram;
    ring->diagram_size = diagram_size;
    ring->radius = ring_radius;
    ring->max_age = max_age;
}

void libtofcan_ring_reset(struct libtofcan_ring *ring) {
    for(int i = 0; i < ring->diagram_size; i++) {
        ring->diagram[i] = (struct libtofcan_ring_point) {
            .distance = -1,
            .age = ring->max_age
        };
    }
}

struct Polar {
    double angle;
    int distance;
};

static double angle_of_point(int index, int count) {
    double normalized = (double) index / (count - 1); // [0, 1]
    return (normalized - 0.5) * M_PI_4; // [-22.5, +22.5] degrees
}

static void get_absolute_polar(struct Polar *result,
                               const struct Polar *input,
                               int ring_radius, double sensor_angle) {
    // convert input from polar to cartesian
    double x = input->distance * cos(input->angle);
    double y = input->distance * sin(input->angle);

    // add ring radius
    x += ring_radius;

    // convert back to polar
    result->angle = atan2(y, x);
    result->distance = sqrt(x * x + y * y);

    // add sensor angle
    result->angle += sensor_angle;

    // normalize angle in [0, 2 PI)
    while(result->angle < 0)
        result->angle += 2 * M_PI;
    while(result->angle >= 2 * M_PI)
        result->angle -= 2 * M_PI;
}

void libtofcan_ring_insert(struct libtofcan_ring *ring,
                           const struct libtofcan_batch *batch,
                           double angle) {
    double angle_per_cell = (2 * M_PI) / ring->diagram_size;

    for(int i = 0; i < batch->data_length; i++) {
        // ignore invalid points, leaving any previous data intact
        if(batch->data[i] == -1)
            continue;

        struct Polar input = {
            .angle = angle_of_point(i, batch->data_length),
            .distance = batch->data[i]
        };

        struct Polar point;
        get_absolute_polar(&point, &input, ring->radius, angle);

        int diagram_index = point.angle / angle_per_cell;
        ring->diagram[diagram_index] = (struct libtofcan_ring_point) {
            .distance = point.distance,
            .age = 0
        };
    }

    // increment age of diagram points and invalidate old ones
    for(int i = 0; i < ring->diagram_size; i++) {
        if(ring->diagram[i].age < ring->max_age)
            ring->diagram[i].age++;
        else
            ring->diagram[i].distance = -1;
    }
}
