#include "sender.h"

#include <stdio.h>
#include <fcntl.h>
#include <nuttx/can/can.h>

void *sender_run(void *arg) {
    int can_fd = open("/dev/can0", O_WRONLY | O_NOCTTY);
    if(can_fd < 0) {
        perror("Sender: /dev/can0 open");
        printf("Sender: stopping");
        return NULL;
    }

    // TODO bit timing

    while(true) {
        struct can_msg_s msg;
        // TODO set msg data

        // TODO
    }
    return NULL;
}
