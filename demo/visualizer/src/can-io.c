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
#include "can-io.h"

#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>

#include "distance-sensor.h"

static int sockfd;

#define RTR_BIT (1 << 30)

static int can_open(const char *ifname) {
    if((sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket");
        return 1;
    }

    struct ifreq ifr = { 0 };
    strcpy(ifr.ifr_name, ifname);
    ioctl(sockfd, SIOCGIFINDEX, &ifr);

    struct sockaddr_can addr = { 0 };
    addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Bind");
		return 1;
	}
    return 0;
}

static int can_write(uint32_t can_id, void *data, int len) {
    struct can_frame frame;

    frame.can_id  = can_id;
    frame.can_dlc = len;
    memcpy(frame.data, data, len);

    const int frame_size = sizeof(struct can_frame);
    if(write(sockfd, &frame, frame_size) != frame_size) {
        perror("CAN write");
        return -1;
    }
    return len;
}

static int can_read(uint32_t *can_id, void *data) {
    struct can_frame frame;

    if(read(sockfd, &frame, sizeof(struct can_frame)) < 0) {
        perror("CAN read");
        return -1;
    }

    *can_id = frame.can_id;
    int len = frame.can_dlc;
    memcpy(data, frame.data, len);
    return len;
}

static void batch_reset(struct DataBatch *batch,
                        int sensor_id, int batch_id) {
    // check if the previous batch was interrupted
    if(batch->packets_received != batch->packets_expected) {
        printf(
            "[Receiver] batch interrupted before receiving all packets "
            "(sensor=%d, batch=%d, received only %d)\n",
            sensor_id, batch->batch_id, batch->packets_received
        );
    }

    batch->data_length      = 0;
    batch->batch_id         = batch_id;
    batch->packets_received = 0;
    batch->packets_expected = 0;
}

static void batch_insert(struct DataBatch *batch, int sensor_id,
                         struct distance_sensor_can_data_packet *packet) {
    const int buffer_length = sizeof(batch->data) / sizeof(int16_t);
    const int offset = packet->sequence_number * 3;

    // check if packet data would overflow the buffer
    if(offset + packet->data_length > buffer_length) {
        printf(
            "[Receiver] received packet that would overflow buffer "
            "(sensor=%d, seq_number=%d, data_len=%d)\n",
            sensor_id, packet->sequence_number, packet->data_length
        );
        return;
    }

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

static void batch_print(struct DataBatch *batch, int sensor_id) {
    const int line_length = (batch->data_length == 16 ? 4 : 8);

    printf("[Receiver] received samples (sensor=%d):\n", sensor_id);
    for(int i = 0; i < batch->data_length; i++) {
        if(i % line_length == 0) {
            if(i != 0)
                printf("\n");
            printf("  ");
        }
        printf("%d, ", batch->data[i]);
    }
    printf("\n");
}

#define QUEUE_SIZE 8
static struct {
    struct DataBatch batches[QUEUE_SIZE];
    int head;
    int tail;
    int count;

    pthread_mutex_t mutex;
} batch_queue;

static void data_packets_receiver(void) {
    static struct DataBatch batches[DISTANCE_SENSOR_MAX_COUNT];

    while(1) {
        uint32_t can_id;
        uint8_t data[8];
        can_read(&can_id, data);

        if(can_id & RTR_BIT)
            continue;

        const int sensor_id = can_id % DISTANCE_SENSOR_MAX_COUNT;
        const int msg_type  = can_id - sensor_id;

        if(msg_type == DISTANCE_SENSOR_CAN_DATA_PACKET_MASK_ID) {
            struct distance_sensor_can_data_packet packet;
            memcpy(&packet, data, sizeof(packet));

            struct DataBatch *batch = &batches[sensor_id];

            // if packet has a new batch ID, reset the batch buffer
            if(batch->batch_id != packet.batch_id)
                batch_reset(batch, sensor_id, packet.batch_id);

            // insert new data into the batch buffer
            batch_insert(batch, sensor_id, &packet);

            // if all packets have been received, print the batch
            if(batch->packets_received == batch->packets_expected) {
                batch_print(batch, sensor_id);

                // insert batch into queue
                pthread_mutex_lock(&batch_queue.mutex);
                memcpy(
                    &batch_queue.batches[batch_queue.head],
                    batch, sizeof(struct DataBatch)
                );
                batch_queue.head = (batch_queue.head + 1) % QUEUE_SIZE;
                if(batch_queue.count < QUEUE_SIZE)
                    batch_queue.count++;
                pthread_mutex_unlock(&batch_queue.mutex);
            }
        }
    }
}


void *can_io_start(void *arg) {
    if(can_open("can0")) {
        printf("Error trying to open CAN device\n");
        return NULL;
    }

    pthread_mutex_init(&batch_queue.mutex, NULL);

    data_packets_receiver();
    return NULL;
}

int can_io_get_data(struct DataBatch *batch) {
    int err = 0;
    pthread_mutex_lock(&batch_queue.mutex);

    if(batch_queue.count == 0) {
        err = 1;
        goto exit;
    }

    // extract one batch from queue
    memcpy(
        batch,
        &batch_queue.batches[batch_queue.tail],
        sizeof(struct DataBatch)
    );
    batch_queue.tail = (batch_queue.tail + 1) % QUEUE_SIZE;
    batch_queue.count--;

    exit:
    pthread_mutex_unlock(&batch_queue.mutex);
    return err;
}
