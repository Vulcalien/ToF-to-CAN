// SPDX-License-Identifier: GPL-3.0-or-later

/* Copyright 2025 Rocco Rusponi
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
#include "processing.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "tof.h"

#define AREA_MATRIX 0
#define AREA_COLUMN 1
#define AREA_ROW    2
#define AREA_POINT  3

#define SELECTOR_MIN     0
#define SELECTOR_MAX     1
#define SELECTOR_AVERAGE 2
#define SELECTOR_ALL     3

static struct {
    pthread_mutex_t mutex;
    sem_t           available;

    int     buffer_length;
    int16_t buffer[PROCESSING_DATA_MAX_LENGTH];

    bool below_threshold;
    bool threshold_event;
} data;

static struct {
    int x0, y0, x1, y1;
} bounds;
static int result_selector;

static int threshold;
static int threshold_delay;

static bool is_paused = true; // after startup, device is idle

int processing_init(void) {
    // initialize data mutex
    if(pthread_mutex_init(&data.mutex, NULL)) {
        printf("[Processing] error initializing data mutex\n");
        return 1;
    }

    // initialize data available semaphore
    if(sem_init(&data.available, 0, 0)) {
        printf("[Processing] error initializing data available semaphore\n");
        return 1;
    }
    return 0;
}

static void dump_data(int16_t *matrix) {
    printf("=== DATA DUMP ===\n");

    for(int y = bounds.y0; y <= bounds.y1; y++) {
        for(int x = bounds.x0; x <= bounds.x1; x++) {
            const int index = x + y * tof_matrix_width;
            printf("%d, ", matrix[index]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Distance data: ");
    for(int i = 0; i < data.buffer_length; i++)
        printf("%d, ", data.buffer[i]);
    printf("\n");

    if(data.buffer_length == 1) {
        printf("Below threshold: %d\n", data.below_threshold);
        printf("Threshold event: %d\n", data.threshold_event);
    }
    printf("\n");
}

static int process_matrix(int16_t *matrix, uint8_t *status,
                          int *count, int *sum, int *min, int *max) {
    *count = 0;
    *sum   = 0;
    *min   = INT_MAX;
    *max   = INT_MIN;

    // look for data within the configured bounds
    for(int y = bounds.y0; y <= bounds.y1; y++) {
        for(int x = bounds.x0; x <= bounds.x1; x++) {
            const int index = x + y * tof_matrix_width;
            const int sample = matrix[index];

            // check if point is valid
            if(status[index] != 5 && status[index] != 9) {
                matrix[index] = -1;
                continue;
            }

            *count += 1;
            *sum   += sample;

            if(*min > sample)
                *min = sample;

            if(*max < sample)
                *max = sample;
        }
    }

    // if no valid samples were found, return an error
    return (*count == 0);
}

static void update_threshold_status(void) {
    static bool previous    = false;
    static int  consistency = 0;

    const bool current = (data.buffer[0] < threshold);
    if(current == previous)
        consistency++;
    else
        consistency = 0;
    previous = current;

    const bool consistent_enough = (consistency >= threshold_delay);
    const bool status_change = (data.below_threshold != current);

    // if readings are consistent enough, update status and event flag
    if(consistent_enough && status_change) {
        data.below_threshold = current;
        data.threshold_event = true;
    } else {
        data.threshold_event = false;
    }

    // update status of LEDs: green=below threshold, red=threshold event
    board_userled(BOARD_GREEN_LED, data.below_threshold);
    board_userled(BOARD_RED_LED,   data.threshold_event);
}

static int update_data(void) {
    int16_t *matrix;
    uint8_t *status;

    // read ToF data, if available
    if(tof_read_data(&matrix, &status))
        return 1;

    // process the matrix to gather data about its elements
    int count, sum, min, max;
    if(process_matrix(matrix, status, &count, &sum, &min, &max)) {
        printf("[Processing] no valid data point was found\n");
        return 1;
    }

    // lock data mutex
    if(pthread_mutex_lock(&data.mutex)) {
        printf("[Processing] error trying to lock data mutex\n");
        return 1;
    }

    // update data based on result selector
    switch(result_selector) {
        case SELECTOR_MIN: {
            data.buffer[0] = min;
        } break;

        case SELECTOR_MAX: {
            data.buffer[0] = max;
        } break;

        case SELECTOR_AVERAGE: {
            data.buffer[0] = (sum / count);
        } break;

        case SELECTOR_ALL: {
            // copy all data points within the configured bounds
            int i = 0;
            for(int y = bounds.y0; y <= bounds.y1; y++) {
                for(int x = bounds.x0; x <= bounds.x1; x++) {
                    const int index = x + y * tof_matrix_width;
                    data.buffer[i++] = matrix[index];
                }
            }
        } break;
    }

    // if there is only one data value, update threshold status
    if(data.buffer_length == 1)
        update_threshold_status();

    // unlock data mutex
    if(pthread_mutex_unlock(&data.mutex)) {
        printf("[Processing] error trying to unlock data mutex\n");
        return 1;
    }

    // dump ToF matrix and processed data
    if(debug_flag)
        dump_data(matrix);

    return 0;
}

static void *processing_run(void *arg) {
    printf("[Processing] thread started\n");

    while(true) {
        // if not paused, try to update data
        if(!is_paused && update_data() == 0) {
            // get semaphore's current value
            int sem_value;
            sem_getvalue(&data.available, &sem_value);

            // Since old data is overwritten by new data, the maximum
            // value of the semaphore is 1: only increase its value if
            // the current value is 0.
            if(sem_value == 0)
                sem_post(&data.available);
        }

        // wait 1ms
        usleep(1000);
    }
    return NULL;
}

int processing_start(void) {
    pthread_t thread;
    if(pthread_create(&thread, NULL, processing_run, NULL)) {
        printf("[Processing] error creating thread\n");
        return 1;
    }
    return 0;
}

void processing_pause(void) {
    if(is_paused)
        return;

    is_paused = true;
    usleep(10000); // wait 10ms to ensure no data is being processed
    tof_stop_ranging();

    // invalidate data, if it was marked available
    sem_trywait(&data.available);
}

void processing_resume(void) {
    if(!is_paused)
        return;

    tof_start_ranging();
    is_paused = false;
}

int processing_get_data(int16_t *buffer,
                        int *length,
                        bool *below_threshold,
                        bool *threshold_event) {
    // if data is available, consume it, otherwise return an error
    if(sem_trywait(&data.available))
        return 1;

    // lock data mutex
    if(pthread_mutex_lock(&data.mutex)) {
        printf("[Processing] error trying to lock data mutex\n");
        return 1;
    }

    // copy data to the given pointers
    memcpy(buffer, data.buffer, data.buffer_length * sizeof(int16_t));
    *length          = data.buffer_length;
    *below_threshold = data.below_threshold;
    *threshold_event = data.threshold_event;

    // unlock data mutex
    if(pthread_mutex_unlock(&data.mutex)) {
        printf("[Processing] error trying to unlock data mutex\n");
        return 1;
    }
    return 0;
}

int processing_set_mode(int mode) {
    const int area = (mode >> 6) & 3;

    // set bounds of area to process and result selector
    switch(area) {
        case AREA_MATRIX: {
            const int selector = (mode >> 4) & 3;

            bounds.x0 = bounds.y0 = 0;
            bounds.x1 = bounds.y1 = tof_matrix_width - 1;
            result_selector = selector;
        } break;

        case AREA_COLUMN: {
            const int column = (mode & 7);
            const int selector = (mode >> 4) & 3;

            bounds.x0 = bounds.x1 = column;
            bounds.y0 = 0;
            bounds.y1 = tof_matrix_width - 1;
            result_selector = selector;
        } break;

        case AREA_ROW: {
            const int row = (mode & 7);
            const int selector = (mode >> 4) & 3;

            bounds.x0 = 0;
            bounds.x1 = tof_matrix_width - 1;
            bounds.y0 = bounds.y1 = row;
            result_selector = selector;
        } break;

        case AREA_POINT: {
            const int x = (mode & 7);
            const int y = (mode >> 3) & 7;

            bounds.x0 = bounds.x1 = x;
            bounds.y0 = bounds.y1 = y;
            result_selector = SELECTOR_MIN;
        } break;
    }

    // set data length
    if(result_selector == SELECTOR_ALL) {
        const int width  = (bounds.x1 - bounds.x0 + 1);
        const int height = (bounds.y1 - bounds.y0 + 1);
        data.buffer_length = width * height;
    } else {
        data.buffer_length = 1;
    }

    printf(
        "[Processing] setting area to (%d, %d, %d, %d), "
        "result selector to %d, data length to %d\n",
        bounds.x0, bounds.y0, bounds.x1, bounds.y1,
        result_selector, data.buffer_length
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
