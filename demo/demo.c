#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
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

static void config_sensor(struct distance_sensor_can_config *config) {
    const char *result_selector = "?";
    switch((config->processing_mode >> 4) & 3) {
        case 0:
            result_selector = "min";
            break;
        case 1:
            result_selector = "max";
            break;
        case 2:
            result_selector = "average";
            break;
        case 3:
            result_selector = "all points";
            break;
    }

    char mode_str[256] = { 0 };
    switch((config->processing_mode >> 6) & 3) {
        case 0:
            snprintf(mode_str, 256, "%s in matrix", result_selector);
            break;
        case 1:
            snprintf(
                mode_str, 256,
                "%s in column n. %d",
                result_selector, config->processing_mode & 7
            );
            break;
        case 2:
            snprintf(
                mode_str, 256,
                "%s in row n. %d",
                result_selector, config->processing_mode & 7
            );
            break;
        case 3:
            snprintf(
                mode_str, 256,
                "point at x=%d, y=%d",
                config->processing_mode & 7,
                (config->processing_mode >> 3) & 7
            );
            break;
    }

    const char *timing = (
        config->transmit_timing ? "continuous" : "on-demand"
    );

    const char *condition = "?";
    switch(config->transmit_condition) {
        case 0:
            condition = "always true";
            break;
        case 1:
            condition = "below threshold event";
            break;
        case 2:
            condition = "above threshold event";
            break;
        case 3:
            condition = "any threshold event";
            break;
    }

    printf("[Sender] configuring distance sensor:\n");
    printf("  resolution: %d points\n", config->resolution);
    printf("  frequency: %d Hz\n", config->frequency);
    printf("  processing mode: %s\n", mode_str);
    printf("  threshold: %d mm\n", config->threshold);
    printf("  threshold delay: %d\n", config->threshold_delay);
    printf("  transmit timing: %s\n", timing);
    printf("  transmit condition: %s\n", condition);

    can_write(
        DISTANCE_SENSOR_CAN_CONFIG_MASK_ID,
        config,
        DISTANCE_SENSOR_CAN_CONFIG_SIZE
    );
}

static void request_sample(void) {
    printf("[Sender] requesting sample\n");

    uint32_t can_id = DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID;
    uint8_t data[8];
    can_write(can_id | RTR_BIT, data, 0);
}

static void single_sample_receiver(void) {
    while(1) {
        uint32_t can_id;
        uint8_t data[8];
        can_read(&can_id, data);

        if(can_id & RTR_BIT)
            continue;

        const int sensor_id = can_id % DISTANCE_SENSOR_MAX_COUNT;
        const int msg_type  = can_id - sensor_id;

        if(msg_type == DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID) {
            struct distance_sensor_can_sample sample_data;
            memcpy(&sample_data, data, sizeof(sample_data));

            printf("[Receiver] received sample (sensor=%d):\n", sensor_id);
            printf("  distance: %d\n", sample_data.distance);
            printf("  below threshold: %d\n", sample_data.below_threshold);
        }
    }
}

struct DataBatch {
    int16_t data[64];
    int data_length;

    int batch_id;
    int packets_received;
    int packets_expected;
};

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
            if(batch->packets_received == batch->packets_expected)
                batch_print(batch, sensor_id);
        }
    }
}

/* ================================================================== */
/*                    demo: default configuration                     */
/* ================================================================== */

static void demo_default_config_sender(void) {
    struct distance_sensor_can_config config = {
        .resolution = 16,
        .frequency  = 1,
        .sharpener  = 5,

        .processing_mode = 0, // min in matrix
        .threshold       = 200,
        .threshold_delay = 2,

        .transmit_timing    = 0, // on-demand
        .transmit_condition = 0, // always true
    };
    config_sensor(&config);

    while(1) {
        getchar();
        request_sample();
    }
}

/* ================================================================== */
/*                       demo: higher frequency                       */
/* ================================================================== */

static void demo_higher_frequency_sender(void) {
    struct distance_sensor_can_config config = {
        .resolution = 16,
        .frequency  = 60,
        .sharpener  = 5,

        .processing_mode = 0, // min in matrix
        .threshold       = 200,
        .threshold_delay = 2,

        .transmit_timing    = 0, // on-demand
        .transmit_condition = 0, // always true
    };
    config_sensor(&config);

    while(1) {
        getchar();
        request_sample();
    }
}

