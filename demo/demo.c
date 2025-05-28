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

static int sockfd;

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
}

static int can_write(uint32_t can_id, uint8_t *data, int len) {
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

static int can_read(uint32_t *can_id, uint8_t *data) {
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
    // TODO ...
};

int main(int argc, int *argv[]) {
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

    if(selected > 0 && selected <= DEMO_COUNT) {
        pthread_t sender_thread, receiver_thread;

        void *arg = &demos[selected];

        pthread_create(&sender_thread,   NULL, run_sender,   arg);
        pthread_create(&receiver_thread, NULL, run_receiver, arg);

        pthread_join(sender_thread,   NULL);
        pthread_join(receiver_thread, NULL);
    }
    return 0;
}
