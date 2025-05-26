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
#include "tof.h"

#define TIMING_ON_DEMAND  0
#define TIMING_CONTINUOUS 1

#define CONDITION_ALWAYS          0
#define CONDITION_BELOW_THRESHOLD 1
#define CONDITION_ABOVE_THRESHOLD 2
#define CONDITION_THRESHOLD_EVENT 3

static int can_fd;
static int sensor_id; // TODO set value

static int transmit_timing;
static int transmit_condition;

static sem_t request_data;

/* ================================================================== */
/*                              Receiver                              */
/* ================================================================== */

#define RECEIVER_BUFFER_SIZE (sizeof(struct can_msg_s))

static inline int set_transmit_timing(int timing) {
    int err = 0;
    if(timing >= 0 && timing < 2)
        transmit_timing = timing;
    else
        err = 1;

    // TODO if switching to continuous mode, send a data request to
    // unlock the semaphore wait.

    printf(
        "[CAN-IO] setting transmit timing to %d (err=%d)\n",
        timing, err
    );
    return err;
}

static inline int set_transmit_condition(int condition) {
    int err = 0;
    if(condition >= 0 && condition < 4)
        transmit_condition = condition;
    else
        err = 1;

    printf(
        "[CAN-IO] setting transmit condition to %d (err=%d)\n",
        condition, err
    );
    return err;
}

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
            set_transmit_timing(config->transmit_timing);
            set_transmit_condition(config->transmit_condition);

            // resume data ranging
            tof_start_ranging();
        } break;

        case DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID: {
            // if RTR bit is set, request data
            if(msg->cm_hdr.ch_rtr)
                sem_post(&request_data);
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

static int write_single_sample(short distance, bool threshold_status) {
    struct can_msg_s msg;

    const int datalen = sizeof(struct distance_sensor_can_sample);
    const int id = DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID | sensor_id;

    // set CAN header
    msg.cm_hdr = (struct can_hdr_s) {
        .ch_id  = id,
        .ch_dlc = datalen,
        .ch_rtr = false,
        .ch_tcf = false
    };

    // set CAN data
    struct distance_sensor_can_sample msg_data = {
        .distance        = distance,
        .below_threshold = threshold_status
    };
    memcpy(msg.cm_data, &msg_data, datalen);

    // write CAN message
    const int msglen = CAN_MSGLEN(datalen);
    const int nbytes = write(can_fd, &msg, msglen);
    if(nbytes != msglen) {
        // TODO handle error
    }
    return 0;
}

static void retrieve_data(short *data, bool *threshold_status) {
    // wait for data to become available
    binarysem_wait(&processing_data_available);

    // lock data mutex
    if(pthread_mutex_lock(&processing_data_mutex)) {
        printf("[CAN-IO] error trying to lock data mutex\n");
        return;
    }

    // copy data from processing module
    memcpy(data, processing_data, processing_data_length * sizeof(short));
    *threshold_status = processing_threshold_status;

    // unlock data mutex
    if(pthread_mutex_unlock(&processing_data_mutex)) {
        printf("[CAN-IO] error trying to unlock data mutex\n");
        return;
    }
}

static void *sender_run(void *arg) {
    printf("[CAN-IO] sender thread started\n");

    static short data[PROCESSING_DATA_MAX_LENGTH];
    bool threshold_status;

    while(true) {
        // wait for a request
        sem_wait(&request_data);

        // retrieve data
        retrieve_data(data, &threshold_status);

        // send CAN message(s)
        if(processing_data_length == 1) {
            write_single_sample(data[0], threshold_status);
        } else {
            // TODO send multiple messages to transmit all data
        }

        // if timing is continuous, request more data
        if(transmit_timing == TIMING_CONTINUOUS)
            sem_post(&request_data);
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

    // initialize request data semaphore
    if(sem_init(&request_data, 0, 0)) {
        printf("[CAN-IO] error initializing request data semaphore\n");
        return 1;
    }

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
