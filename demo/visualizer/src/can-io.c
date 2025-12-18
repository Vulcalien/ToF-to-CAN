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

#include "libtofcan.h"
#include "tof2can.h"

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

static int can_write(const struct libtofcan_msg *msg) {
    struct can_frame frame;
    frame.can_id  = msg->id | (msg->rtr * RTR_BIT);
    frame.can_dlc = msg->len;
    memcpy(frame.data, msg->data, msg->len);

    const int frame_size = sizeof(struct can_frame);
    if(write(sockfd, &frame, frame_size) != frame_size) {
        perror("CAN write");
        return -1;
    }
    return msg->len;
}

static int can_read(struct libtofcan_msg *msg) {
    struct can_frame frame;
    if(read(sockfd, &frame, sizeof(struct can_frame)) < 0) {
        perror("CAN read");
        return -1;
    }

    msg->id  = frame.can_id & ~RTR_BIT;
    msg->rtr = frame.can_id & RTR_BIT;
    msg->len = frame.can_dlc;
    memcpy(msg->data, frame.data, msg->len);

    return msg->len;
}

#define QUEUE_SIZE 8
static struct {
    struct {
        int sensor;
        struct libtofcan_batch batch;
    } batches[QUEUE_SIZE];

    int head;
    int tail;
    int count;

    pthread_mutex_t mutex;
} queue;

static void callback_batch(int sensor, struct libtofcan_batch *data, bool valid) {
    // if batch is not valid, write an error
    if(!valid) {
        printf(
            "[Receiver] batch interrupted before receiving all packets "
            "(sensor=%d, batch=%d, received only %d)\n",
            sensor, data->batch_id, data->packets_received
        );
        return;
    }

    // print batch
    const int line_length = (data->data_length == 16 ? 4 : 8);
    printf("[Receiver] received samples (sensor=%d):\n", sensor);
    for(int i = 0; i < data->data_length; i++) {
        if(i % line_length == 0) {
            if(i != 0)
                printf("\n");
            printf("  ");
        }
        printf("%d, ", data->data[i]);
    }
    printf("\n");

    // insert batch into queue
    pthread_mutex_lock(&queue.mutex);
    queue.batches[queue.head].sensor = sensor;
    memcpy(
        &queue.batches[queue.head].batch,
        data, sizeof(struct libtofcan_batch)
    );
    queue.head = (queue.head + 1) % QUEUE_SIZE;
    if(queue.count < QUEUE_SIZE)
        queue.count++;
    pthread_mutex_unlock(&queue.mutex);
}

void *can_io_start(void *arg) {
    if(can_open("can0")) {
        printf("Error trying to open CAN device\n");
        return NULL;
    }

    pthread_mutex_init(&queue.mutex, NULL);

    // configure sensor
    struct libtofcan_msg msg;
    libtofcan_config(0, &msg, &(struct tof2can_config) {
        .resolution = 64, // 8x8
        .frequency  = 5, // 5 Hz
        .sharpener  = 5,

        .processing_mode = TOF2CAN_PROCMODE_ALL_IN_ROW(3),
        .threshold       = 0, // ignored
        .threshold_delay = 0, // ignored

        .transmit_timing    = 1, // continuous
        .transmit_condition = 0, // ignored (multiple samples)
    });
    can_write(&msg);

    libtofcan_set_callbacks(NULL, callback_batch);
    while(1) {
        struct libtofcan_msg msg;
        if(can_read(&msg) > 0)
            libtofcan_receive(&msg);
    }
    return NULL;
}

int can_io_get_data(int *sensor, struct libtofcan_batch *batch) {
    int err = 0;
    pthread_mutex_lock(&queue.mutex);

    if(queue.count == 0) {
        err = 1;
        goto exit;
    }

    // extract one batch from queue
    *sensor = queue.batches[queue.tail].sensor;
    memcpy(
        batch,
        &queue.batches[queue.tail].batch,
        sizeof(struct libtofcan_batch)
    );
    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
    queue.count--;

    exit:
    pthread_mutex_unlock(&queue.mutex);
    return err;
}
