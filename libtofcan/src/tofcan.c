/* Copyright 2025 Vulcalien
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
#include "libtofcan.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tof2can.h"

struct {
    void (*sample)(int sensor, struct tof2can_sample *data);
    void (*batch)(int sensor, struct libtofcan_batch *data, bool valid);
} callbacks;

void libtofcan_set_callbacks(
    void (*sample)(int sensor, struct tof2can_sample *data),
    void (*batch)(int sensor, struct libtofcan_batch *data, bool valid)
) {
    callbacks.sample = sample;
    callbacks.batch = batch;
}

/* ================================================================== */
/*                          config & request                          */
/* ================================================================== */

void libtofcan_config(int sensor, struct libtofcan_msg *msg,
                      struct tof2can_config *config) {
    msg->id  = TOF2CAN_CONFIG_MASK_ID | sensor;
    msg->rtr = false;
    msg->len = TOF2CAN_CONFIG_SIZE;
    memcpy(msg->data, config, TOF2CAN_CONFIG_SIZE);
}

void libtofcan_request(int sensor, struct libtofcan_msg *msg) {
    msg->id  = TOF2CAN_SAMPLE_MASK_ID | sensor;
    msg->rtr = true;
    msg->len = 0;
}

/* ================================================================== */
/*                              receiver                              */
/* ================================================================== */

static void publish_sample(int sensor, struct tof2can_sample *data) {
    if(callbacks.sample)
        callbacks.sample(sensor, data);
}

static void publish_batch(int sensor, struct libtofcan_batch *data,
                          bool valid) {
    if(callbacks.batch)
        callbacks.batch(sensor, data, valid);
}

static void handle_sample(int sensor, const void *data, int len) {
    // check if message size is correct
    if(len != sizeof(struct tof2can_sample))
        return;

    struct tof2can_sample sample;
    memcpy(&sample, data, sizeof(sample));

    publish_sample(sensor, &sample);
}

static void batch_reset(struct libtofcan_batch *batch,
                        int sensor, int batch_id) {
    // if previous batch was interrupted, send it as invalid
    if(batch->packets_received != batch->packets_expected)
        publish_batch(sensor, batch, false);

    batch->data_length      = 0;
    batch->batch_id         = batch_id;
    batch->packets_received = 0;
    batch->packets_expected = 0;
}

static void batch_insert(struct libtofcan_batch *batch,
                         struct tof2can_data_packet *packet) {
    const int buffer_length = sizeof(batch->data) / sizeof(int16_t);
    const int offset = packet->sequence_number * 3;

    // check if packet data would overflow the buffer
    if(offset + packet->data_length > buffer_length)
        return;

    // copy packet data into buffer
    batch->packets_received++;
    batch->data_length += packet->data_length;
    memcpy(
        &batch->data[offset],
        &packet->data,
        packet->data_length * sizeof(int16_t)
    );

    // if packet is last of batch, set count of expected packets
    if(packet->last_of_batch)
        batch->packets_expected = 1 + packet->sequence_number;
}

static void handle_data_packet(int sensor, const void *data, int len) {
    static struct libtofcan_batch batches[TOF2CAN_MAX_SENSOR_COUNT];

    // check if message size is correct
    if(len != sizeof(struct tof2can_data_packet))
        return;

    struct libtofcan_batch *batch = &batches[sensor];
    struct tof2can_data_packet packet;
    memcpy(&packet, data, sizeof(packet));

    // if packet has a new batch ID, reset the batch buffer
    if(batch->batch_id != packet.batch_id)
        batch_reset(batch, sensor, packet.batch_id);

    // insert new data into the batch buffer
    batch_insert(batch, &packet);

    // if all packets have been received, send the batch
    if(batch->packets_received == batch->packets_expected)
        publish_batch(sensor, batch, true);
}

void libtofcan_receive(const struct libtofcan_msg *msg) {
    // ignore RTR messages
    if(msg->rtr)
        return;

    const int sensor   = msg->id % TOF2CAN_MAX_SENSOR_COUNT;
    const int msg_type = msg->id - sensor;

    switch(msg_type) {
        case TOF2CAN_SAMPLE_MASK_ID:
            handle_sample(sensor, msg->data, msg->len);
            break;

        case TOF2CAN_DATA_PACKET_MASK_ID:
            handle_data_packet(sensor, msg->data, msg->len);
            break;
    }
}

/* ================================================================== */
/*                           config_string                            */
/* ================================================================== */

void libtofcan_config_string(struct tof2can_config *config,
                             char *str, int maxlen) {
    int result = (config->processing_mode >> 4) & 3;
    const char *result_str = (const char *[]) {
        "min", "max", "average", "all points"
    }[result];

    char mode_str[64] = { 0 };
    int area = (config->processing_mode >> 6) & 3;
    switch(area) {
        case 0:
            snprintf(
                mode_str, sizeof(mode_str),
                "%s in matrix", result_str
            );
            break;
        case 1:
            snprintf(
                mode_str, sizeof(mode_str),
                "%s in column n. %d", result_str,
                config->processing_mode & 7
            );
            break;
        case 2:
            snprintf(
                mode_str, sizeof(mode_str),
                "%s in row n. %d", result_str,
                config->processing_mode & 7
            );
            break;
        case 3:
            snprintf(
                mode_str, sizeof(mode_str),
                "point at x=%d y=%d",
                config->processing_mode & 7,
                (config->processing_mode >> 3) & 7
            );
            break;
    }

    const char *timing_str = (const char *[]) {
        "on-demand", "continuous"
    }[config->transmit_timing & 1];

    const char *condition_str = (const char *[]) {
        "always true", "below threshold event",
        "above threshold event", "any threshold event"
    }[config->transmit_condition & 3];

    printf("-> %d\n\n", snprintf(
        str, maxlen,
        "resolution: %d points\n"
        "frequency: %d Hz\n"
        "processing mode: %s\n"
        "threshold: %d mm\n"
        "threshold delay: %d\n"
        "transmit timing: %s\n"
        "transmit condition: %s",
        config->resolution, config->frequency, mode_str,
        config->threshold, config->threshold_delay,
        timing_str, condition_str
    ));
}
