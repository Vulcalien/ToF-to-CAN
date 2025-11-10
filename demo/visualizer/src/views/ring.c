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
#include "view.h"

#include <math.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "libtofcan.h"
#include "libtofcan-ring.h"
#include "can-io.h"

#define DIAGRAM_SIZE 64

#define RING_RADIUS 0
#define RING_SENSOR_COUNT 10

static struct libtofcan_ring ring;

static int ring_init(void) {
    static int16_t diagram[DIAGRAM_SIZE];
    libtofcan_ring_config(&ring, diagram, DIAGRAM_SIZE, RING_RADIUS);
    libtofcan_ring_reset(&ring);

    return 0;
}

static int ring_update(SDL_Renderer *renderer, TTF_Font *font) {
    while(true) {
        int sensor;
        struct libtofcan_batch batch;
        if(can_io_get_data(&sensor, &batch))
            break;

        // insert batch into diagram
        double angle = (sensor - 1) * (2 * M_PI / RING_SENSOR_COUNT);
        libtofcan_ring_insert(&ring, &batch, angle);
    }

    // TODO

    return 0;
}

const struct View view_ring = {
    .init = ring_init,
    .update = ring_update
};
