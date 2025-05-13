#include "processing.h"

#include <stdio.h>

#include "binarysem.h"

#include "tof.h"

int  processing_distance;
bool processing_threshold_status;

binarysem processing_request_sample;
binarysem processing_sample_available;

static int processing_mode;
static int threshold;
static int threshold_delay;

void processing_init(void) {
    binarysem_init(&processing_request_sample,   1); // request   = 1
    binarysem_init(&processing_sample_available, 0); // available = 0
}

// process the area with bounds (x0, y0, x1, y1), and return the
// quantity chosen through result_selector: 0=min, 1=max, 2=average
static int process_area(int16_t *matrix, uint8_t *status,
                        int x0, int y0, int x1, int y1,
                        int result_selector) {
    int min = -1, max = -1, sum = -1, count = 0;
    for(int y = y0; y <= y1; y++) {
        for(int x = x0; x <= x1; x++) {
            const int index = x + y * tof_matrix_width;
            const int sample = matrix[index];

            // check if point is valid
            if(status[index] != 5 && status[index] != 9)
                continue;

            if(min < 0 || sample < min) min = sample;
            if(max < 0 || sample > max) max = sample;

            if(sum < 0) sum = sample;
            else sum += sample;

            count++;
        }
    }

    // return quantity chosen through result_selector
    switch(result_selector) {
        case 0: return min;
        case 1: return max;

        case 2: // average
            if(count == 0 || sum < 0)
                return -1;
            return (sum / count);
    }
    printf("[Processing] unknown result selector: %d\n", result_selector);
    return -1;
}

static inline void update_distance(void) {
    int16_t *matrix;
    uint8_t *status_matrix;

    tof_read_data(&matrix, &status_matrix);

    // determine which area of the matrix should be processed
    int x0, y0, x1, y1;
    int result_selector;
    switch((processing_mode >> 6) & 3) {
        case 0: { // matrix
            result_selector = (processing_mode >> 4) & 3;
            x0 = y0 = 0;
            x1 = y1 = tof_matrix_width;
        } break;

        case 1: { // column
            x0 = x1 = (processing_mode & 7);
            y0 = 0; y1 = tof_matrix_width;
            result_selector = (processing_mode >> 4) & 3;
        } break;

        case 2: { // row
            x0 = 0; x1 = tof_matrix_width;
            y0 = y1 = (processing_mode & 7);
            result_selector = (processing_mode >> 4) & 3;
        } break;

        case 3: { // point
            x0 = x1 = (processing_mode & 7);
            y0 = y1 = (processing_mode >> 3) & 7;
            result_selector = 0; // min
        }
    }

    processing_distance = process_area(
        matrix, status_matrix,
        x0, y0, x1, y1, // bounds of the area
        result_selector
    );
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
        binarysem_wait(&processing_request_sample);

        update_distance();
        update_threshold_status();

        // notify other threads that a sample is available
        binarysem_post(&processing_sample_available);
    }
    return NULL;
}

int processing_set_mode(int mode) {
    int err = 0; // TODO handle invalid modes?

    processing_mode = mode;
    printf(
        "[Processing] setting mode to %d (err=%d)\n",
        mode, err
    );
    return 0;
}

int processing_set_threshold(int _threshold) {
    int err = 0;

    if(_threshold >= 0) threshold = _threshold;
    else err = 1;

    printf(
        "[Processing] setting threshold to %d (err=%d)\n",
        _threshold, err
    );
    return err;
}

int processing_set_threshold_delay(int delay) {
    int err = 0;

    if(delay >= 0) threshold_delay = delay;
    else err = 1;

    printf(
        "[Processing] setting threshold delay to %d (err=%d)\n",
        delay, err
    );
    return err;
}
