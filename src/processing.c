#include "receiver.h"

#include <stdio.h>

int  processing_distance;
bool processing_threshold_status;

static int threshold;
static int threshold_delay;
static int processing_mode;

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
        // TODO make sure a new sample is needed (?)

        update_distance();
        update_threshold_status();

        // notify other threads of the new sample (?)
    }
    return NULL;
}
