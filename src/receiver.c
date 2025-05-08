#include "receiver.h"

#include <stdio.h>
#include <fcntl.h>
#include <nuttx/can/can.h>

static void handle_message(struct can_msg_s *msg) {
    // TODO ignore confirmation messages???
    /*if(msg->cm_hdr.tcf)*/
        /*return;*/

    switch(msg->cm_hdr.ch_id) {
        // TODO
    }
}

void *receiver_run(void *arg) {
    int fd = open("/dev/can0", O_RDONLY | O_NOCTTY);
    if(fd < 0) {
        perror("Receiver: /dev/can0 open");
        printf("Receiver: stopping");
        return NULL;
    }

    // TODO bit timing

    while(true) {
        struct can_msg_s msg;
        // TODO read fd

        handle_message(&message);
    }
    return NULL;
}
