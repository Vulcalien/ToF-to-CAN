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
#include "visualizer.h"

#include <stdio.h>

#include <pthread.h>

#include "libtofcan.h"
#include "can-io.h"
#include "display.h"
#include "view.h"

int main(int argc, char *argv[]) {
    if(display_init()) {
        puts("Display initialization failed");
        return 1;
    }

    pthread_t can_io_thread;
    pthread_create(&can_io_thread, NULL, can_io_start, NULL);

    view_set(&view_grid);
    while(!display_tick()) {
        display_update();

        // sleep 10 ms
        nanosleep(&(struct timespec) { .tv_nsec = 10000000 }, NULL);
    }
    return 0;
}
