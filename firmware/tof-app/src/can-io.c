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
#include <sys/ioctl.h>
#include <nuttx/can/can.h>

#include "tof2can.h"
#include "processing.h"
#include "tof.h"

static int can_fd;
static int sensor_id;

static int transmit_timing;
static int transmit_condition;

static int data_requests = 0;

/* ================================================================== */
/*                              Receiver                              */
/* ================================================================== */

#define RECEIVER_BUFFER_SIZE (sizeof(struct can_msg_s))

static void handle_message(const struct can_msg_s *msg) {
    // TODO ignore confirmation messages???
    /*if(msg->cm_hdr.tcf)*/
        /*return;*/

    const int msg_sensor_id = msg->cm_hdr.ch_id % TOF2CAN_MAX_SENSOR_COUNT;
    const int msg_type      = msg->cm_hdr.ch_id - msg_sensor_id;

    // check if message is addressed to this device (ID=0 is broadcast)
    if(msg_sensor_id != 0 && msg_sensor_id != sensor_id)
        return;

    switch(msg_type) {
        case TOF2CAN_CONFIG_MASK_ID: {
            // check if message size is correct
            if(msg->cm_hdr.ch_dlc != TOF2CAN_CONFIG_SIZE) {
                printf(
                    "[CAN-IO] malformed config message "
                    "(size=%d, expected=%d)\n",
                    msg->cm_hdr.ch_dlc, TOF2CAN_CONFIG_SIZE
                );
                break;
            }

            printf("\n=== Configuring ===\n");
            board_userled(BOARD_GREEN_LED, true);
            processing_pause();
            data_requests = 0; // drop all data requests

            struct tof2can_config *config =
                (struct tof2can_config *) &msg->cm_data;

            // set ToF settings
            tof_set_resolution(config->resolution);
            tof_set_frequency(config->frequency);
            tof_set_sharpener(config->sharpener);

            // set processing settings
            processing_set_mode(config->processing_mode);
            processing_set_threshold(config->threshold);
            processing_set_threshold_delay(config->threshold_delay);
            processing_set_threshold_focus(config->threshold_focus);

            // set transmission settings
            can_io_set_transmit_timing(config->transmit_timing);
            can_io_set_transmit_condition(config->transmit_condition);

            processing_resume();
            board_userled(BOARD_GREEN_LED, false);
            printf("\n"); // write blank line as separator
        } break;

        case TOF2CAN_SAMPLE_MASK_ID:
        case TOF2CAN_DATA_PACKET_MASK_ID: {
            // if RTR bit is set, request a data message
            if(msg->cm_hdr.ch_rtr)
                data_requests++;
        } break;
    }
}

static void receiver_run(void) {
    static char buffer[RECEIVER_BUFFER_SIZE];
    int offset = 0;

    // read CAN message(s)
    int nbytes = read(can_fd, buffer, RECEIVER_BUFFER_SIZE);
    if(nbytes < 0)
        return;

    // handle CAN message(s)
    while(offset < nbytes) {
        struct can_msg_s *msg = (struct can_msg_s *) &buffer[offset];
        handle_message(msg);

        // move buffer offset forward
        int msglen = CAN_MSGLEN(msg->cm_hdr.ch_dlc);
        offset += msglen;
    }
}

/* ================================================================== */
/*                               Sender                               */
/* ================================================================== */

static int write_single_sample(int16_t distance, bool below_threshold) {
    struct can_msg_s msg;

    const int datalen = sizeof(struct tof2can_sample);
    const int id = TOF2CAN_SAMPLE_MASK_ID | sensor_id;

    // set CAN header
    msg.cm_hdr = (struct can_hdr_s) {
        .ch_id  = id,
        .ch_dlc = datalen,
        .ch_rtr = false,
        .ch_tcf = false
    };

    // set CAN data
    struct tof2can_sample msg_data = {
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

    const int datalen = sizeof(struct tof2can_data_packet);
    const int id = TOF2CAN_DATA_PACKET_MASK_ID | sensor_id;

    // set CAN header
    msg.cm_hdr = (struct can_hdr_s) {
        .ch_id  = id,
        .ch_dlc = datalen,
        .ch_rtr = false,
        .ch_tcf = false
    };

    const int packet_count = (length + 2) / 3;
    for(int i = 0; i < packet_count; i++) {
        struct tof2can_data_packet packet = {
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

static bool should_transmit(bool below_threshold, bool threshold_event) {
    switch(transmit_condition) {
        case TOF2CAN_CONDITION_ALWAYS_TRUE:
            return true;

        case TOF2CAN_CONDITION_BELOW_THRESHOLD:
            return below_threshold;

        case TOF2CAN_CONDITION_ABOVE_THRESHOLD:
            return !below_threshold;

        case TOF2CAN_CONDITION_ANY_THRESHOLD_EVENT:
            return threshold_event;

        case TOF2CAN_CONDITION_BELOW_THRESHOLD_EVENT:
            return threshold_event && below_threshold;

        case TOF2CAN_CONDITION_ABOVE_THRESHOLD_EVENT:
            return threshold_event && !below_threshold;
    }
    return false;
}

static int retrieve_data(int16_t *buffer, int *length, bool *below_thd) {
    bool thd_event;
    if(processing_get_data(buffer, length, below_thd, &thd_event))
        return 1;

    // check if data meets the transmit condition
    return !should_transmit(*below_thd, thd_event);
}

static void sender_run(void) {
    static int16_t buffer[PROCESSING_DATA_MAX_LENGTH];
    int buffer_length;
    bool below_threshold;

    // if timing is on-demand and there are no data requests, do nothing
    if(transmit_timing == TOF2CAN_TIMING_ON_DEMAND && data_requests == 0)
        return;

    // try to retrieve data
    if(retrieve_data(buffer, &buffer_length, &below_threshold))
        return;

    // send CAN message(s)
    board_userled(BOARD_RED_LED, true);
    if(buffer_length == 1)
        write_single_sample(buffer[0], below_threshold);
    else
        write_data_packets(buffer, buffer_length);
    board_userled(BOARD_RED_LED, false);
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

static void *can_io_run(void *arg) {
    printf("[CAN-IO] thread started\n");

    while(true) {
        receiver_run();
        sender_run();
        usleep(1000); // wait 1ms
    }
    return NULL;
}

int can_io_start(void) {
    // open CAN device in read-write mode
    can_fd = open("/dev/can0", O_RDWR | O_NONBLOCK);
    if(can_fd < 0) {
        perror("[CAN-IO] error opening /dev/can0");
        return 1;
    }
    printf("[CAN-IO] /dev/can0 opened\n");

    // print bit timing information
    print_bit_timing(can_fd);

    pthread_t thread;
    if(pthread_create(&thread, NULL, can_io_run, NULL)) {
        printf("[CAN-IO] error creating thread\n");
        return 1;
    }
    return 0;
}

int can_io_set_sensor_id(int id) {
    int err = 0;

    if(id > 0 && id < TOF2CAN_MAX_SENSOR_COUNT)
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
