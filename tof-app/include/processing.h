#pragma once

#include "main.h"

#include <pthread.h>

#include "binarysem.h"

#define PROCESSING_AREA_MATRIX 0
#define PROCESSING_AREA_COLUMN 1
#define PROCESSING_AREA_ROW    2
#define PROCESSING_AREA_POINT  3

#define PROCESSING_SELECTOR_MIN     0
#define PROCESSING_SELECTOR_MAX     1
#define PROCESSING_SELECTOR_AVERAGE 2
#define PROCESSING_SELECTOR_ALL     3

extern pthread_mutex_t processing_data_mutex;
extern binarysem       processing_data_available;

extern int  processing_distance;
extern bool processing_threshold_status;

extern int processing_area;
extern int processing_selector;

extern int processing_init(void);
extern int processing_start(void);

extern int processing_set_mode(int mode);
extern int processing_set_threshold(int threshold);
extern int processing_set_threshold_delay(int delay);
