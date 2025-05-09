#include "main.h"

#include <pthread.h>

#include "processing.h"
#include "receiver.h"
#include "sender.h"

int main(int argc, char *argv[]) {
    processing_init();

    pthread_t processing_thread;
    pthread_t receiver_thread;
    pthread_t sender_thread;

    // TODO handle returned errors
    pthread_create(&processing_thread, NULL, processing_run, NULL);
    pthread_create(&receiver_thread,   NULL, receiver_run,   NULL);
    pthread_create(&sender_thread,     NULL, sender_run,     NULL);

    return 0;
}
