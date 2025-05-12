#pragma once

#include "main.h"

#include "binarysem.h"

extern int  processing_distance;
extern bool processing_threshold_status;

extern binarysem processing_request_sample;
extern binarysem processing_sample_available;

extern void processing_init(void);

extern void *processing_run(void *arg);

extern int processing_set_mode(int mode);
extern int processing_set_threshold(int threshold);
extern int processing_set_threshold_delay(int delay);
