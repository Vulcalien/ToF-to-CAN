#include "can-io.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <nuttx/can/can.h>

#include "binarysem.h"

#include "distance-sensor.h"
#include "processing.h"
#include "transmit.h"
#include "tof.h"

static int can_fd;
static int sensor_id; // TODO set value

/* ================================================================== */
/*                              Receiver                              */
/* ================================================================== */

#define RECEIVER_BUFFER_SIZE (sizeof(struct can_msg_s))

static void handle_message(const struct can_msg_s *msg) {
    // TODO ignore confirmation messages???
    /*if(msg->cm_hdr.tcf)*/
        /*return;*/

    const int msg_sensor_id = msg->cm_hdr.ch_id % DISTANCE_SENSOR_MAX_COUNT;
    const int msg_type      = msg->cm_hdr.ch_id - msg_sensor_id;

    // check if message is addressed to this device (ID=0 is broadcast)
    if(msg_sensor_id != 0 && msg_sensor_id != sensor_id)
        return;

    switch(msg_type) {
        case DISTANCE_SENSOR_CAN_CONFIG_MASK_ID: {
            // keep data ranging paused during configuration
            tof_stop_ranging();

            struct distance_sensor_can_config *config =
                (struct distance_sensor_can_config *) &msg->cm_data;

            // set ToF settings
            tof_set_resolution(config->resolution);
            tof_set_ranging_frequency(config->ranging_frequency);
            tof_set_ranging_mode(config->ranging_mode);
            tof_set_sharpener(config->sharpener);

            // set processing settings
            processing_set_mode(config->processing_mode);
            processing_set_threshold(config->threshold);
            processing_set_threshold_delay(config->threshold_delay);

            // set transmission settings
            transmit_set_timing(config->transmit_timing);

            // resume data ranging
            tof_start_ranging();
        } break;

        case DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID: {
            // check if RTR bit is set
            if(msg->cm_hdr.ch_rtr) {
                // if transmit timing is on-demand, request sample
                if(transmit_timing == TRANSMIT_ON_DEMAND)
                    binarysem_post(&processing_request_sample);
            }
        } break;
    }
}

static void *receiver_run(void *arg) {
    printf("[CAN-IO] receiver thread started\n");

    static char buffer[RECEIVER_BUFFER_SIZE];
    while(true) {
        int offset = 0;

        // read CAN message(s)
        int nbytes = read(can_fd, buffer, RECEIVER_BUFFER_SIZE);
        if(nbytes < 0) {
            printf("[CAN-IO] error reading from CAN device\n");
            continue;
        }

        // handle CAN message(s)
        while(offset < nbytes) {
            struct can_msg_s *msg = (struct can_msg_s *) &buffer[offset];
            handle_message(msg);

            // move buffer offset forward
            int msglen = CAN_MSGLEN(msg->cm_hdr.ch_dlc);
            offset += msglen;
        }
    }
    return NULL;
}

/* ================================================================== */
/*                               Sender                               */
/* ================================================================== */

static inline int write_distance(void) {
    // lock data mutex
    int err = pthread_mutex_lock(&processing_data_mutex);
    if(err) {
        printf("[CAN-IO] error trying to lock data mutex\n");
        return err;
    }

    const int  distance         = processing_distance;
    const bool threshold_status = processing_threshold_status;

    // unlock data mutex
    err = pthread_mutex_unlock(&processing_data_mutex);
    if(err) {
        printf("[CAN-IO] error trying to unlock data mutex\n");
        return err;
    }

    const int datalen = sizeof(struct distance_sensor_can_sample);
    const int id = DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID | sensor_id;

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
    int nbytes = write(can_fd, &msg, msglen);
    if(nbytes != msglen) {
        // TODO handle error
    }
    return 0;
}

static void *sender_run(void *arg) {
    printf("[CAN-IO] sender thread started\n");

    while(true) {
        // wait for a sample to become available
        binarysem_wait(&processing_sample_available);

        // send CAN message
        if(processing_selector == PROCESSING_SELECTOR_ALL) {
            // send multiple messages to transmit the whole area
            ;// TODO
        } else {
            // write a single distance point with threshold status
            write_distance();
        }

        // if transmit timing is continuous, request sample
        if(transmit_timing == TRANSMIT_CONTINUOUS)
            binarysem_post(&processing_request_sample);
    }
    return NULL;
}

/* ================================================================== */

static inline void print_bit_timing(int fd) {
    struct canioc_bittiming_s bt;
    int ret = ioctl(
        fd, CANIOC_GET_BITTIMING,
        (unsigned long) ((uintptr_t) &bt)
    );

    if(ret < 0) {
        printf("[CAN-IO] bit timing not available\n");
    } else {
        printf("[CAN-IO] bit timing:\n");
        printf("   Baud: %lu\n", (unsigned long) bt.bt_baud);
        printf("  TSEG1: %u\n", bt.bt_tseg1);
        printf("  TSEG2: %u\n", bt.bt_tseg2);
        printf("    SJW: %u\n", bt.bt_sjw);
    }
}

int can_io_start(void) {
    // open CAN device in read-write mode
    can_fd = open("/dev/can0", O_RDWR | O_NOCTTY);
    if(can_fd < 0) {
        perror("[CAN-IO] error opening /dev/can0");
        return 1;
    }
    printf("[CAN-IO] /dev/can0 opened\n");

    // print bit timing information
    print_bit_timing(can_fd);

    // create receiver and sender threads
    pthread_t receiver_thread;
    pthread_t sender_thread;

    if(pthread_create(&receiver_thread, NULL, receiver_run, NULL)) {
        printf("[CAN-IO] error creating receiver thread\n");
        return 1;
    }
    if(pthread_create(&sender_thread, NULL, sender_run, NULL)) {
        printf("[CAN-IO] error creating sender thread\n");
        return 1;
    }
    return 0;
}
