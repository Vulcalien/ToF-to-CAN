#include "main.h"

#include <stdio.h>
#include <string.h>

#include "distance-sensor.h"
#include "processing.h"
#include "can-io.h"
#include "tof.h"

static int cmd_start(void) {
    while(tof_init())
        printf("[Main] ToF initialization failed: retrying\n");
    while(processing_init())
        printf("[Main] Processing initialization failed: retrying\n");

    can_io_start();
    processing_start();
    return 0;
}

static int cmd_set_id(int id) {
    // TODO
    return 0;
}

static int cmd_help(char *arg0) {
    printf("Usage: %s [command] [args]\n", arg0);
    printf("\n");
    printf("List of available commands:\n");
    printf("    start       starts sampling and activates CAN I/O\n");
    printf("    set-id      permanently sets the sensor ID\n");
    printf("    help        prints this help message\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        cmd_help(argc < 1 ? "<PROGRAM-NAME>" : argv[0]);
        return 1;
    }
    char *cmd = argv[1];

    // start
    if(!strcmp(cmd, "start"))
        return cmd_start();

    // set-id <id>
    if(!strcmp(cmd, "set-id")) {
        int id = 0;
        if(argc >= 3)
            id = atoi(argv[2]);

        if(id <= 0 || id >= DISTANCE_SENSOR_MAX_COUNT) {
            printf(
                "Error: please specify a sensor ID in the range [1, %d]\n",
                DISTANCE_SENSOR_MAX_COUNT - 1
            );
            return 1;
        }
        return cmd_set_id(id);
    }

    // help
    if(!strcmp(cmd, "help"))
        return cmd_help(argv[0]);

    cmd_help(argv[0]);
    return 1;
}
