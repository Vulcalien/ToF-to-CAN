#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <pthread.h>

#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <nuttx/leds/userled.h>
#include <nuttx/timers/timer.h>
#include <sched.h>

#include "can_sender.h"
#include "daemon_utils.h"
#include "params.h"

MC_CanSender         *can_sender        = NULL;

PeriodicTaskScheduler sender_scheduler;

t_daemon_struct       sender_daemon;

static int sender_start(int argc, FAR char **argv) {
    printf("[Sender] Starting...\n");

    can_sender = new MC_CanSender(sender_scheduler);

    can_sender->sched_append();
    can_sender->on();

    printf("[Sender] Started with sampling frequency of 10 Hz (100 ms)\n");
    daemon_start_notify(&sender_daemon);

    //int id = param_get_int("TOF_ID", 255);

    while (true) {
        usleep(5000);
        float t = TICK2SEC((float)clock_systime_ticks());
        sender_scheduler.run_all_tasks(t);
    }
    return 0;
}

extern "C" int sender_main(int argc, char *argv[]) {
    if (argc < 2) {
    error:
        printf("Usage: %s [command]\n\n", argv[0]);
        printf("Command can be:\n");
        printf("start             starts the sender\n");
        printf("pose 1|0          pose sender\n");
        printf("speed 1|0         speed sender\n");
        printf("speedl 1|0        L speed sender\n");
        printf("speedr 1|0        R speed sender\n");
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "start")) {
        if (can_sender != NULL) {
            printf("Already started\n");
            return EXIT_FAILURE;
        }
        if (!daemon_start("sender", &sender_daemon, sender_start,
                          SCHED_PRIORITY_MAX - 15, 2048)) {
            printf("Error in SENDER start\n");
            return EXIT_FAILURE;
        } else
            return EXIT_SUCCESS;
    }
    else
        goto error;

    return EXIT_SUCCESS;
}
