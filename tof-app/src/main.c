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
#include "main.h"

#include <stdio.h>
#include <string.h>

#include "distance-sensor.h"
#include "processing.h"
#include "can-io.h"
#include "tof.h"

bool debug_flag = false;

static int cmd_help(char *arg0) {
    printf("Usage: %s [command] [args]\n", arg0);
    printf("List of available commands:\n");
    printf("    set-id      sets the sensor ID\n");
    printf("    debug       toggles debug messages\n");
    printf("    exit        exits the program\n");
    printf("    help        prints this help message\n");
    return 0;
}

int main(int argc, char *argv[]) {
    char *arg0 = (argc > 0 ? argv[0] : "<PROGRAM-NAME>");

    while(tof_init())
        printf("[Main] ToF initialization failed: retrying\n");
    while(processing_init())
        printf("[Main] Processing initialization failed: retrying\n");

    can_io_start();
    processing_start();

    // if SENSOR_ID environment variable is set, use it as sensor ID
    char *sensor_id_var = getenv("SENSOR_ID");
    if(sensor_id_var) {
        int id;
        sscanf(sensor_id_var, "%d", &id);
        can_io_set_sensor_id(id);
    }

    cmd_help(arg0);
    while(true) {
        static char cmd[128];
        scanf("%s", cmd);

        if(!strcmp(cmd, "set-id")) {
            int id;
            scanf("%d", &id);

            can_io_set_sensor_id(id);
        } else if(!strcmp(cmd, "debug")) {
            debug_flag = !debug_flag;

            const char *msg = (debug_flag ? "ENABLED" : "DISABLED");
            printf("[Main] debug messages %s\n", msg);
        } else if(!strcmp(cmd, "exit")) {
            break;
        } else {
            cmd_help(arg0);
        }
    }
    return 0;
}
