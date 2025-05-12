#include "sender.h"

#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <nuttx/can/can.h>

#include "distance-sensor.h"
#include "transmission.h"
#include "processing.h"

static inline void write_distance(int fd, int distance, bool threshold_status) {
    const int datalen = sizeof(struct distance_sensor_can_sample);
    const int id = DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID; // TODO sensor ID

    struct can_msg_s msg;

    // set CAN header
    msg.cm_hdr = (struct can_hdr_s) {
        .ch_id  = id,
        .ch_dlc = datalen,
        .ch_rtr = false,
        .ch_tcf = false
    };

    // set CAN data
    struct distance_sensor_can_sample data = {
        .distance        = distance,
        .below_threshold = threshold_status
    };
    memcpy(msg.cm_data, &data, datalen);

    // write CAN message
    int msglen = CAN_MSGLEN(datalen);
    int nbytes = write(fd, &msg, msglen);
    if(nbytes != msglen) {
        // TODO handle error
    }
}

void *sender_run(void *arg) {
    int fd = open("/dev/can0", O_WRONLY | O_NOCTTY);
    if(fd < 0) {
        perror("Sender: /dev/can0 open");
        printf("Sender: stopping\n");
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

        if(transmission_timing == TRANSMISSION_TIMING_CONTINUOUS)
            sem_post(&processing_request_sample);
    }
    return NULL;
}
