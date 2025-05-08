#include "receiver.h"

#include <stdio.h>
#include <semaphore.h>

int  processing_distance;
bool processing_threshold_status;

sem_t processing_request_sample;
sem_t processing_sample_available;

static int processing_mode;
static int threshold;
static int threshold_delay;

void processing_init(void) {
    sem_init(&processing_request_sample,   0, 1); // request   = 1
    sem_init(&processing_sample_available, 0, 0); // available = 0
}

void processing_config(int _mode, int _threshold, int _threshold_delay) {
    processing_mode = _mode;
    threshold       = _threshold;
    threshold_delay = _threshold_delay;
}

static inline void update_distance(void) {
    // TODO
}

static inline void update_threshold_status(void) {
    static bool previous    = false;
    static int  consistency = 0;

    const bool current = (processing_distance < threshold);
    if(current == previous)
        consistency++;
    else
        consistency = 0;
    previous = current;

    // if readings are consistent enough, update threshold status
    if(consistency >= threshold_delay)
        processing_threshold_status = current;
}

void *processing_run(void *arg) {
    while(true) {
        // wait for a sample request
        sem_wait(&processing_request_sample);

        update_distance();
        update_threshold_status();

        // notify other threads that a sample is available
        sem_post(&processing_sample_available);
    }
    return NULL;
}
