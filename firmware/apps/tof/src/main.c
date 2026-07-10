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
#include <sched.h>

#include "processing.h"
#include "can-io.h"
#include "tof.h"

bool debug_flag = false;

static void init(void) {
    while(tof_init())
        puts("[Main] ToF initialization failed: retrying");

    while(can_io_init())
        puts("[Main] CAN IO initialization failed: retrying");
}

static int task_main(int argc, char *argv[]) {
    board_userled(BOARD_GREEN_LED, true);
    board_userled(BOARD_RED_LED, true);
    init();
    board_userled(BOARD_RED_LED, false);

    while(true) {
        processing_run();
        can_io_run();
    }
    return EXIT_SUCCESS;
}

/* ================================================================== */
/*                              Commands                              */
/* ================================================================== */

static void print_help(const char *program) {
    printf("Usage: %s [command]\n\n", program);
    puts("Command can be:");
    puts("  start           start the app");
    puts("  debug           enable debugging info");
    puts("  help            prints this help message");
}

// TODO take sensor IDs as input
static int cmd_start(void) {
    static bool started = false;
    if(started) {
        puts("[Main] already started");
        return EXIT_FAILURE;
    }

    int id = task_create(
        "tof-task", SCHED_PRIORITY_MAX, 4096,
        task_main, NULL
    );

    return (id == 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int cmd_debug(void) {
    // TODO
    return EXIT_SUCCESS;
}

int tof_main(int argc, char *argv[]) {
    if(argc < 2) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    const char *cmd = argv[1];
    if(!strcmp(cmd, "start"))
        return cmd_start();

    if(!strcmp(cmd, "debug"))
        return cmd_debug();

    print_help(argv[0]);
    return EXIT_SUCCESS;
}
