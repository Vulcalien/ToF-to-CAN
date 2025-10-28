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

#include "distance-sensor.h"

struct {
    void (*sample)(int sensor, struct distance_sensor_can_sample *data);
    void (*batch)(int sensor, struct libtofcan_batch *data, bool valid);
} callbacks;

void libtofcan_set_callbacks(
    void (*sample)(int sensor, struct distance_sensor_can_sample *data),
    void (*batch)(int sensor, struct libtofcan_batch *data, bool valid)
) {
    callbacks.sample = sample;
    callbacks.batch = batch;
}

/* ================================================================== */
/*                          config & request                          */
/* ================================================================== */

void libtofcan_config(int sensor, struct libtofcan_msg *msg,
                      struct distance_sensor_can_config *config) {
    static uint8_t data[DISTANCE_SENSOR_CAN_CONFIG_SIZE];
    memcpy(data, config, DISTANCE_SENSOR_CAN_CONFIG_SIZE);

    *msg = (struct libtofcan_msg) {
        .id   = DISTANCE_SENSOR_CAN_CONFIG_MASK_ID | sensor,
        .rtr  = false,
        .data = data,
        .len  = DISTANCE_SENSOR_CAN_CONFIG_SIZE
    };
}

void libtofcan_request(int sensor, struct libtofcan_msg *msg) {
    *msg = (struct libtofcan_msg) {
        .id   = DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID | sensor,
        .rtr  = true,
        .data = NULL,
        .len  = 0
    };
}

/* ================================================================== */
/*                              receiver                              */
/* ================================================================== */

static void handle_sample(int sensor, void *data, int len) {
    // check if message size is correct
    if(len != sizeof(struct distance_sensor_can_sample))
        return;

    struct distance_sensor_can_sample sample;
    memcpy(&sample, data, sizeof(sample));

    callbacks.sample(sensor, &sample);
}

static void batch_reset(struct libtofcan_batch *batch,
                        int sensor, int batch_id) {
    // if previous batch was interrupted, send it as invalid
    if(batch->packets_received != batch->packets_expected)
        callbacks.batch(sensor, batch, false);

    batch->data_length      = 0;
    batch->batch_id         = batch_id;
    batch->packets_received = 0;
    batch->packets_expected = 0;
}

static void batch_insert(struct libtofcan_batch *batch,
                         struct distance_sensor_can_data_packet *packet) {
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

static void handle_data_packet(int sensor, void *data, int len) {
    static struct libtofcan_batch batches[DISTANCE_SENSOR_MAX_COUNT];

    // check if message size is correct
    if(len != sizeof(struct distance_sensor_can_data_packet))
        return;

    struct libtofcan_batch *batch = &batches[sensor];
    struct distance_sensor_can_data_packet packet;
    memcpy(&packet, data, sizeof(packet));

    // if packet has a new batch ID, reset the batch buffer
    if(batch->batch_id != packet.batch_id)
        batch_reset(batch, sensor, packet.batch_id);

    // insert new data into the batch buffer
    batch_insert(batch, &packet);

    // if all packets have been received, send the batch
    if(batch->packets_received == batch->packets_expected)
        callbacks.batch(sensor, batch, true);
}

void libtofcan_receive(const struct libtofcan_msg *msg) {
    // ignore RTR messages
    if(msg->rtr)
        return;

    const int sensor   = msg->id % DISTANCE_SENSOR_MAX_COUNT;
    const int msg_type = msg->id - sensor;

    switch(msg_type) {
        case DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID:
            handle_sample(sensor, msg->data, msg->len);
            break;

        case DISTANCE_SENSOR_CAN_DATA_PACKET_MASK_ID:
            handle_data_packet(sensor, msg->data, msg->len);
            break;
    }
}
