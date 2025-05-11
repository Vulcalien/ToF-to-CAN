#include "receiver.h"

#include <stdio.h>
#include <fcntl.h>
#include <nuttx/can/can.h>

#include "distance-sensor.h"

static void handle_message(struct can_msg_s *msg) {
    // TODO ignore confirmation messages???
    /*if(msg->cm_hdr.tcf)*/
        /*return;*/

    const int msg_sensor_id = msg->cm_hdr.ch_id % DISTANCE_SENSOR_MAX_COUNT;
    const int msg_type      = msg->cm_hdr.ch_id - msg_sensor_id;

    // check if message is addressed to this device (ID=0 is broadcast)
    if(msg_sensor_id != 0 && msg_sensor_id != /*TODO*/0)
        return;

    switch(msg_type) {
        case DISTANCE_SENSOR_CAN_CONFIG_MASK_ID: {
            struct distance_sensor_can_config *config =
                (struct distance_sensor_can_config *) &msg->cm_data;

            // TODO apply config
        } break;

        case DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID: {
            // check if message has 'Remote Transmit Request' bit set
            if(msg->cm_hdr.ch_rtr) {
                // TODO if transmit mode is on-demand, request sample
            }
        } break;
    }
}

void *receiver_run(void *arg) {
    int fd = open("/dev/can0", O_RDONLY | O_NOCTTY);
    if(fd < 0) {
        perror("Receiver: /dev/can0 open");
        printf("Receiver: stopping\n");
        return NULL;
    }

    // TODO bit timing

    while(true) {
        struct can_msg_s msg;
        // TODO read fd

        handle_message(&msg);
    }
    return NULL;
}
