// SPDX-License-Identifier: GPL-3.0-or-later

/* Copyright 2025 Rocco Rusponi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "can-io.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <nuttx/can/can.h>

#include "distance-sensor.h"
#include "processing.h"
#include "tof.h"

#define TIMING_ON_DEMAND  0
#define TIMING_CONTINUOUS 1

#define CONDITION_ALWAYS_TRUE           0
#define CONDITION_BELOW_THRESHOLD_EVENT 1
#define CONDITION_ABOVE_THRESHOLD_EVENT 2
#define CONDITION_ANY_THRESHOLD_EVENT   3

static int can_fd;
static int sensor_id;

static int transmit_timing;
static int transmit_condition;

static sem_t request_data_message;

static void sender_pause(void);
static void sender_resume(void);

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
            // check if message size is correct
            if(msg->cm_hdr.ch_dlc != DISTANCE_SENSOR_CAN_CONFIG_SIZE) {
                printf(
                    "[CAN-IO] malformed config message "
                    "(size=%d, expected=%d)\n",
                    msg->cm_hdr.ch_dlc, DISTANCE_SENSOR_CAN_CONFIG_SIZE
                );
                break;
            }

            printf("\n=== Configuring ===\n");
            sender_pause(); // pause sender thread

            struct distance_sensor_can_config *config =
                (struct distance_sensor_can_config *) &msg->cm_data;

            // set ToF settings
            tof_set_resolution(config->resolution);
            tof_set_frequency(config->frequency);
            tof_set_sharpener(config->sharpener);

            // set processing settings
            processing_set_mode(config->processing_mode);
            processing_set_threshold(config->threshold);
            processing_set_threshold_delay(config->threshold_delay);

            // set transmission settings
            can_io_set_transmit_timing(config->transmit_timing);
            can_io_set_transmit_condition(config->transmit_condition);

            sender_resume(); // resume sender thread
            printf("\n"); // write blank line as separator
        } break;

        case DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID:
        case DISTANCE_SENSOR_CAN_DATA_PACKET_MASK_ID: {
            // if RTR bit is set, request a data message
            if(msg->cm_hdr.ch_rtr)
                sem_post(&request_data_message);
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

static bool is_sender_paused = true; // after startup, device is idle

static void sender_pause(void) {
    processing_pause();

    // drop all data message requests
    while(sem_trywait(&request_data_message) == 0);

    // pause sender thread
    is_sender_paused = true;
    usleep(10000); // wait 10ms to ensure the sender blocks
}

static void sender_resume(void) {
    is_sender_paused = false;
    processing_resume();

    // if timing is continuous, unblock the sender waiting for a request
    if(transmit_timing == TIMING_CONTINUOUS)
        sem_post(&request_data_message);
}

static int write_single_sample(int16_t distance, bool below_threshold) {
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
        .below_threshold = below_threshold
    };
    memcpy(msg.cm_data, &msg_data, datalen);

    // write CAN message
    const int msglen = CAN_MSGLEN(datalen);
    const int nbytes = write(can_fd, &msg, msglen);
    if(nbytes != msglen) {
        printf("[CAN-IO] error writing to CAN device\n");
        return 1;
    }
    return 0;
}

static int write_data_packets(int16_t *data, int length) {
    static int batch_id = 0;
    batch_id++;

    struct can_msg_s msg;

    const int datalen = sizeof(struct distance_sensor_can_data_packet);
    const int id = DISTANCE_SENSOR_CAN_DATA_PACKET_MASK_ID | sensor_id;

    // set CAN header
    msg.cm_hdr = (struct can_hdr_s) {
        .ch_id  = id,
        .ch_dlc = datalen,
        .ch_rtr = false,
        .ch_tcf = false
    };

    const int packet_count = (length + 2) / 3;
    for(int i = 0; i < packet_count; i++) {
        struct distance_sensor_can_data_packet packet = {
            .sequence_number = i,
            .batch_id        = batch_id,
            .last_of_batch   = (i == packet_count - 1),
        };

        // set packet sample data
        packet.data_length = 0;
        for(int s = 0; s < 3; s++) {
            const int sample_index = i * 3 + s;
            if(sample_index >= length)
                break;

            packet.data[s] = data[sample_index];
            packet.data_length++;
        }

        // set CAN data
        memcpy(msg.cm_data, &packet, datalen);

        // write CAN message
        const int msglen = CAN_MSGLEN(datalen);
        const int nbytes = write(can_fd, &msg, msglen);
        if(nbytes != msglen) {
            printf("[CAN-IO] error writing to CAN device\n");
            return 1;
        }
    }
    return 0;
}

static bool should_transmit(int buffer_length,
                            bool below_threshold,
                            bool threshold_event) {
    // if there are multiple data points, ignore transmit condition
    if(buffer_length > 1)
        return true;

    switch(transmit_condition) {
        case CONDITION_ALWAYS_TRUE:
            return true;

        case CONDITION_BELOW_THRESHOLD_EVENT:
            return threshold_event && below_threshold;

        case CONDITION_ABOVE_THRESHOLD_EVENT:
            return threshold_event && !below_threshold;

        case CONDITION_ANY_THRESHOLD_EVENT:
            return threshold_event;
    }
    return false;
}

static int retrieve_data(int16_t *buffer, int *length,
                         bool *below_threshold) {
    while(true) {
        // if sender was paused while waiting, do not retrieve data
        if(is_sender_paused)
            return 1;

        bool threshold_event;
        int err = processing_get_data(
            buffer, length, below_threshold, &threshold_event
        );

        // if data was not available, wait some time and try again
        if(err) {
            usleep(1000); // wait 1ms
            continue;
        }

        // if data should be transmitted, break the loop
        if(should_transmit(*length, *below_threshold, threshold_event))
            break;
    }
    return 0;
}

static void *sender_run(void *arg) {
    printf("[CAN-IO] sender thread started\n");

    static int16_t buffer[PROCESSING_DATA_MAX_LENGTH];
    int buffer_length;
    bool below_threshold;

    while(true) {
        // if paused or timing is on-demand, wait for a request
        if(is_sender_paused || transmit_timing == TIMING_ON_DEMAND)
            sem_wait(&request_data_message);

        // retrieve data; on failure, drop the request
        if(retrieve_data(buffer, &buffer_length, &below_threshold))
            continue;

        // send CAN message(s)
        if(buffer_length == 1)
            write_single_sample(buffer[0], below_threshold);
        else
            write_data_packets(buffer, buffer_length);
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

    // initialize request data message semaphore
    if(sem_init(&request_data_message, 0, 0)) {
        printf("[CAN-IO] error initializing request data message semaphore\n");
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

int can_io_set_sensor_id(int id) {
    int err = 0;

    if(id > 0 && id < DISTANCE_SENSOR_MAX_COUNT)
        sensor_id = id;
    else
        err = 1;

    printf(
        "[CAN-IO] setting sensor ID to %d (err=%d)\n",
        id, err
    );
    return err;
}

int can_io_set_transmit_timing(int timing) {
    int err = 0;
    if(timing >= 0 && timing < 2)
        transmit_timing = timing;
    else
        err = 1;

    printf(
        "[CAN-IO] setting transmit timing to %d (err=%d)\n",
        timing, err
    );
    return err;
}

int can_io_set_transmit_condition(int condition) {
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
