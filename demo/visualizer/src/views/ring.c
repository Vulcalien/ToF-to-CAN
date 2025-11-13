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
#include "display.h"
#include "can-io.h"

#define DIAGRAM_SIZE 64

#define RING_RADIUS 0
#define RING_SENSOR_COUNT 10

#define DIAGRAM_XC (DISPLAY_WIDTH / 2)
#define DIAGRAM_YC (DISPLAY_HEIGHT / 2)

static struct libtofcan_ring ring;

static int ring_init(void) {
    static int16_t diagram[DIAGRAM_SIZE];
    libtofcan_ring_config(&ring, diagram, DIAGRAM_SIZE, RING_RADIUS);
    libtofcan_ring_reset(&ring);

    return 0;
}

static inline void write_value(int val, int bg_color, int x, int y) {
    char text[16];
    snprintf(text, sizeof(text), "%d", val);
    display_write_small(text, bg_color, x, y);
}

static bool ring_update(SDL_Renderer *renderer) {
    bool new_data = false;
    while(true) {
        int sensor;
        struct libtofcan_batch batch;
        if(can_io_get_data(&sensor, &batch))
            break;

        // insert batch into diagram
        double angle = (sensor - 1) * (2 * M_PI / RING_SENSOR_COUNT);
        libtofcan_ring_insert(&ring, &batch, angle);

        new_data = true;
    }

    // if no new data was added, don't refresh
    if(!new_data)
        return false;

    for(int i = 0; i < DIAGRAM_SIZE; i++) {
        double angle = i * (2 * M_PI) / DIAGRAM_SIZE;
        int distance = ring.diagram[i];

        int x = DIAGRAM_XC + cos(angle) * distance + 0.5;
        int y = DIAGRAM_YC + sin(angle) * distance + 0.5;

        // draw a line from center to square
        SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 255);
        SDL_RenderDrawLine(renderer, DIAGRAM_XC, DIAGRAM_YC, x, y);

        // draw a small square
        SDL_Rect rect = { x - 2, y - 2, 5, 5 };
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 255);
        SDL_RenderFillRect(renderer, &rect);

        // write value number on top of point
        write_value(
            distance, 0x000000,
            x + cos(angle) * 12,
            y + sin(angle) * 12
        );
    }
    return true;
}

static void ring_keypress(struct DisplayInput *input) {
}

const struct View view_ring = {
    .init = ring_init,
    .update = ring_update,
    .keypress = ring_keypress
};
