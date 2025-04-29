/*
 * dds_main.cpp
 */

#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dds.h"

bool dds_started = false;

extern "C" int dds_main(int argc, char *argv[])
{
    if (argc < 2) {
    error:
        printf("Usage: %s [command]\n\n", argv[0]);
        printf("Command can be:\n");
        printf("start                  starts the DDS\n");
        printf("list                   lists variables in the DDS\n");
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "start")) {
        if (dds_started != NULL) {
            printf("Already started\n");
            return EXIT_FAILURE;
        }
        dds_started = true;
        DDS::init();
        return EXIT_SUCCESS;
    }
    else if (!dds_started) {
        printf("Not started\n");
        return EXIT_FAILURE;
    }
    else if (!strcmp(argv[1], "list")) {
        DDS::show();
    }
    else if (!strcmp(argv[1], "get") && argc == 3) {
        DDS::display(argv[2]);
    }
    else
        goto error;

    return EXIT_SUCCESS;
}
