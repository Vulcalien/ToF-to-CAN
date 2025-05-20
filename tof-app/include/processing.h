#pragma once

#include "main.h"

#include "binarysem.h"

#define PROCESSING_AREA_MATRIX 0
#define PROCESSING_AREA_COLUMN 1
#define PROCESSING_AREA_ROW    2
#define PROCESSING_AREA_POINT  3

#define PROCESSING_SELECTOR_MIN     0
#define PROCESSING_SELECTOR_MAX     1
#define PROCESSING_SELECTOR_AVERAGE 2
#define PROCESSING_SELECTOR_ALL     3

extern int  processing_distance;
extern bool processing_threshold_status;

extern binarysem processing_request_sample;
extern binarysem processing_sample_available;

extern int processing_area;
extern int processing_selector;

extern void processing_init(void);

extern void *processing_run(void *arg);

extern int processing_set_mode(int mode);
extern int processing_set_threshold(int threshold);
extern int processing_set_threshold_delay(int delay);