/* ================================================================== */
/*                    demo: below threshold event                     */
/* ================================================================== */

static void demo_below_threshold_sender(void) {
    struct distance_sensor_can_config config = {
        .resolution = 16,
        .frequency  = 60,
        .sharpener  = 5,

        .processing_mode = 0, // min in matrix
        .threshold       = 200,
        .threshold_delay = 2,

        .transmit_timing    = 0, // on-demand
        .transmit_condition = 1, // below threshold event
    };
    config_sensor(&config);

    while(1) {
        getchar();
        request_sample();
    }
}

/* ================================================================== */
/*                   demo: continuous transmission                    */
/* ================================================================== */

static void demo_continuous_sender(void) {
    struct distance_sensor_can_config config = {
        .resolution = 16,
        .frequency  = 1,
        .sharpener  = 5,

        .processing_mode = 0, // min in matrix
        .threshold       = 200,
        .threshold_delay = 2,

        .transmit_timing    = 1, // continuous
        .transmit_condition = 0, // always true
    };
    config_sensor(&config);

    while(1) {
        getchar();
        request_sample();
    }
}

/* ================================================================== */
/*         demo: continuous transmission, any threshold event         */
/* ================================================================== */

static void demo_continuous_threshold_event_sender(void) {
    struct distance_sensor_can_config config = {
        .resolution = 16,
        .frequency  = 60,
        .sharpener  = 5,

        .processing_mode = 0, // min in matrix
        .threshold       = 200,
        .threshold_delay = 2,

        .transmit_timing    = 1, // continuous
        .transmit_condition = 3, // any threshold event
    };
    config_sensor(&config);

    while(1) {
        getchar();
        request_sample();
    }
}

/* ================================================================== */
/*                     demo: all points in matrix                     */
/* ================================================================== */

static void demo_all_points_in_matrix_sender(void) {
    struct distance_sensor_can_config config = {
        .resolution = 16,
        .frequency  = 1,
        .sharpener  = 5,

        .processing_mode = 0x30, // all points in matrix
        .threshold       = 0, // ignored
        .threshold_delay = 0, // ignored

        .transmit_timing    = 0, // on-demand
        .transmit_condition = 0, // always true
    };
    config_sensor(&config);

    while(1) {
        getchar();
        request_sample();
    }
}

/* ================================================================== */
/*                                main                                */
/* ================================================================== */

struct Demo {
    const char *name;
    void (*sender)  (void);
    void (*receiver)(void);
};

static void *run_sender(void *arg) {
    const struct Demo *demo = (const struct Demo *) arg;
    if(demo->sender)
        demo->sender();
    return NULL;
}

static void *run_receiver(void *arg) {
    const struct Demo *demo = (const struct Demo *) arg;
    if(demo->receiver)
        demo->receiver();
    return NULL;
}

#define DEMO_COUNT (sizeof(demos) / sizeof(struct Demo))
static struct Demo demos[] = {
    {
        "default configuration",
        demo_default_config_sender,
        single_sample_receiver
    }, {
        "higher frequency (consecutive requests are handled quickly)",
        demo_higher_frequency_sender,
        single_sample_receiver
    }, {
        "wait for below threshold event",
        demo_below_threshold_sender,
        single_sample_receiver
    }, {
        "continuous transmission",
        demo_continuous_sender,
        single_sample_receiver
    }, {
        "continuous transmission, but wait for any threshold event",
        demo_continuous_threshold_event_sender,
        single_sample_receiver
    }, {
        "all points in matrix",
        demo_all_points_in_matrix_sender,
        data_packets_receiver
    }
};

int main(int argc, char *argv[]) {
    if(can_open("can0")) {
        printf("Error trying to open CAN device\n");
        return 1;
    }

    puts("Please select a demo to run:");
    for(int i = 0; i < DEMO_COUNT; i++)
        printf("[%d] %s\n", i + 1, demos[i].name);
    printf("\n> ");

    int selected;
    scanf("%d", &selected);
    getchar(); // consume the newline character

    if(selected > 0 && selected <= DEMO_COUNT) {
        pthread_t sender_thread, receiver_thread;

        void *arg = &demos[selected - 1];

        pthread_create(&sender_thread,   NULL, run_sender,   arg);
        pthread_create(&receiver_thread, NULL, run_receiver, arg);

        pthread_join(sender_thread,   NULL);
        pthread_join(receiver_thread, NULL);
    }
    return 0;
}
