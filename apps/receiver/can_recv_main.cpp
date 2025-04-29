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

#include "daemon_utils.h"
#include "can_receiver.h"

CANReceiver * can_receiver;

PeriodicTaskScheduler receiver_scheduler;

t_daemon_struct       receiver_daemon;

static int receiver_start(int argc, FAR char **argv) {
    printf("[CANRecv] Starting...\n");

    can_receiver = new CANReceiver(receiver_scheduler);

    can_receiver->sched_append();
    can_receiver->on();

    printf("[CANRecv] Started with sampling frequency of 100 Hz (10 ms)\n");
    daemon_start_notify(&receiver_daemon);

    while (true) {
        //usleep(10000);
        float t = TICK2SEC((float)clock_systime_ticks());
        receiver_scheduler.run_all_tasks(t);
    }
    return 0;
}

extern "C" int can_recv_main(int argc, char *argv[]) {
    if (argc < 2) {
    error:
        printf("Usage: %s [command]\n\n", argv[0]);
        printf("Command can be:\n");
        printf("start             starts the sender\n");
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "start")) {
        if (can_receiver != NULL) {
            printf("Already started\n");
            return EXIT_FAILURE;
        }
        if (!daemon_start("can_recv", &receiver_daemon, receiver_start,
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
