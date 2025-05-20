#include "processing.h"

#include <stdio.h>

#include "binarysem.h"

#include "tof.h"

int  processing_distance;
bool processing_threshold_status;

binarysem processing_request_sample;
binarysem processing_sample_available;

int processing_area;
int processing_selector;

// arguments used by the processing functions
static int arg0, arg1;

static int threshold;
static int threshold_delay;

void processing_init(void) {
    binarysem_init(&processing_request_sample,   1); // request   = 1
    binarysem_init(&processing_sample_available, 0); // available = 0
}

// process the area with bounds (x0, y0, x1, y1)
static int process_area(int16_t *matrix, uint8_t *status,
                        int x0, int y0, int x1, int y1) {
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

    // return quantity chosen through processing_selector
    switch(processing_selector) {
        case PROCESSING_SELECTOR_MIN: return min;
        case PROCESSING_SELECTOR_MAX: return max;

        case PROCESSING_SELECTOR_AVERAGE:
            if(count == 0 || sum < 0)
                return -1;
            return (sum / count);

        case PROCESSING_SELECTOR_ALL: // TODO
            return -1;
    }
    return -1;
}

// return 0 if distance is valid
static inline int update_distance(void) {
    int16_t *matrix;
    uint8_t *status_matrix;

    tof_read_data(&matrix, &status_matrix);

    // determine which area of the matrix should be processed
    int x0, y0, x1, y1;
    switch(processing_area) {
        case PROCESSING_AREA_MATRIX:
            x0 = y0 = 0;
            x1 = y1 = tof_matrix_width;
            break;

        case PROCESSING_AREA_COLUMN:
            x0 = x1 = arg0;
            y0 = 0; y1 = tof_matrix_width;
            break;

        case PROCESSING_AREA_ROW:
            x0 = 0; x1 = tof_matrix_width;
            y0 = y1 = arg0;
            break;

        case PROCESSING_AREA_POINT:
            x0 = x1 = arg0;
            y0 = y1 = arg1;
            break;
    }

    processing_distance = process_area(
        matrix, status_matrix,
        x0, y0, x1, y1 // bounds of the area
    );
    return (processing_distance < 0);
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

        int err = update_distance();
        if(err) {
            // failure: try to sample again
            printf("[Processing] sampling failed, retrying\n");
            binarysem_post(&processing_request_sample);
        } else {
            // success: update threshold status and notify other threads
            if(processing_selector != PROCESSING_SELECTOR_ALL)
                update_threshold_status();
            binarysem_post(&processing_sample_available);
        }
    }
    return NULL;
}

int processing_set_mode(int mode) {
    processing_area = (mode >> 6) & 3;

    if(processing_area == PROCESSING_AREA_POINT) {
        processing_selector = PROCESSING_SELECTOR_MIN;
        arg0 = mode & 7;
        arg1 = (mode >> 3) & 7;
    } else {
        processing_selector = (mode >> 4) & 3;
        arg0 = mode & 7;
    }

    printf(
        "[Processing] setting area to %d and selector to %d"
        "arg0=%d, arg1=%d\n",
        processing_area, processing_selector, arg0, arg1
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
