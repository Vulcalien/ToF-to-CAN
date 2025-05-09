#include "sender.h"

#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <nuttx/can/can.h>

#include "processing.h"

static inline void write_distance(int fd, int distance, bool threshold_status) {
    struct can_msg_s msg;

    // set CAN header
    msg.cm_hdr = (struct can_hdr_s) {
        .ch_id  = 0, // TODO
        .ch_dlc = 0, // TODO msg size
        .ch_rtr = false,
        .ch_tcf = false
    };

    // set CAN data
    // TODO

    // TODO write to fd
}

void *sender_run(void *arg) {
    int fd = open("/dev/can0", O_WRONLY | O_NOCTTY);
    if(fd < 0) {
        perror("Sender: /dev/can0 open");
        printf("Sender: stopping");
        return NULL;
    }

    // TODO bit timing

    while(true) {
        // wait for a sample to become available
        sem_wait(&processing_sample_available);

        // read distance and threshold status variables
        const int  distance         = processing_distance;
        const bool threshold_status = processing_threshold_status;

        // send CAN message
        write_distance(fd, distance, threshold_status);

        // TODO request new sample?
    }
    return NULL;
}
